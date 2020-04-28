/****************************************************************************
**
** Copyright (C) 2020 Alexis Jeandet.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "mesonprocess.h"
#include "mesonoutputparser.h"
#include <coreplugin/messagemanager.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/taskhub.h>
#include <utils/stringutils.h>

namespace MesonProjectManager {
namespace Internal {
MesonProcess::MesonProcess()
{
    connect(&m_cancelTimer, &QTimer::timeout, this, &MesonProcess::checkForCancelled);
    m_cancelTimer.setInterval(500);
}

bool MesonProcess::run(const Command &command, const Utils::Environment env, bool captureStdo)
{
    if(!sanityCheck(command))
        return false;
    m_currentCommand = command;
    m_stdo.clear();
    m_processWasCanceled = false;
    m_future = decltype(m_future){};
    setupProcess(command, env, captureStdo);
    ProjectExplorer::TaskHub::clearTasks(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM);
    m_future.setProgressRange(0, 1);
    Core::ProgressManager::addTimedTask(m_future,
                                        tr("Configuring \"%1\"").arg("TODO Set project Name"),
                                        "Meson.Configure",
                                        10);
    emit started();
    m_elapsed.start();
    m_process->start();
    m_cancelTimer.start(500);
    return true;
}

QProcess::ProcessState MesonProcess::state() const
{
    return m_process->state();
}

void MesonProcess::reportCanceled()
{
    m_future.reportCanceled();
}

void MesonProcess::reportFinished()
{
    m_future.reportFinished();
}

void MesonProcess::setProgressValue(int p)
{
    m_future.setProgressValue(p);
}

void MesonProcess::handleProcessFinished(int code, QProcess::ExitStatus status)
{
    m_cancelTimer.stop();
    m_stdo = m_process->readAllStandardOutput();
    m_stderr = m_process->readAllStandardError();
    if (status == QProcess::NormalExit) {
        m_future.setProgressValue(1);
        m_future.reportFinished();
    } else {
        m_future.reportCanceled();
        m_future.reportFinished();
    }
    const QString elapsedTime = Utils::formatElapsedTime(m_elapsed.elapsed());
    Core::MessageManager::write(elapsedTime);
    emit finished(code, status);
}

void MesonProcess::handleProcessError(QProcess::ProcessError error)
{
    QString message;
    QString commandStr = m_currentCommand.toString();
    switch (error) {
    case QProcess::ProcessError::FailedToStart:
        message = tr("Failed to start");
        break;
    case QProcess::ProcessError::Crashed:
        message = tr("Command crashed");
        break;
    case QProcess::ProcessError::Timedout:
        message = tr("Command timedout");
        break;
    case QProcess::ProcessError::WriteError:
        message = tr("Write error");
        break;
    case QProcess::ProcessError::ReadError:
        message = tr("Read error");
        break;
    case QProcess::ProcessError::UnknownError:
        message = tr("Unknown error with command");
        break;
    }
    ProjectExplorer::TaskHub::addTask(
        ProjectExplorer::BuildSystemTask{ProjectExplorer::Task::TaskType::Error,
                                         QString("%1: %2").arg(message).arg(commandStr)});
    handleProcessFinished(-1, QProcess::CrashExit);
}

void MesonProcess::checkForCancelled()
{
    if (m_future.isCanceled()) {
        m_cancelTimer.stop();
        m_processWasCanceled = true;
        m_process->close();
    }
}

void MesonProcess::setupProcess(const Command &command,
                                const Utils::Environment env,
                                bool captureStdo)
{
    if (m_process)
        disconnect(m_process.get());
    m_process = std::make_unique<Utils::QtcProcess>();
    connect(m_process.get(),
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &MesonProcess::handleProcessFinished);
    connect(m_process.get(), &QProcess::errorOccurred, this, &MesonProcess::handleProcessError);
    if (!captureStdo) {
        connect(m_process.get(),
                &QProcess::readyReadStandardOutput,
                this,
                &MesonProcess::processStandardOutput);

        connect(m_process.get(),
                &QProcess::readyReadStandardError,
                this,
                &MesonProcess::processStandardError);
    }

    m_process->setWorkingDirectory(command.workDir.toString());
    m_process->setEnvironment(env);
    Utils::CommandLine commandLine{command.exe, command.arguments};
    m_process->setCommand(commandLine);
}

bool MesonProcess::sanityCheck(const Command &command) const
{
    if(!command.exe.exists())
    {
        //Should only reach this point if Meson exe is removed while a Meson project is opened
        ProjectExplorer::TaskHub::addTask(
            ProjectExplorer::BuildSystemTask{ProjectExplorer::Task::TaskType::Error,tr("Following executable doens't exist: %1").arg(command.exe.toString())});
        return false;
    }
    if(!command.exe.toFileInfo().isExecutable())
    {
        ProjectExplorer::TaskHub::addTask(
            ProjectExplorer::BuildSystemTask{ProjectExplorer::Task::TaskType::Error,tr("Following command is not executable: %1").arg(command.exe.toString())});
        return false;
    }
    return true;
}

void MesonProcess::processStandardOutput()
{
    QTC_ASSERT(m_process, return );
    auto data = m_process->readAllStandardOutput();
    Core::MessageManager::write(QString::fromLatin1(data));
    emit readyReadStandardOutput(data);
}

void MesonProcess::processStandardError()
{
    QTC_ASSERT(m_process, return );

    Core::MessageManager::write(QString::fromLatin1(m_process->readAllStandardError()));
}
} // namespace Internal
} // namespace MesonProjectManager

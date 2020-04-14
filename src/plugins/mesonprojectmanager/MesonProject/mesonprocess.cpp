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
#include "coreplugin/messagemanager.h"
#include "coreplugin/progressmanager/progressmanager.h"
#include "projectexplorer/projectexplorerconstants.h"
#include "projectexplorer/taskhub.h"
#include "utils/stringutils.h"
namespace MesonProjectManager {
namespace Internal {
MesonProcess::MesonProcess()
{
    connect(&m_cancelTimer, &QTimer::timeout, this, &MesonProcess::checkForCancelled);
    m_cancelTimer.setInterval(500);
}

void MesonProcess::run(const MesonCommand &command, const Utils::Environment env, bool captureStdo)
{
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
    // TODO process output
    m_cancelTimer.stop();
    if (status == QProcess::NormalExit) {
        m_stdo = m_process->readAllStandardOutput();
        m_future.setProgressValue(1);
        m_future.reportFinished();
    } else {
        m_future.reportCanceled();
    }
    const QString elapsedTime = Utils::formatElapsedTime(m_elapsed.elapsed());
    Core::MessageManager::write(elapsedTime);
    emit finished(code, status);
}

void MesonProcess::checkForCancelled()
{
    if (m_future.isCanceled()) {
        m_cancelTimer.stop();
        m_processWasCanceled = true;
        m_process->close();
    }
}

void MesonProcess::setupProcess(const MesonCommand &command,
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

    if (!captureStdo)
        connect(m_process.get(),
                &QProcess::readyReadStandardOutput,
                this,
                &MesonProcess::processStandardOutput);

    connect(m_process.get(),
            &QProcess::readyReadStandardError,
            this,
            &MesonProcess::processStandardError);

    m_process->setWorkingDirectory(command.workDir.toString());
    m_process->setEnvironment(env);
    Utils::CommandLine commandLine(command.exe, command.arguments);
    m_process->setCommand(commandLine);
}

void MesonProcess::processStandardOutput()
{
    QTC_ASSERT(m_process, return );

    Core::MessageManager::write(QString::fromLatin1(m_process->readAllStandardOutput()));
}

void MesonProcess::processStandardError()
{
    QTC_ASSERT(m_process, return );

    Core::MessageManager::write(QString::fromLatin1(m_process->readAllStandardError()));
}
} // namespace Internal
} // namespace MesonProjectManager

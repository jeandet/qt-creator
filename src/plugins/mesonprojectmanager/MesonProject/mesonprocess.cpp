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
    connect(&m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &MesonProcess::handleProcessFinished);
}

void MesonProcess::run(const MesonCommand &command, const Utils::Environment env)
{
    m_stdo.clear();
    m_processWasCanceled = false;
    m_future = decltype(m_future){};
    m_cancelTimer.start();
    m_process.setWorkingDirectory(command.workDir.toString());
    m_process.setEnvironment(env);
    Utils::CommandLine commandLine(command.exe, command.arguments);
    ProjectExplorer::TaskHub::clearTasks(ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM);
    m_future.setProgressRange(0, 1);
    Core::ProgressManager::addTimedTask(m_future,
                                        tr("Configuring \"%1\"").arg("TODO Set project Name"),
                                        "Meson.Configure",
                                        10);
    m_process.setCommand(commandLine);
    emit started();
    m_elapsed.start();
    m_process.start();
}

QProcess::ProcessState MesonProcess::state() const
{
    return m_process.state();
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
        m_stdo = m_process.readAllStandardOutput();
        m_future.setProgressValue(1);
        m_future.reportFinished();
    } else {
        m_future.reportCanceled();
    }
    emit finished(code, status);
    const QString elapsedTime = Utils::formatElapsedTime(m_elapsed.elapsed());
    Core::MessageManager::write(elapsedTime);
}

void MesonProcess::checkForCancelled()
{
    if (m_future.isCanceled()) {
        m_cancelTimer.stop();
        m_processWasCanceled = true;
        m_process.close();
    }
}
} // namespace Internal
} // namespace MesonProjectManager

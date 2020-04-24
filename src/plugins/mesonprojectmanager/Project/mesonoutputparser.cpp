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
#include "mesonoutputparser.h"
#include <projectexplorer/task.h>
#include <projectexplorer/taskhub.h>
namespace MesonProjectManager {
namespace Internal {

void MesonOutputParser::pushLine(const QString &line)
{
    m_remainingLines--;
    m_pending.append(line);
    m_pending.append('\n');
    if (m_remainingLines == 0) {
        ProjectExplorer::TaskHub::addTask(
            ProjectExplorer::BuildSystemTask(ProjectExplorer::Task::TaskType::Warning, m_pending));
        m_pending.clear();
    }
}

MesonOutputParser::MesonOutputParser() {}

Utils::OutputLineParser::Result MesonOutputParser::handleLine(const QString &line,
                                                              Utils::OutputFormat type)
{
    if (type != Utils::OutputFormat::StdOutFormat)
        return ProjectExplorer::OutputTaskParser::Status::NotHandled;
    if (m_remainingLines) {
        pushLine(line);
        return ProjectExplorer::OutputTaskParser::Status::Done;
    }
    auto optionsErrors = m_errorOptionRegex.match(line);
    if (optionsErrors.hasMatch()) {
        ProjectExplorer::TaskHub::addTask(
            ProjectExplorer::BuildSystemTask(ProjectExplorer::Task::TaskType::Error, line));
        return ProjectExplorer::OutputTaskParser::Status::Done;
    }
    auto locatedErrors = m_errorFileLocRegex.match(line);
    if (locatedErrors.hasMatch()) {
        ProjectExplorer::TaskHub::addTask(
            ProjectExplorer::BuildSystemTask(ProjectExplorer::Task::TaskType::Error,
                                             line,
                                             absoluteFilePath(Utils::FilePath::fromString(
                                                 locatedErrors.captured(1))),
                                             locatedErrors.captured(2).toInt()));

        return ProjectExplorer::OutputTaskParser::Status::Done;
    }
    auto warning = m_3linesWarning.match(line);
    if (warning.hasMatch()) {
        m_remainingLines = 3;
        pushLine(line);
        return ProjectExplorer::OutputTaskParser::Status::Done;
    }
    warning = m_2linesWarning.match(line);
    if (warning.hasMatch()) {
        m_remainingLines = 3;
        pushLine(line);
        return ProjectExplorer::OutputTaskParser::Status::Done;
    }
    warning = m_1lineWarning.match(line);
    if (warning.hasMatch()) {
        m_remainingLines = 1;
        pushLine(line);
        return ProjectExplorer::OutputTaskParser::Status::Done;
    }
    return ProjectExplorer::OutputTaskParser::Status::NotHandled;
}

void MesonOutputParser::readStdo(const QByteArray &data)
{
    auto str = QString::fromLatin1(data);
    for (const auto &line : str.split('\n'))
        handleLine(line, Utils::OutputFormat::StdOutFormat);
}

void MesonOutputParser::setSourceDirectory(const Utils::FilePath &sourceDir)
{
    emit addSearchDir(sourceDir);
}

} // namespace Internal
} // namespace MesonProjectManager

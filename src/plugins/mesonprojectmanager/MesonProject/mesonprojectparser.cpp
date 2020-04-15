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
#include "mesonprojectparser.h"
#include "MesonInfoParser/mesoninfoparser.h"
#include "ProjectTree/mesonprojectnodes.h"
#include "ProjectTree/projecttree.h"
#include <projectexplorer/projectexplorer.h>
#include <utils/runextensions.h>
#include <QTextStream>

namespace MesonProjectManager {
namespace Internal {
MesonProjectParser::MesonProjectParser(const MesonWrapper &meson)
    : m_meson{meson}
    , m_configuring{false}
{
    connect(&m_process,
            &MesonProcess::finished,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                    startParser();
                }
            });
}

void MesonProjectParser::configure(const Utils::FilePath &sourcePath,
                                   const Utils::FilePath &buildPath,
                                   const QStringList &args,
                                   const Utils::Environment &env)
{
    m_introType = IntroDataType::file;
    m_buildDir = buildPath;
    m_process.run(m_meson.configure(sourcePath, buildPath, args), env);
}

void MesonProjectParser::parse(const Utils::FilePath &sourcePath, const Utils::FilePath &buildPath)
{
    m_srcDir = sourcePath;
    if (!isSetup(buildPath)) {
        parse(sourcePath);
    } else {
        startParser();
    }
}

void MesonProjectParser::parse(const Utils::FilePath &sourcePath)
{
    m_srcDir = sourcePath;
    m_introType = IntroDataType::stdo;
    m_process.run(m_meson.introspect(sourcePath), Utils::Environment{}, true);
}
void MesonProjectParser::startParser()
{
    m_parserResult = Utils::runAsync(ProjectExplorer::ProjectExplorerPlugin::sharedThreadPool(),
                                     [process = &m_process,
                                      introType = m_introType,
                                      buildDir = m_buildDir.toString(),
                                      srcDir = m_srcDir]() {
                                         if (introType == IntroDataType::file) {
                                             MesonInfoParser parser(buildDir);
                                             return extractParserResults(srcDir, parser);
                                         } else {
                                             MesonInfoParser parser(process->stdo());
                                             return extractParserResults(srcDir, parser);
                                         }
                                     });

    Utils::onFinished(m_parserResult, this, [this](const QFuture<ParserData *> &data) {
        auto parserData = data.result();
        m_targets = std::move(parserData->targets);
        m_buildOptions = std::move(parserData->buildOptions);
        m_projectParts = std::move(parserData->projectParts);
        m_rootNode = std::move(parserData->rootNode);
        delete data;
        emit parsingCompleted(true);
    });
}

MesonProjectParser::ParserData *MesonProjectParser::extractParserResults(
    const Utils::FilePath &srcDir, MesonInfoParser &parser)
{
    auto targets = parser.targets();
    auto buildOptions = parser.buildOptions();
    auto projectParts = buildProjectParts(targets);
    auto rootNode = ProjectTree::buildTree(srcDir, targets);
    return new ParserData{std::move(targets),
                          std::move(buildOptions),
                          std::move(projectParts),
                          std::move(rootNode)};
}

ProjectExplorer::RawProjectParts MesonProjectParser::buildProjectParts(const TargetsList &targets)
{
    ProjectExplorer::RawProjectParts parts;
    return parts;
}
} // namespace Internal
} // namespace MesonProjectManager

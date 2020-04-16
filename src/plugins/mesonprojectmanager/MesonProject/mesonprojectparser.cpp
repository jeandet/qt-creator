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
#include <utils/optional.h>
#include <utils/runextensions.h>
#include <QStringList>
#include <QTextStream>

namespace MesonProjectManager {
namespace Internal {

struct CompilerArgs
{
    QStringList args;
    QStringList includePaths;
    ProjectExplorer::Macros macros;
};



inline Utils::optional<QString> extractValueIfMatches(const QString &arg, const QStringList& candidates)
{
    for(const auto& flag:candidates)
    {
        if(arg.startsWith(flag))
            return arg.mid(flag.length());
    }
    return Utils::nullopt;
}

inline Utils::optional<QString> extractInclude(const QString &arg)
{
    return extractValueIfMatches(arg,{"-I","/I", "-isystem", "-imsvc", "/imsvc"});
}
inline Utils::optional<QString> extractMacro(const QString &arg)
{
    return extractValueIfMatches(arg,{"-D","/D", "-U", "/U"});
}

CompilerArgs splitArgs(const QStringList &args)
{
    CompilerArgs splited;
    std::for_each(std::cbegin(args), std::cend(args), [&splited](const QString &arg) {
        auto inc = extractInclude(arg);
        if (inc) {
            splited.includePaths << *inc;
        } else {
            auto macro = extractMacro(arg);
            if (macro) {
                splited.macros << ProjectExplorer::Macro(macro->toLatin1());
            } else {
                splited.args << arg;
            }
        }
    });
    return splited;
}

MesonProjectParser::MesonProjectParser(const Core::Id& meson)
    : m_meson{meson}
    , m_configuring{false}
{
    connect(&m_process,
            &MesonProcess::finished,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                    startParser();
                }
                else
                    emit parsingCompleted(false);
            });
}

void MesonProjectParser::setMesonTool(const Core::Id &meson)
{
    m_meson = meson;
}

void MesonProjectParser::configure(const Utils::FilePath &sourcePath,
                                   const Utils::FilePath &buildPath,
                                   const QStringList &args,
                                   const Utils::Environment &env)
{
    m_introType = IntroDataType::file;
    m_buildDir = buildPath;
    m_process.run(MesonTools::tool(m_meson)->configure(sourcePath, buildPath, args), env);
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
    m_process.run(MesonTools::tool(m_meson)->introspect(sourcePath), Utils::Environment{}, true);
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
    auto rootNode = ProjectTree::buildTree(srcDir, targets);
    return new ParserData{std::move(targets), std::move(buildOptions), std::move(rootNode)};
}

ProjectExplorer::RawProjectParts MesonProjectParser::buildProjectParts(
    const ProjectExplorer::ToolChain *cxxToolChain, const ProjectExplorer::ToolChain *cToolChain)
{
    ProjectExplorer::RawProjectParts parts;
    std::for_each(std::cbegin(m_targets),
                  std::cend(m_targets),
                  [&parts, &cxxToolChain, &cToolChain](const Target &target) {
                      std::for_each(
                          std::cbegin(target.sources),
                          std::cend(target.sources),
                          [&parts, &cxxToolChain, &cToolChain](const Target::Source &sourceList) {
                              ProjectExplorer::RawProjectPart part;
                              part.setFiles(sourceList.sources);
                              auto flags = splitArgs(sourceList.parameters);
                              part.setMacros(flags.macros);
                              part.setIncludePaths(flags.includePaths);
                              if (sourceList.langage == "cpp")
                                  part.setFlagsForCxx({cxxToolChain, flags.args});
                              else if (sourceList.langage == "c")
                                  part.setFlagsForC({cToolChain, flags.args});
                              parts.push_back(part);
                          });
                  });
    return parts;
}
} // namespace Internal
} // namespace MesonProjectManager

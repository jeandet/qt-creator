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
#include <MesonInfoParser/mesoninfoparser.h>
#include <ExeWrappers/mesontools.h>
#include "ProjectTree/mesonprojectnodes.h"
#include "ProjectTree/projecttree.h"
#include <projectexplorer/projectexplorer.h>
#include <coreplugin/messagemanager.h>
#include <utils/optional.h>
#include <utils/runextensions.h>
#include <QStringList>
#include <QTextStream>

namespace MesonProjectManager {
namespace Internal {
static Q_LOGGING_CATEGORY(mesonParserLog, "qtc.meson.buildsystem", QtDebugMsg);


struct CompilerArgs
{
    QStringList args;
    QStringList includePaths;
    ProjectExplorer::Macros macros;
};

inline Utils::optional<QString> extractValueIfMatches(const QString &arg,
                                                      const QStringList &candidates)
{
    for (const auto &flag : candidates) {
        if (arg.startsWith(flag))
            return arg.mid(flag.length());
    }
    return Utils::nullopt;
}

inline Utils::optional<QString> extractInclude(const QString &arg)
{
    return extractValueIfMatches(arg, {"-I", "/I", "-isystem", "-imsvc", "/imsvc"});
}
inline Utils::optional<ProjectExplorer::Macro> extractMacro(const QString &arg)
{
    auto define = extractValueIfMatches(arg, {"-D", "/D"});
    if(define)
        return ProjectExplorer::Macro(define->replace("="," ").toLatin1());
    auto undef = extractValueIfMatches(arg, {"-U", "/U"});
    if(undef)
        return ProjectExplorer::Macro(undef->replace("="," ").toLatin1(), ProjectExplorer::MacroType::Undefine);
    return Utils::nullopt;
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
                // TODO fix this, the goal is to replace name=value with name value
                splited.macros << *macro;
            } else {
                splited.args << arg;
            }
        }
    });
    return splited;
}

QStringList toAbsolutePath(const Utils::FilePath &refPath, QStringList &pathList)
{
    QStringList allAbs;
    std::transform(std::cbegin(pathList),
                   std::cend(pathList),
                   std::back_inserter(allAbs),
                   [refPath](const QString &path) {
                       if (path.startsWith("/"))
                           return path;
                       return refPath.pathAppended(path).toString();
                   });
    return allAbs;
}

MesonProjectParser::MesonProjectParser(const Core::Id &meson, Utils::Environment env)
    : m_env{env}
    , m_meson{meson}
    , m_configuring{false}
{
    connect(&m_process,
            &MesonProcess::finished,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                    startParser();
                } else
                {
                    if(m_introType==IntroDataType::stdo)
                    {
                        auto data = m_process.stdErr();
                        Core::MessageManager::write(QString::fromLatin1(data));
                        m_outputParser.readStdo(data);
                    }
                    emit parsingCompleted(false);
                }
            });
    connect(&m_process,&MesonProcess::readyReadStandardOutput,&m_outputParser,&MesonOutputParser::readStdo);
}

void MesonProjectParser::setMesonTool(const Core::Id &meson)
{
    m_meson = meson;
}

void MesonProjectParser::configure(const Utils::FilePath &sourcePath,
                                   const Utils::FilePath &buildPath,
                                   const QStringList &args)
{
    m_introType = IntroDataType::file;
    m_buildDir = buildPath;
    m_outputParser.setSourceDirectory(sourcePath);
    auto cmd = MesonTools::tool<MesonWrapper>(m_meson)->configure(sourcePath, buildPath, args);
    qCDebug(mesonParserLog) << "Starting:" << cmd.exe << cmd.arguments.join(' ');
    m_process.run(cmd, m_env);
}

void MesonProjectParser::setup(const Utils::FilePath &sourcePath,
                               const Utils::FilePath &buildPath,
                               const QStringList &args)
{
    m_introType = IntroDataType::file;
    m_buildDir = buildPath;
    m_outputParser.setSourceDirectory(sourcePath);
    auto cmdArgs = args;
    if (isSetup(buildPath))
        cmdArgs << "--wipe";
    auto cmd = MesonTools::tool<MesonWrapper>(m_meson)->setup(sourcePath, buildPath, cmdArgs);
    qCDebug(mesonParserLog) << "Starting:" << cmd.exe << cmd.arguments.join(' ');
    m_process.run(cmd, m_env);
}

void MesonProjectParser::parse(const Utils::FilePath &sourcePath, const Utils::FilePath &buildPath)
{
    m_srcDir = sourcePath;
    m_buildDir = buildPath;
    if (!isSetup(buildPath)) {
        parse(sourcePath);
    } else {
        m_introType = IntroDataType::file;
        startParser();
    }
}

void MesonProjectParser::parse(const Utils::FilePath &sourcePath)
{
    m_srcDir = sourcePath;
    m_introType = IntroDataType::stdo;
    m_process.run(MesonTools::tool<MesonWrapper>(m_meson)->introspect(sourcePath), m_env, true);
}

QList<ProjectExplorer::BuildTargetInfo> MesonProjectParser::appsTargets() const
{
    QList<ProjectExplorer::BuildTargetInfo> apps;
    std::for_each(std::cbegin(m_targets), std::cend(m_targets), [&apps](const Target &target) {
        if (target.type == Target::Type::executable) {
            ProjectExplorer::BuildTargetInfo bti;
            bti.displayName = target.name;
            bti.buildKey = Target::fullName(target);
            bti.displayNameUniquifier = bti.buildKey;
            bti.targetFilePath = Utils::FilePath::fromString(target.fileName.first());
            bti.workingDirectory = Utils::FilePath::fromString(target.fileName.first())
                                       .absolutePath();
            bti.usesTerminal = true;
            apps.append(bti);
        }
    });
    return apps;
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
                                             MesonInfoParser parser(process->stdOut());
                                             return extractParserResults(srcDir, parser);
                                         }
                                     });

    Utils::onFinished(m_parserResult, this, &MesonProjectParser::update);
}

MesonProjectParser::ParserData *MesonProjectParser::extractParserResults(
    const Utils::FilePath &srcDir, MesonInfoParser &parser)
{
    auto targets = parser.targets();
    auto buildOptions = parser.buildOptions();
    auto rootNode = ProjectTree::buildTree(srcDir, targets);
    return new ParserData{std::move(targets), std::move(buildOptions), std::move(rootNode)};
}

void MesonProjectParser::addMissingTargets(QStringList &targetList)
{
    // Not all targets are listed in introspection data
    for (const auto target : additionalTargets()) {
        if (!targetList.contains(target)) {
            targetList.append(target);
        }
    }
}

void MesonProjectParser::update(const QFuture<MesonProjectParser::ParserData *> &data)
{
    auto parserData = data.result();
    m_targets = std::move(parserData->targets);
    m_buildOptions = std::move(parserData->buildOptions);
    m_rootNode = std::move(parserData->rootNode);
    m_targetsNames.clear();
    std::transform(std::cbegin(m_targets),
                   std::cend(m_targets),
                   std::back_inserter(m_targetsNames),
                   Target::fullName);
    addMissingTargets(m_targetsNames);
    m_targetsNames.sort();
    delete data;
    emit parsingCompleted(true);
}

ProjectExplorer::RawProjectPart MesonProjectParser::buildRawPart(
    const Target &target,
    const Target::SourceGroup &sources,
    const ProjectExplorer::ToolChain *cxxToolChain,
    const ProjectExplorer::ToolChain *cToolChain)
{
    ProjectExplorer::RawProjectPart part;
    part.setDisplayName(target.name);
    part.setBuildSystemTarget(target.name);
    part.setFiles(sources.sources + sources.generatedSources);
    auto flags = splitArgs(sources.parameters);
    part.setMacros(flags.macros);
    part.setIncludePaths(toAbsolutePath(this->m_buildDir, flags.includePaths));
    if (sources.language == "cpp")
        part.setFlagsForCxx({cxxToolChain, flags.args});
    else if (sources.language == "c")
        part.setFlagsForC({cToolChain, flags.args});
    part.setQtVersion(m_qtVersion);
    return part;
}

ProjectExplorer::RawProjectParts MesonProjectParser::buildProjectParts(
    const ProjectExplorer::ToolChain *cxxToolChain, const ProjectExplorer::ToolChain *cToolChain)
{
    ProjectExplorer::RawProjectParts parts;
    for_each_source_group(m_targets,
                          [&parts,
                           &cxxToolChain,
                           &cToolChain,
                           this](const Target &target, const Target::SourceGroup &sourceList) {
                              parts.push_back(
                                  buildRawPart(target, sourceList, cxxToolChain, cToolChain));
                          });
    return parts;
}

bool sourceGroupMatchesKit(const KitData &kit, const Target::SourceGroup &group)
{
    if (group.language == "c")
        return kit.cCompilerPath == group.compiler[0];
    if (group.language == "cpp")
        return kit.cxxCompilerPath == group.compiler[0];
    return true;
}

bool MesonProjectParser::matchesKit(const KitData &kit)
{
    bool matches = true;
    for_each_source_group(m_targets,
                          [&matches, &kit](const Target &,
                                           const Target::SourceGroup &sourceGroup) {
                              matches = matches && sourceGroupMatchesKit(kit, sourceGroup);
                          });
    return matches;
}

bool MesonProjectParser::usesSameMesonVersion(const Utils::FilePath &buildPath)
{
    auto info = MesonInfoParser::mesonInfo(buildPath.toString());
    auto meson = MesonTools::tool<MesonWrapper>(m_meson);
    return info && meson && info->mesonVersion == meson->version();
}
} // namespace Internal
} // namespace MesonProjectManager

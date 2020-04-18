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
#pragma once
#include "ProjectTree/mesonprojectnodes.h"
#include "mesonprocess.h"
#include <MesonInfoParser/mesoninfoparser.h>
#include <MesonWrapper/mesonwrapper.h>
#include <projectexplorer/buildsystem.h>
#include <projectexplorer/rawprojectpart.h>
#include <utils/fileutils.h>
#include <utils/environment.h>
#include <QFuture>
#include <QFutureWatcher>
#include <QObject>

namespace MesonProjectManager {
namespace Internal {
class MesonProjectParser : public QObject
{
    Q_OBJECT
    enum class IntroDataType { file, stdo };
    struct ParserData
    {
        TargetsList targets;
        BuildOptionsList buildOptions;
        std::unique_ptr<MesonProjectNode> rootNode;
    };

public:
    MesonProjectParser(const Core::Id &meson, Utils::Environment env);
    void setMesonTool(const Core::Id &meson);
    Q_SLOT void configure(const Utils::FilePath &sourcePath,
                          const Utils::FilePath &buildPath,
                          const QStringList &args);
    Q_SLOT void parse(const Utils::FilePath &sourcePath, const Utils::FilePath &buildPath);
    Q_SLOT void parse(const Utils::FilePath &sourcePath);

    Q_SIGNAL void parsingCompleted(bool success);

    std::unique_ptr<MesonProjectNode> takeProjectNode() { return std::move(m_rootNode); }

    inline const BuildOptionsList &buildOptions() const { return m_buildOptions; };
    inline const TargetsList &targets() const { return m_targets; }
    inline const QStringList &targetsNames() const { return m_targetsNames; }

    static inline QStringList additionalTargets()
    {
        return QStringList{Constants::Targets::all,
                           Constants::Targets::clean,
                           Constants::Targets::install,
                           Constants::Targets::benchmark,
                           Constants::Targets::scan_build};
    }

    QList<ProjectExplorer::BuildTargetInfo> appsTargets() const;

    ProjectExplorer::RawProjectParts buildProjectParts(
        const ProjectExplorer::ToolChain *cxxToolChain,
        const ProjectExplorer::ToolChain *cToolChain);

    inline void setEnvironment(const Utils::Environment& environment){m_env = environment;}

private:
    void startParser();
    static ParserData *extractParserResults(const Utils::FilePath &srcDir, MesonInfoParser &parser);
    static void addMissingTargets(QStringList &targetList);
    void update(const QFuture<ParserData *> &data);
    ProjectExplorer::RawProjectPart buildRawPart(const Target &target, const Target::SourceGroup &sources, const ProjectExplorer::ToolChain *cxxToolChain, const ProjectExplorer::ToolChain *cToolChain);
    MesonProcess m_process;

    Utils::Environment m_env;
    Core::Id m_meson;
    Utils::FilePath m_buildDir;
    Utils::FilePath m_srcDir;
    QFuture<ParserData *> m_parserResult;
    bool m_configuring;
    IntroDataType m_introType;
    BuildOptionsList m_buildOptions; // <- Project build settings
    TargetsList m_targets;
    QStringList m_targetsNames;
    std::unique_ptr<MesonProjectNode> m_rootNode; // <- project tree root node
};
} // namespace Internal
} // namespace MesonProjectManager

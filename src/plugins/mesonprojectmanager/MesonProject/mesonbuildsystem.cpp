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

#include "mesonbuildsystem.h"
#include "mesonbuildconfiguration.h"
#include <MesonToolSettings/mesontoolkitaspect.h>
#include <pluginmanager.h>
#include <projectexplorer/buildconfiguration.h>
#include <qtsupport/qtcppkitinfo.h>
#include <qtsupport/qtkitinformation.h>

namespace MesonProjectManager {
namespace Internal {

MesonBuildSystem::MesonBuildSystem(MesonBuildConfiguration *bc)
    : ProjectExplorer::BuildSystem{bc}
    , m_parser{std::make_unique<MesonWrapper>( //TODO fix this hack
          *static_cast<MesonTools *>(
              ExtensionSystem::PluginManager::getObjectByName(Constants::MESON_TOOL_MANAGER))
              ->autoDetected())}
{
    m_tools = static_cast<MesonTools *>(
        ExtensionSystem::PluginManager::getObjectByName(Constants::MESON_TOOL_MANAGER));
    init();
}

void MesonBuildSystem::triggerParsing()
{
    parseProject();
}

void MesonBuildSystem::configure(const Utils::FilePath &buildDir, const QStringList &arguments)
{
    if (m_parseGuard.guardsProject())
        return;
    m_parseGuard = guardParsingRun();
    const auto &srcDir = projectDirectory();
    m_parser.configure(srcDir, buildDir, arguments, Utils::Environment{});
}

void MesonBuildSystem::init()
{
    connect(project(),
            &ProjectExplorer::Project::projectFileIsDirty,
            this,
            &MesonBuildSystem::parseProject);
    connect(&m_parser, &MesonProjectParser::parsingCompleted, this, [this](bool success) {
        m_parseGuard.markAsSuccess();
        m_parseGuard = {};
        if (success) {
            setRootProjectNode(m_parser.takeProjectNode());
            if (kit() && buildConfiguration()) {
                ProjectExplorer::KitInfo kitInfo{kit()};
                m_cppCodeModelUpdater.update(
                    {project(),
                     QtSupport::CppKitInfo(kit()),
                     buildConfiguration()->environment(),
                     m_parser.buildProjectParts(kitInfo.cxxToolChain, kitInfo.cToolChain)});
            }
            emitBuildSystemUpdated();
        }
    });
}

void MesonBuildSystem::parseProject()
{
    if (m_parseGuard.guardsProject())
        return;
    m_parseGuard = guardParsingRun();
    const auto &srcDir = projectDirectory();
    auto bc = buildConfiguration();
    if (bc) {
        const auto &buildDir = bc->buildDirectory();
        m_parser.parse(srcDir, buildDir);
    } else {
        m_parser.parse(srcDir);
    }
}
} // namespace Internal
} // namespace MesonProjectManager

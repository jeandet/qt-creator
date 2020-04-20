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
#include "kithelper.h"
#include "mesonbuildconfiguration.h"
#include <MesonToolSettings/mesontoolkitaspect.h>
#include <MesonWrapper/mesonnativefilegenerator.h>
#include <pluginmanager.h>
#include <projectexplorer/buildconfiguration.h>
#include <qtsupport/qtcppkitinfo.h>
#include <qtsupport/qtkitinformation.h>

#define LEAVE_IF_BUSY() \
    { \
        if (m_parseGuard.guardsProject()) \
            return; \
    }
#define LOCK() \
    { \
        m_parseGuard = guardParsingRun(); \
    }

#define UNLOCK(success) \
    { \
        if (success) \
            m_parseGuard.markAsSuccess(); \
        m_parseGuard = {}; \
    };

namespace MesonProjectManager {
namespace Internal {

MesonBuildSystem::MesonBuildSystem(MesonBuildConfiguration *bc)
    : ProjectExplorer::BuildSystem{bc}
    , m_parser{MesonToolKitAspect::mesonToolId(bc->target()->kit()), bc->environment()}
{
    init();
}

void MesonBuildSystem::triggerParsing()
{
    parseProject();
}

bool MesonBuildSystem::needsSetup()
{
    const Utils::FilePath &buildDir = buildConfiguration()->buildDirectory();
    if (!isSetup(buildDir) || !m_parser.usesSameMesonVersion(buildDir)
        || !m_parser.matchesKit(m_kitData))
        return true;
    return false;
}

void MesonBuildSystem::parsingCompleted(bool success)
{
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
        setApplicationTargets(m_parser.appsTargets());
        UNLOCK(true);
        emitBuildSystemUpdated();
    } else {
        UNLOCK(false);
        emitBuildSystemUpdated();
    }
}

QStringList MesonBuildSystem::configArgs(bool isSetup)
{
    if (!isSetup)
        return m_pendingConfigArgs+mesonBuildConfiguration()->mesonConfigArgs();
    else
        return QStringList{QString("--native-file=%1").arg(m_nativeFile->fileName())}
               + m_pendingConfigArgs+mesonBuildConfiguration()->mesonConfigArgs();
}

void MesonBuildSystem::configure()
{
    LEAVE_IF_BUSY();
    if (needsSetup())
        return setup();
    LOCK();
    m_parser.configure(projectDirectory(),
                       buildConfiguration()->buildDirectory(),
                       configArgs(false));
}

void MesonBuildSystem::setup()
{
    LEAVE_IF_BUSY();
    LOCK();
    m_parser.setup(projectDirectory(), buildConfiguration()->buildDirectory(), configArgs(true));
}

MesonBuildConfiguration *MesonBuildSystem::mesonBuildConfiguration()
{
    return static_cast<MesonBuildConfiguration *>(buildConfiguration());
}

void MesonBuildSystem::init()
{
    auto dir = buildConfiguration()->buildDirectory();
    updateKit(buildConfiguration()->target()->kit());
    connect(buildConfiguration()->target(), &ProjectExplorer::Target::kitChanged, this, [this] {
        updateKit(buildConfiguration()->target()->kit());
    });
    connect(mesonBuildConfiguration(),
            &MesonBuildConfiguration::buildDirectoryChanged,
            this,
            [this]() { this->triggerParsing(); });
    connect(mesonBuildConfiguration(), &MesonBuildConfiguration::environmentChanged, this, [this]() {
        m_parser.setEnvironment(buildConfiguration()->environment());
    });
    connect(project(),
            &ProjectExplorer::Project::projectFileIsDirty,
            this,
            &MesonBuildSystem::parseProject);
    connect(&m_parser, &MesonProjectParser::parsingCompleted, this, &MesonBuildSystem::parsingCompleted);
}

void MesonBuildSystem::parseProject()
{
    LEAVE_IF_BUSY();
    LOCK();
    QTC_ASSERT(buildConfiguration(),return);
    m_parser.parse(projectDirectory(), buildConfiguration()->buildDirectory());
}

void MesonBuildSystem::updateNativeFile()
{
    m_nativeFile = std::make_unique<QTemporaryFile>("Meson-Native");
    m_nativeFile->open();
    MesonNativeFileGenerator::makeNativeFile(m_nativeFile.get(), m_kitData);
    m_nativeFile->flush();
}

void MesonBuildSystem::updateKit(ProjectExplorer::Kit *kit)
{
    QTC_ASSERT(kit, return );
    m_kitData = KitHelper::kitData(kit);
    updateNativeFile();
}
} // namespace Internal
} // namespace MesonProjectManager

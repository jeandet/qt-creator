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
#include "coreplugin/id.h"
#include "projectexplorer/buildconfiguration.h"
#include "projectexplorer/target.h"
#include "utils/fileutils.h"
#include <QString>

namespace MesonProjectManager {
namespace Internal {

enum class MesonBuildType { plain, debug, debugoptimized, release, minsize, custom};

inline QString buildTypeName(MesonBuildType type){
    switch (type)
    {
    case MesonBuildType::plain:
        return "Plain";
    case MesonBuildType::debug:
        return "Debug";
    case MesonBuildType::debugoptimized:
        return "DebugWithOptim";
    case MesonBuildType::release:
        return "Release";
    case MesonBuildType::minsize:
        return "MinSize";
    default:
        return "Custom";
    }
}

inline QString buildTypeDisplayName(MesonBuildType type){
    switch (type)
    {
    case MesonBuildType::plain:
        return "Plain";
    case MesonBuildType::debug:
        return "Debug";
    case MesonBuildType::debugoptimized:
        return "Debug With Optimizations";
    case MesonBuildType::release:
        return "Release";
    case MesonBuildType::minsize:
        return "Minimum Size";
    default:
        return "Custom";
    }
}

inline ProjectExplorer::BuildConfiguration::BuildType buildType(MesonBuildType type){
    switch (type)
    {
    case MesonBuildType::plain:
        return ProjectExplorer::BuildConfiguration::Unknown;
    case MesonBuildType::debug:
        return ProjectExplorer::BuildConfiguration::Debug;
    case MesonBuildType::debugoptimized:
        return ProjectExplorer::BuildConfiguration::Profile;
    case MesonBuildType::release:
        return ProjectExplorer::BuildConfiguration::Release;
    case MesonBuildType::minsize:
        return ProjectExplorer::BuildConfiguration::Release;
    default:
        return ProjectExplorer::BuildConfiguration::Unknown;
    }
}

class MesonBuildSystem;
class MesonTools;

class MesonBuildConfiguration final : public ProjectExplorer::BuildConfiguration
{
    Q_OBJECT
public:
    MesonBuildConfiguration(ProjectExplorer::Target *target, Core::Id id);

    static Utils::FilePath shadowBuildDirectory(const Utils::FilePath &projectFilePath,
                                         const ProjectExplorer::Kit *k,
                                         const QString &bcName,
                                         ProjectExplorer::BuildConfiguration::BuildType buildType);

     ProjectExplorer::BuildSystem *buildSystem() const final;
     void build(const QString& target);

private:
    ProjectExplorer::NamedWidget *createConfigWidget() final;
    MesonBuildSystem* m_buildSystem;

};

class MesonBuildConfigurationFactory final : public ProjectExplorer::BuildConfigurationFactory
{
public:
    MesonBuildConfigurationFactory();
};
} // namespace Internal
} // namespace MesonProjectManager

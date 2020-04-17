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
#include "mesonprojectparser.h"
#include "MesonWrapper/mesonwrapper.h"

#include <projectexplorer/buildsystem.h>
#include <projectexplorer/target.h>
#include <cpptools/cppprojectupdater.h>
#include <utils/filesystemwatcher.h>

namespace MesonProjectManager {
namespace Internal {
class MesonBuildConfiguration;
class MesonBuildSystem final:public ProjectExplorer::BuildSystem
{
public:
    MesonBuildSystem(MesonBuildConfiguration* bc);

    void triggerParsing() final;

    inline const BuildOptionsList& buildOptions()const {return m_parser.buildOptions();}
    inline const TargetsList& targets()const {return m_parser.targets();}

    void configure(const Utils::FilePath &buildDir, const QStringList& arguments);

    const QStringList& targetList()const
    {
        return m_parser.targetsNames();
    }
private:
    void init();
    void parseProject();
    ProjectExplorer::BuildSystem::ParseGuard m_parseGuard;
    MesonProjectParser m_parser;
    CppTools::CppProjectUpdater m_cppCodeModelUpdater;
};
} // namespace Internal
} // namespace MesonProjectManager

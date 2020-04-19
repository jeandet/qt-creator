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
#include <projectexplorer/kit.h>
#include <utils/qtcassert.h>
#include <utils/macroexpander.h>
#include <QString>
#include "kitdata.h"
#pragma once
namespace MesonProjectManager {
namespace Internal {
namespace KitHelper {
namespace  {
QString expand(ProjectExplorer::Kit* kit, const QString& macro)
{
    return  kit->macroExpander()->expand(macro);
}
}

inline QString cCompilerPath(ProjectExplorer::Kit* kit)
{
        QTC_ASSERT(kit, return "");
    return expand(kit, "%{Compiler:Executable:C}");
}

inline QString cxxCompilerPath(ProjectExplorer::Kit* kit)
{
        QTC_ASSERT(kit, return "");
    return expand(kit, "%{Compiler:Executable:Cxx}");
}

inline QString qmakePath(ProjectExplorer::Kit* kit)
{
    return expand(kit, "%{Qt:qmakeExecutable}");
}

inline QString cmakePath(ProjectExplorer::Kit* kit)
{
    return expand(kit, "%{CMake:Executable:FilePath}");
}

inline QString qtVersion(ProjectExplorer::Kit* kit)
{
        QTC_ASSERT(kit, return "");
    return expand(kit, "%{Qt:Version}");
}

inline KitData kitData(ProjectExplorer::Kit* kit)
{
    QTC_ASSERT(kit, return {});
    KitData data;
    data.cCompilerPath = cCompilerPath(kit);
    data.cxxCompilerPath = cxxCompilerPath(kit);
    data.cmakePath = cmakePath(kit);
    data.qmakePath = qmakePath(kit);
    data.qtVersion = qtVersion(kit);
    return data;
}

}

} // namespace Internal
} // namespace MesonProjectManager

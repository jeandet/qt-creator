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
#include "mesonnativefilegenerator.h"
#include <MesonProject/kithelper.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/toolchain.h>
#include <utils/macroexpander.h>
#include <utils/qtcassert.h>

namespace MesonProjectManager {
namespace Internal {
MesonNativeFileGenerator::MesonNativeFileGenerator() {}

inline void addEntrie(QIODevice *nativeFile, const QString &key, const QString &value)
{
    nativeFile->write(QString("%1 = '%2'\n").arg(key).arg(value).toLatin1());
}

void writeBinariesSection(QIODevice *nativeFile, const KitData &kitData)
{
    nativeFile->write("[binaries]\n");
    addEntrie(nativeFile, "c", kitData.cCompilerPath);
    addEntrie(nativeFile, "cpp", kitData.cxxCompilerPath);
    addEntrie(nativeFile, "qmake", kitData.qmakePath);
    if(kitData.qtVersion==Utils::QtVersion::Qt4)
        addEntrie(nativeFile, QString{"qmake-qt4"}, kitData.qmakePath);
    else if(kitData.qtVersion==Utils::QtVersion::Qt5)
        addEntrie(nativeFile, QString{"qmake-qt5"}, kitData.qmakePath);
    addEntrie(nativeFile, "cmake", kitData.cmakePath);
}

void MesonNativeFileGenerator::makeNativeFile(QIODevice *nativeFile, const KitData &kitData)
{
    QTC_ASSERT(nativeFile, return );
    writeBinariesSection(nativeFile, kitData);
}
} // namespace Internal
} // namespace MesonProjectManager

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
#include "MesonWrapper/mesonwrapper.h"
#include "MesonInfoParser/mesoninfoparser.h"
#include "mesonprocess.h"
#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include "utils/fileutils.h"
#include "projectexplorer/buildsystem.h"

namespace MesonProjectManager {
namespace Internal {
class MesonProjectParser: public QObject
{
    Q_OBJECT
    enum class IntroDataType{file,stdo};
public:
    MesonProjectParser(const MesonWrapper& meson);
    Q_SLOT void configure(const Utils::FilePath& sourcePath, const Utils::FilePath& buildPath, const QStringList& args ,const Utils::Environment& env);
    Q_SLOT void parse(const Utils::FilePath& sourcePath, const Utils::FilePath& buildPath);
    Q_SLOT void parse(const Utils::FilePath& sourcePath);

    Q_SIGNAL void parsingCompleted(bool success);

    inline const BuildOptionsList& buildOptions()const {return m_buildOptions;};
    inline const TargetsList& targets()const {return m_targets;}
private:
    void startParser();
    void getParserResults(MesonInfoParser &parser);
    BuildOptionsList m_buildOptions;
    TargetsList m_targets;
    MesonProcess m_process;
    MesonWrapper m_meson;
    IntroDataType m_introType;
    Utils::FilePath m_buildDir;
    bool m_configuring;
};
} // namespace Internal
} // namespace MesonProjectManager

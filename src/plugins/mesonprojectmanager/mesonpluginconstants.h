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

namespace MesonProjectManager {
namespace Constants {

const char MIMETYPE[] = "text/x-meson";
namespace Project {
const char MIMETYPE[] = "text/x-meson-project";
const char ID[] = "MesonProjectManager.MesonProject";
} // namespace Project

// Settings page
const char MESON_SETTINGSPAGE_ID[] = "Z.Meson";

namespace Settings {
const char FILENAME[] = "mesontools.xml";
const char ENTRY_KEY[] = "Meson.";
const char ENTRY_COUNT[] = "Meson.Count";
const char EXE_KEY[] = "exe";
const char AUTO_DETECTED_KEY[] = "autodetected";
const char NAME_KEY[] = "name";
const char ID_KEY[] = "uuid";
} // namespace Settings

const char MESON_INFO_DIR[] = "meson-info";
const char MESON_INTRO_BENCHMARKS[] = "intro-benchmarks.json";
const char MESON_INTRO_BUIDOPTIONS[] = "intro-buildoptions.json";
const char MESON_INTRO_BUILDSYSTEM_FILES[] = "intro-buildsystem_files.json";
const char MESON_INTRO_DEPENDENCIES[] = "intro-dependencies.json";
const char MESON_INTRO_INSTALLED[] = "intro-installed.json";
const char MESON_INTRO_PROJECTINFO[] = "intro-projectinfo.json";
const char MESON_INTRO_TARGETS[] = "intro-targets.json";
const char MESON_INTRO_TESTS[] = "intro-tests.json";
const char MESON_INFO[] = "meson-info.json";


const char MESON_TOOL_MANAGER[] = "MesonProjectManager.Tools";

const char MESON_BUILD_STEP_ID[] = "MesonProjectManager.BuildStep";
namespace Targets {
const char all[] = "all";
const char clean[] = "clean";
const char install[] = "install";
const char tests[] = "test";
const char benchmark[] = "benchmark";
const char clang_format[] = "clang-format";
const char scan_build[] = "scan-build";
} // namespace Targets
const char MESON_BUILD_CONFIG_ID[] = "MesonProjectManager.BuildConfiguration";

} // namespace Constants
} // namespace MesonProjectManager

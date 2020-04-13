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

#include "mesonwrapper.h"
#include "utils/algorithm.h"
#include "utils/qtcassert.h"
#include <QUuid>

namespace {
template<typename First>
void impl_option_cat(QStringList &list, const First &first)
{
    list.append(first);
}

template<typename First, typename... T>
void impl_option_cat(QStringList &list, const First &first, const T &... args)
{
    impl_option_cat(list, first);
    impl_option_cat(list, args...);
}

template<typename... T>
QStringList options_cat(const T &... args)
{
    QStringList result;
    impl_option_cat(result, args...);
    return result;
}
bool run_meson(const QString &meson_exe, const QStringList &options, QIODevice *output = nullptr)
{
    QProcess process;
    process.start(meson_exe, options);
    if (!process.waitForFinished())
        return false;
    if (output) {
        output->write(process.readAllStandardOutput());
    }
    return process.exitCode() == 0;
}

} // namespace
namespace MesonProjectManager {
namespace Internal {

MesonWrapper::MesonWrapper(const QString &name, const Utils::FilePath &path, bool autoDetected)
    : m_version(read_version(path))
    , m_isValid{path.exists() && m_version.isValid}
    , m_autoDetected{autoDetected}
    , m_id{Core::Id::fromString(QUuid::createUuid().toString())}
    , m_exe{path}
    , m_name{name}
{}

MesonWrapper::MesonWrapper(const QString &name,
                           const Utils::FilePath &path,
                           const Core::Id &id,
                           bool autoDetected)
    : m_version(read_version(path))
    , m_isValid{path.exists() && m_version.isValid}
    , m_autoDetected{autoDetected}
    , m_id{id}
    , m_exe{path}
    , m_name{name}
{
    QTC_ASSERT(m_id.isValid(), m_id = Core::Id::fromString(QUuid::createUuid().toString()));
}

bool MesonWrapper::setup(const Utils::FilePath &sourceDirectory,
                         const Utils::FilePath &buildDirectory,
                         const QStringList &options) const
{
    return run_meson(m_exe.toString(),
                     options_cat("setup",
                                 options,
                                 sourceDirectory.toString(),
                                 buildDirectory.toString()));
}

bool MesonWrapper::configure(const Utils::FilePath &sourceDirectory,
                             const Utils::FilePath &buildDirectory,
                             const QStringList &options) const
{
    if (!isSetup(buildDirectory))
        return setup(sourceDirectory, buildDirectory, options);
    return run_meson(m_exe.toString(), options_cat("configure", options, buildDirectory.toString()));
}

bool MesonWrapper::introspect(const Utils::FilePath &sourceDirectory, QIODevice *introFile) const
{
    return run_meson(m_exe.toString(),
                     {"introspect","-a", QString("%1/meson.build").arg(sourceDirectory.toString())},
                     introFile);
}

void MesonWrapper::setExe(const Utils::FilePath &newExe)
{
    m_exe = newExe;
    m_version = read_version(m_exe);
}

void MesonTools::updateItem(const Core::Id &itemId, const QString &name, const Utils::FilePath &exe)
{
    auto item = std::find_if(std::begin(m_tools), std::end(m_tools), [&itemId](const auto &tool) {
        return tool.id() == itemId;
    });
    if (item != std::end(m_tools)) {
        item->setExe(exe);
        item->setName(name);
    } else {
        m_tools.emplace_back(name, exe, itemId);
    }
}

void MesonTools::removeItem(const Core::Id &id)
{
    auto item = Utils::take(m_tools, [&id](const auto &item) { return item.id() == id; });
    QTC_ASSERT(item, return );
    emit mesonToolRemoved(*item);
}

} // namespace Internal
} // namespace MesonProjectManager

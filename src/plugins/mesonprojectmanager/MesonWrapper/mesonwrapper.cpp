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

MesonCommand MesonWrapper::setup(const Utils::FilePath &sourceDirectory,
                                 const Utils::FilePath &buildDirectory,
                                 const QStringList &options) const
{
    return {m_exe,
            sourceDirectory,
            options_cat("setup", options, sourceDirectory.toString(), buildDirectory.toString())};
}

MesonCommand MesonWrapper::configure(const Utils::FilePath &sourceDirectory,
                                     const Utils::FilePath &buildDirectory,
                                     const QStringList &options) const
{
    if (!isSetup(buildDirectory))
        return setup(sourceDirectory, buildDirectory, options);
    return {m_exe, buildDirectory, options_cat("configure", options, buildDirectory.toString())};
}

MesonCommand MesonWrapper::introspect(const Utils::FilePath &sourceDirectory) const
{
    return {m_exe,
            sourceDirectory,
            {"introspect", "-a", QString("%1/meson.build").arg(sourceDirectory.toString())}};
}

void MesonWrapper::setExe(const Utils::FilePath &newExe)
{
    m_exe = newExe;
    m_version = read_version(m_exe);
}

void MesonTools::updateTool(const Core::Id &itemId, const QString &name, const Utils::FilePath &exe)
{
    auto self = instance();
    auto item = std::find_if(std::begin(self->m_tools),
                             std::end(self->m_tools),
                             [&itemId](const auto &tool) { return tool.id() == itemId; });
    if (item != std::end(self->m_tools)) {
        item->setExe(exe);
        item->setName(name);
    } else {
        self->m_tools.emplace_back(name, exe, itemId);
        emit self->mesonToolAdded(self->m_tools.back());
    }
}

void MesonTools::removeTool(const Core::Id &id)
{
    auto self = instance();
    auto item = Utils::take(self->m_tools, [&id](const auto &item) { return item.id() == id; });
    QTC_ASSERT(item, return );
    emit self->mesonToolRemoved(*item);
}

} // namespace Internal
} // namespace MesonProjectManager

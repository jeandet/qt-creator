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
#include "../mesonpluginconstants.h"
#include "utils/environment.h"
#include "utils/fileutils.h"
#include "utils/optional.h"
#include <coreplugin/id.h>
#include <tuple>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QProcess>
#include <QTemporaryFile>



namespace MesonProjectManager {
namespace Internal {
struct MesonCommand
{
    Utils::FilePath exe;
    Utils::FilePath workDir;
    QStringList arguments;
};


namespace {

template<typename File_t>
bool containsFiles(const QString &path, const File_t &file)
{
    return QFile::exists(QString("%1/%2").arg(path).arg(file));
}

template<typename File_t, typename... T>
bool containsFiles(const QString &path, const File_t &file, const T &... files)
{
    return containsFiles(path, file) && containsFiles(path, files...);
}
} // namespace

inline bool run_meson(const MesonCommand& command, QIODevice *output = nullptr)
{
    QProcess process;
    process.setWorkingDirectory(command.workDir.toString());
    process.start(command.exe.toString(), command.arguments);
    if (!process.waitForFinished())
        return false;
    if (output) {
        output->write(process.readAllStandardOutput());
    }
    return process.exitCode() == 0;
}

inline Utils::optional<Utils::FilePath> findMeson()
{
    using namespace Utils;
    Environment systemEnvironment = Environment::systemEnvironment();
    const FilePath meson_path = systemEnvironment.searchInPath("meson");
    if (meson_path.exists())
        return meson_path;
    return Utils::nullopt;
}

inline bool isSetup(const Utils::FilePath &buildPath)
{
    using namespace Utils;
    return containsFiles(buildPath.pathAppended(Constants::MESON_INFO_DIR).toString(),
                         Constants::MESON_INTRO_TESTS,
                         Constants::MESON_INTRO_TARGETS,
                         Constants::MESON_INTRO_INSTALLED,
                         Constants::MESON_INTRO_BENCHMARKS,
                         Constants::MESON_INTRO_BUIDOPTIONS,
                         Constants::MESON_INTRO_PROJECTINFO,
                         Constants::MESON_INTRO_DEPENDENCIES,
                         Constants::MESON_INTRO_BUILDSYSTEM_FILES);
}

struct MesonVersion
{
    int major = -1;
    int minor = -1;
    int patch = -1;
    bool isValid = false;
    MesonVersion() = default;
    MesonVersion(const MesonVersion &) = default;
    MesonVersion(MesonVersion &&) = default;
    MesonVersion &operator=(const MesonVersion &) = default;
    MesonVersion &operator=(MesonVersion &&) = default;

    MesonVersion(int major, int minor, int patch)
        : major{major}
        , minor{minor}
        , patch{patch}
        , isValid{major != -1 && minor != -1 && patch != -1}
    {}
    QString toQString() const noexcept
    {
        return QString("%1.%2.%3").arg(major).arg(minor).arg(patch);
    }
};

class MesonWrapper
{
public:
    MesonWrapper() = delete;
    MesonWrapper(const QString &name, const Utils::FilePath &path, bool m_autoDetected = false);
    MesonWrapper(const QString &name,
                 const Utils::FilePath &path,
                 const Core::Id &id,
                 bool m_autoDetected = false);
    MesonWrapper(const MesonWrapper &other) = default;
    MesonWrapper(MesonWrapper &&other) = default;
    MesonWrapper &operator=(const MesonWrapper &other) = default;
    MesonWrapper &operator=(MesonWrapper &&other) = default;

    MesonCommand setup(const Utils::FilePath &sourceDirectory,
               const Utils::FilePath &buildDirectory,
               const QStringList &options = {}) const;
    MesonCommand configure(const Utils::FilePath &sourceDirectory,
                   const Utils::FilePath &buildDirectory,
                   const QStringList &options = {}) const;

    MesonCommand introspect(const Utils::FilePath &sourceDirectory) const;

    inline const auto &version() const noexcept { return m_version; };
    inline auto isValid() const noexcept { return m_isValid; };
    inline auto autoDetected() const noexcept { return m_autoDetected; };
    inline auto id() const noexcept { return m_id; };
    inline auto exe() const noexcept { return m_exe; };
    inline auto name() const noexcept { return m_name; };

    void setName(const QString &newName) { m_name = newName; }
    void setExe(const Utils::FilePath &newExe);

    friend QVariantMap toVariantMap(const MesonWrapper &);
    friend MesonWrapper fromVariantMap(const QVariantMap &);

    inline static MesonVersion read_version(const Utils::FilePath &mesonPath)
    {
        if (mesonPath.toFileInfo().isExecutable()) {
            QProcess process;
            process.start(mesonPath.toString(), {"--version"});
            if (process.waitForFinished()) {
                const QStringList version = QString::fromUtf8(process.readLine()).split('.');
                return MesonVersion{version[0].toInt(), version[1].toInt(), version[2].toInt()};
            }
        }
        return {};
    }

private:
    MesonVersion m_version;
    bool m_isValid;
    bool m_autoDetected;
    Core::Id m_id;
    Utils::FilePath m_exe;
    QString m_name;
};

inline QVariantMap toVariantMap(const MesonWrapper &meson)
{
    QVariantMap data;
    data.insert(Constants::Settings::NAME_KEY, meson.m_name);
    data.insert(Constants::Settings::EXE_KEY, meson.m_exe.toVariant());
    data.insert(Constants::Settings::AUTO_DETECTED_KEY, meson.m_autoDetected);
    data.insert(Constants::Settings::ID_KEY, meson.m_id.toSetting());
    return data;
}

inline MesonWrapper fromVariantMap(const QVariantMap &data)
{
    return MesonWrapper(data[Constants::Settings::NAME_KEY].toString(),
                        Utils::FilePath::fromVariant(data[Constants::Settings::EXE_KEY]),
                        Core::Id::fromSetting(data[Constants::Settings::ID_KEY]),
                        data[Constants::Settings::AUTO_DETECTED_KEY].toBool());
}

class MesonTools : public QObject
{
    Q_OBJECT
    MesonTools() { setObjectName(Constants::MESON_TOOL_MANAGER); }
    ~MesonTools() {}
public:
    static inline void addTool(MesonWrapper &&meson)
    {
        auto self = instance();
        self->m_tools.emplace_back(std::move(meson));
        emit self->mesonToolAdded(self->m_tools.back());
    }
    static inline void setTools(std::vector<MesonWrapper> &&mesonTools)
    {
        auto self = instance();
        std::swap(self->m_tools, mesonTools);
        auto hasAutoDetected = std::accumulate(std::cbegin(self->m_tools),
                                               std::cend(self->m_tools),
                                               false,
                                               [](bool prev, const MesonWrapper &tool) {
                                                   return prev || tool.autoDetected();
                                               });
        if (!hasAutoDetected) {
            auto meson_path = findMeson();
            if (meson_path) {
                self->m_tools.emplace_back(QString("System Meson at %1").arg(meson_path->toString()),
                                     *meson_path,
                                     true);
            }
        }
    }

    static inline const std::vector<MesonWrapper> &tools() { return instance()->m_tools; }

    static void updateTool(const Core::Id &itemId, const QString &name, const Utils::FilePath &exe);
    static void removeTool(const Core::Id &id);

    static Utils::optional<const MesonWrapper &> autoDetected()
    {
        auto self = instance();
        const auto tool = std::find_if(std::cbegin(self->m_tools),
                                       std::cend(self->m_tools),
                                       [](const MesonWrapper &tool) { return tool.autoDetected(); });
        if (tool != std::cend(self->m_tools))
            return *tool;
        return Utils::nullopt;
    }

    static Utils::optional<const MesonWrapper &> tool(const Core::Id &id)
    {
        auto self = instance();
        const auto tool = std::find_if(std::cbegin(self->m_tools),
                                       std::cend(self->m_tools),
                                       [&id](const MesonWrapper &tool) { return tool.id() == id; });
        if (tool != std::cend(self->m_tools))
            return *tool;
        return Utils::nullopt;
    }

    Q_SIGNAL void mesonToolAdded(const MesonWrapper &tool);
    Q_SIGNAL void mesonToolRemoved(const MesonWrapper &tool);

    static MesonTools* instance()
    {
        static MesonTools inst;
        return &inst;
    }
private:
    std::vector<MesonWrapper> m_tools;
};

} // namespace Internal
} // namespace MesonProjectManager

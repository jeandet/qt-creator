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
#include <coreplugin/id.h>
#include <utils/fileutils.h>
#include <utils/qtcassert.h>
#include <utils/environment.h>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QUuid>
#include <QVariant>
#include <QVariantMap>
#include <versionhelper.h>

namespace MesonProjectManager {
namespace Internal {



struct Command
{
    Utils::FilePath exe;
    Utils::FilePath workDir;
    QStringList arguments;
};



class ToolWrapper
{
public:
    virtual ~ToolWrapper(){};
    ToolWrapper() = delete;
    ToolWrapper(const QString &name, const Utils::FilePath &path, bool autoDetected = false);
    ToolWrapper(const QString &name,
                const Utils::FilePath &path,
                const Core::Id &id,
                bool autoDetected = false);
    ToolWrapper(const ToolWrapper &other) = default;
    ToolWrapper(ToolWrapper &&other) = default;
    ToolWrapper &operator=(const ToolWrapper &other) = default;
    ToolWrapper &operator=(ToolWrapper &&other) = default;

    inline const auto &version() const noexcept { return m_version; };
    inline bool isValid() const noexcept { return m_isValid; };
    inline auto autoDetected() const noexcept { return m_autoDetected; };
    inline auto id() const noexcept { return m_id; };
    inline auto exe() const noexcept { return m_exe; };
    inline auto name() const noexcept { return m_name; };

    inline void setName(const QString &newName) { m_name = newName; }
    virtual void setExe(const Utils::FilePath &newExe);

    inline static Version read_version(const Utils::FilePath &toolPath)
    {
        if (toolPath.toFileInfo().isExecutable()) {
            QProcess process;
            process.start(toolPath.toString(), {"--version"});
            if (process.waitForFinished()) {
                return Version::fromString(QString::fromUtf8(process.readLine()));
            }
        }
        return {};
    }

    static inline Utils::optional<Utils::FilePath> findTool(const QStringList& exeNames)
    {
        using namespace Utils;
        Environment systemEnvironment = Environment::systemEnvironment();
        for(const auto& exe:exeNames)
        {
            const FilePath exe_path = systemEnvironment.searchInPath(exe);
            if (exe_path.exists())
                return exe_path;
        }
        return Utils::nullopt;
    }

    template<typename T>
    friend QVariantMap toVariantMap(const T &);
    template<typename T>
    friend T fromVariantMap(const QVariantMap &);

protected:
    Version m_version;
    bool m_isValid;
    bool m_autoDetected;
    Core::Id m_id;
    Utils::FilePath m_exe;
    QString m_name;
};

template<typename T>
QVariantMap toVariantMap(const T &);
template<typename T>
T* fromVariantMap(const QVariantMap &);

} // namespace Internal
} // namespace MesonProjectManager

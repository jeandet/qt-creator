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
#include <MesonWrapper/mesontools.h>
#include "coreplugin/id.h"
#include "mesonsettingpage.h"
#include "utils/fileutils.h"
#include "utils/treemodel.h"
#include "utils/optional.h"
#include <QCoreApplication>
#include <QString>

namespace MesonProjectManager {
namespace Internal {
class MesoneToolTreeItem final : public Utils::TreeItem
{
    Q_DECLARE_TR_FUNCTIONS(MesonProjectManager::Internal::MesonSettingsPage)
public:
    MesoneToolTreeItem(const QString&name);
    MesoneToolTreeItem(const MesonTools::Tool_t &tool);
    MesoneToolTreeItem(const MesoneToolTreeItem &other);
    QVariant data(int column, int role) const override;
    inline bool isAutoDetected()  const noexcept { return m_autoDetected; }
    inline QString name()  const noexcept { return m_name; }
    inline Utils::FilePath executable()  const noexcept { return m_executable; }
    inline Core::Id id()  const noexcept { return m_id; }
    inline bool hasUnsavedChanges() const noexcept {return m_unsavedChanges;}
    inline void setSaved(){m_unsavedChanges=false;}
    void update(const QString& name, const Utils::FilePath& exe);

private:
    void self_check();
    void update_tooltip(const Version& version);
    void update_tooltip();
    QString m_name;
    QString m_tooltip;
    Utils::FilePath m_executable;
    bool m_autoDetected;
    bool m_pathExists;
    bool m_pathIsFile;
    bool m_pathIsExecutable;
    Core::Id m_id;

    bool m_unsavedChanges = false;
};
} // namespace Internal
} // namespace MesonProjectManager

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

#include "mesonsettingpage.h"
#include <utils/treemodel.h>
#include <QCoreApplication>
#include <memory>
#include "../MesonWrapper/mesonwrapper.h"
#include <QQueue>

namespace MesonProjectManager {
namespace Internal {
class MesoneToolTreeItem;
class MesonToolModel final: public Utils::TreeModel<Utils::TreeItem, Utils::TreeItem, MesoneToolTreeItem>
{
    Q_DECLARE_TR_FUNCTIONS(MesonProjectManager::Internal::MesonSettingsPage)
public:
    MesonToolModel();
    MesoneToolTreeItem *mesoneToolTreeItem(const QModelIndex &index) const;
    Q_SLOT void updateItem(const Core::Id& itemId,const QString& name, const Utils::FilePath& exe);
    void addMesonTool();
    void removeMesonTool(MesoneToolTreeItem * item);
    MesoneToolTreeItem * cloneMesonTool(MesoneToolTreeItem * item);
    void apply();
private:
    void addMesonTool(const MesonWrapper&);
    QString uniqueName(const QString& baseName);
    Utils::TreeItem* autoDetectedGroup() const;
    Utils::TreeItem* manualGroup() const;
    QQueue<Core::Id> m_itemsToRemove;
};
} // namespace Internal
} // namespace MesonProjectManager

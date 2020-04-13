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
#include "MesonInfoParser/mesoninfoparser.h"
#include <utils/treemodel.h>
#include <QAbstractTableModel>
#include <QObject>

namespace MesonProjectManager {
namespace Internal {
class BuidOptionsModel final : public Utils::TreeModel<>
{
    Q_OBJECT
public:
    explicit BuidOptionsModel(QObject *parent = nullptr);

    void setConfiguration(const BuildOptionsList &options);
private:
    BuildOptionsList m_options;
};

template<typename option_type>
class BuildOptionTreeItem final : public Utils::TreeItem
{
    option_type* m_option;
public:
    BuildOptionTreeItem(option_type *option)
    {
        m_option=option;
    }
    QVariant data(int column, int role) const final
    {
        // TODO
        return {};
    };
    bool setData(int column, const QVariant &data, int role) final
    {
        // TODO
        return false;
    };
    Qt::ItemFlags flags(int column) const final
    {
        // TODO
        return Qt::NoItemFlags;
    }

    QString toolTip() const { return m_option->description; }
    QString currentValue() const { return m_option->value(); }
};

} // namespace Internal
} // namespace MesonProjectManager

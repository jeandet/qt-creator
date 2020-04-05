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

#include "mesontoolsettingswidget.h"
#include "mesonetooltreeitem.h"
#include "mesontoolmodel.h"
#include "ui_mesontoolsettingswidget.h"

namespace MesonProjectManager {
namespace Internal {
MesonToolSettingsWidget::MesonToolSettingsWidget(const std::shared_ptr<MesonTools> &tools)
    : Core::IOptionsPageWidget()
    , ui(new Ui::MesonToolSettingsWidget)
    , m_model{tools}
{
    ui->setupUi(this);
    ui->mesonDetails->setState(Utils::DetailsWidget::NoSummary);
    ui->mesonDetails->setVisible(false);
    m_itemSettings = new MesonToolItemSettings;
    ui->mesonDetails->setWidget(m_itemSettings);

    ui->mesonList->setModel(&m_model);
    ui->mesonList->expandAll();
    ui->mesonList->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->mesonList->header()->setSectionResizeMode(1, QHeaderView::Stretch);

    connect(ui->mesonList->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            &MesonToolSettingsWidget::currentMesonToolChanged);
    connect(m_itemSettings,
            &MesonToolItemSettings::applyChanges,
            &m_model,
            &MesonToolModel::updateItem);

    connect(ui->addButton,
            &QPushButton::clicked,
            &m_model,
            qOverload<>(&MesonToolModel::addMesonTool));
    connect(ui->cloneButton,
            &QPushButton::clicked,
            this,
            &MesonToolSettingsWidget::cloneMesonTool);
    connect(ui->removeButton,
            &QPushButton::clicked,
            this,
            &MesonToolSettingsWidget::removeMesonTool);
}

MesonToolSettingsWidget::~MesonToolSettingsWidget()
{
    delete ui;
}

void MesonToolSettingsWidget::cloneMesonTool()
{
    if(m_currentItem)
    {
        auto newItem = m_model.cloneMesonTool(m_currentItem);
        ui->mesonList->setCurrentIndex(newItem->index());
    }
}

void MesonToolSettingsWidget::removeMesonTool()
{
    if(m_currentItem)
    {
        m_model.removeMesonTool(m_currentItem);
    }
}

void MesonToolSettingsWidget::currentMesonToolChanged(const QModelIndex &newCurrent)
{
    m_currentItem = m_model.mesoneToolTreeItem(newCurrent);
    m_itemSettings->load(m_currentItem);
    ui->mesonDetails->setVisible(m_currentItem);
    ui->cloneButton->setEnabled(m_currentItem);
    ui->removeButton->setEnabled(m_currentItem && !m_currentItem->isAutoDetected());
}

void MesonToolSettingsWidget::apply()
{
    m_model.apply();
}
} // namespace Internal
} // namespace MesonProjectManager

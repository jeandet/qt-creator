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
#include "projectexplorer/buildaspects.h"
#include "projectexplorer/projectconfiguration.h"
#include "utils/detailswidget.h"

#include "mesonbuildconfiguration.h"
#include "mesonbuildsettingswidget.h"
#include "mesonbuildsystem.h"
#include "ui_mesonbuildsettingswidget.h"

namespace MesonProjectManager {
namespace Internal {
MesonBuildSettingsWidget::MesonBuildSettingsWidget(MesonBuildConfiguration *buildCfg)
    : ProjectExplorer::NamedWidget{tr("Meson")}
    , ui{new Ui::MesonBuildSettingsWidget}
{
    ui->setupUi(this);
    ui->container->setState(Utils::DetailsWidget::NoSummary);
    ui->container->setWidget(ui->details);
    ProjectExplorer::LayoutBuilder buildDirWBuilder{ui->buildDirWidget};
    auto buildDirAspect = buildCfg->buildDirectoryAspect();
    buildDirAspect->addToLayout(buildDirWBuilder);
    ui->optionsFilterLineEdit->setFiltering(true);
    ui->optionsTreeView->sortByColumn(0, Qt::AscendingOrder);
    ui->optionsTreeView->setModel(&m_optionsModel);

    MesonBuildSystem *bs = static_cast<MesonBuildSystem *>(buildCfg->buildSystem());
    connect(bs, &MesonBuildSystem::parsingFinished, [this, bs](bool success) {
        if (success)
            m_optionsModel.setConfiguration(bs->buildOptions());
    });
    bs->triggerParsing();
}

MesonBuildSettingsWidget::~MesonBuildSettingsWidget()
{
    delete ui;
}

} // namespace Internal
} // namespace MesonProjectManager

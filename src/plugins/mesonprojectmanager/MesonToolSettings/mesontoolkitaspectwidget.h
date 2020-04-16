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

#include "MesonWrapper/mesonwrapper.h"
#include "mesontoolkitaspect.h"
#include "projectexplorer/kitmanager.h"
#include <QCoreApplication>
#include <QComboBox>
#include <QPushButton>
#include <memory>

namespace MesonProjectManager {
namespace Internal {
class MesonToolKitAspectWidget final : public ProjectExplorer::KitAspectWidget
{
    Q_DECLARE_TR_FUNCTIONS(MesonProjectManager::Internal::MesonToolKitAspect)
public:
    MesonToolKitAspectWidget(ProjectExplorer::Kit *kit,
                             const ProjectExplorer::KitAspect *ki);
    ~MesonToolKitAspectWidget();
private:
    void addMesonTool(const MesonWrapper& tool);
    void removeMesonTool(const MesonWrapper& tool);
    void setCurrentToolIndex(int index);
    int indexOf(const Core::Id &id);

    void loadTools();
    void setToDefault();

    void makeReadOnly() override { m_toolsComboBox->setEnabled(false); }
    QWidget *mainWidget() const override { return m_toolsComboBox; }
    QWidget *buttonWidget() const override { return m_manageButton; }
    void refresh() override
    {
        const auto id = MesonToolKitAspect::mesonToolId(m_kit);
        if (id.isValid())
            m_toolsComboBox->setCurrentIndex(indexOf(id));
        else {
            setToDefault();
        }
    }

    QComboBox* m_toolsComboBox;
    QPushButton* m_manageButton;
};
} // namespace Internal
} // namespace MesonProjectManager

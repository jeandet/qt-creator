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

#include "mesonprojectplugin.h"
#include "MesonProject/mesonproject.h"
#include "MesonProject/mesonbuildstep.h"
#include "MesonProject/mesonbuildconfiguration.h"
#include "MesonProject/mesonrunconfiguration.h"
#include "MesonProject/mesonbuildsystem.h"
#include "MesonToolSettings/mesonsettingpage.h"
#include "MesonToolSettings/mesontoolkitaspect.h"
#include "MesonToolSettings/mesontoolsettingaccessor.h"
#include "MesonWrapper/mesonwrapper.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/icore.h>

#include <projectexplorer/kitmanager.h>
#include <projectexplorer/projectmanager.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/runcontrol.h>
#include <projectexplorer/target.h>
#include <projectexplorer/runcontrol.h>

#include <extensionsystem/pluginmanager.h>

#include <memory>
#include <utils/parameteraction.h>
#include <QObject>

using namespace Core;
using namespace ProjectExplorer;
using namespace Utils;

namespace MesonProjectManager {
namespace Internal {

class MesonProjectPluginPrivate : public QObject
{
    Q_OBJECT
public:
    ParameterAction buildTargetContextAction{
        MesonProjectPlugin::tr("Build"),
        MesonProjectPlugin:: tr("Build \"%1\""),
        ParameterAction::AlwaysEnabled/*handled manually*/
    };
    MesonProjectPluginPrivate()
    {
        MesonTools::setTools(m_settings.loadMesonTools(ICore::dialogParent()));
        connect(ICore::instance(),
                &ICore::saveSettingsRequested,
                this,
                &MesonProjectPluginPrivate::saveMesonTools);
    }

    ~MesonProjectPluginPrivate()
    {
    }

private:
    MesonSettingsPage m_settingsPage;
    MesonToolSettingAccessor m_settings;
    MesonToolKitAspect m_kitAspect;
    MesonBuildStepFactory m_buildStepFactory;
    MesonBuildConfigurationFactory m_buildConfigurationFactory;
    MesonRunConfigurationFactory m_runConfigurationFactory;
    ProjectExplorer::RunWorkerFactory m_mesonRunWorkerFactory {
        ProjectExplorer::RunWorkerFactory::make<ProjectExplorer::SimpleTargetRunner>(),
        {ProjectExplorer::Constants::NORMAL_RUN_MODE},
        {m_runConfigurationFactory.id()}
    };
    Q_SLOT void saveMesonTools()
    {
        m_settings.saveMesonTools(MesonTools::tools(), ICore::dialogParent());
    }
};

MesonProjectPlugin::~MesonProjectPlugin()
{
    delete d;
}

bool MesonProjectPlugin::initialize(const QStringList & /*arguments*/, QString *errorMessage)
{
    Q_UNUSED(errorMessage)

    d = new MesonProjectPluginPrivate;
    const Context projectContext{Constants::Project::ID};

    ProjectManager::registerProjectType<MesonProject>(Constants::Project::MIMETYPE);
    FileIconProvider::registerIconOverlayForFilename(Constants::Icons::MESON,
                                                     "meson.build");
    FileIconProvider::registerIconOverlayForFilename(Constants::Icons::MESON,
                                                     "meson_options.txt");

    Command *command = ActionManager::registerAction(&d->buildTargetContextAction,
                                                     "CMake.BuildTargetContextMenu", projectContext);
    command->setAttribute(Command::CA_Hide);
    command->setAttribute(Command::CA_UpdateText);
    command->setDescription(d->buildTargetContextAction.text());

    ActionManager::actionContainer(ProjectExplorer::Constants::M_SUBPROJECTCONTEXT)
        ->addAction(command, ProjectExplorer::Constants::G_PROJECT_BUILD);

    // Wire up context menu updates:
    connect(ProjectTree::instance(), &ProjectTree::currentNodeChanged,
            this, &MesonProjectPlugin::updateContextActions);

    connect(&d->buildTargetContextAction, &ParameterAction::triggered, this, [] {
        if (auto bs = qobject_cast<MesonBuildSystem *>(ProjectTree::currentBuildSystem())) {
            auto targetNode = dynamic_cast<MesonTargetNode *>(ProjectTree::currentNode());
            targetNode->build();
        }
    });

    return true;
}

void MesonProjectPlugin::extensionsInitialized() {}

void MesonProjectPlugin::updateContextActions()
{
    auto targetNode = dynamic_cast<const MesonTargetNode *>(ProjectTree::currentNode());
    const QString targetDisplayName = targetNode ? targetNode->displayName() : QString();

    // Build Target:
    d->buildTargetContextAction.setParameter(targetDisplayName);
    d->buildTargetContextAction.setEnabled(targetNode);
    d->buildTargetContextAction.setVisible(targetNode);
}

} // namespace Internal
} // namespace MesonProjectManager

#include "mesonprojectplugin.moc"

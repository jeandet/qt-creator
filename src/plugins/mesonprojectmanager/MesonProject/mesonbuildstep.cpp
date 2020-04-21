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

#include "mesonbuildstep.h"
#include "mesonbuildconfiguration.h"
#include "mesonbuildstepconfigwidget.h"
#include "mesonbuildsystem.h"
#include "mesonpluginconstants.h"
#include <MesonToolSettings/ninjatoolkitaspect.h>
#include <coreplugin/id.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/processparameters.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>

namespace MesonProjectManager {
namespace Internal {
const char TARGETS_KEY[] = "MesonProjectManager.BuildStep.BuildTargets";
const char TOOL_ARGUMENTS_KEY[] = "MesonProjectManager.BuildStep.AdditionalArguments";

MesonBuildStep::MesonBuildStep(ProjectExplorer::BuildStepList *bsl, Core::Id id)
    : ProjectExplorer::AbstractProcessStep{bsl, id}
{
    setDefaultDisplayName(tr("Meson Build"));
    if (m_targetName.isEmpty())
        setBuildTarget(defaultBuildTarget());
    setLowPriority();
    connect(target(), &ProjectExplorer::Target::parsingFinished, this, &MesonBuildStep::update);
}

ProjectExplorer::BuildStepConfigWidget *MesonBuildStep::createConfigWidget()
{
    return new MesonBuildStepConfigWidget{this};
}

Utils::CommandLine MesonBuildStep::command()
{
    Utils::CommandLine cmd = [this] {
        auto tool = NinjaToolKitAspect::ninjaTool(target()->kit());
        if (tool)
            return Utils::CommandLine{tool->exe()};
        return Utils::CommandLine{};
    }();
    if (!m_commandArgs.isEmpty())
        cmd.addArgs(m_commandArgs, Utils::CommandLine::RawType::Raw);
    cmd.addArg(m_targetName);
    return cmd;
}

QStringList MesonBuildStep::projectTargets()
{
    return static_cast<MesonBuildSystem *>(buildSystem())->targetList();
}

void MesonBuildStep::update(bool parsingSuccessful)
{
    if (parsingSuccessful) {
        if (!projectTargets().contains(m_targetName)) {
            m_targetName = defaultBuildTarget();
        }
        emit targetListChanged();
    }
}

bool MesonBuildStep::init()
{
    // TODO check if the setup is ok
    MesonBuildConfiguration *bc = static_cast<MesonBuildConfiguration *>(buildConfiguration());
    ProjectExplorer::ProcessParameters *pp = processParameters();
    pp->setMacroExpander(bc->macroExpander());
    Utils::Environment env = bc->environment();
    Utils::Environment::setupEnglishOutput(&env);
    pp->setEnvironment(env);
    pp->setWorkingDirectory(bc->buildDirectory());
    pp->setCommandLine(command());
    pp->resolveAll();

    m_ninjaParser = new NinjaParser;
    setOutputParser(m_ninjaParser);
    ProjectExplorer::IOutputParser *parser = target()->kit()->createOutputParser();
    if (parser)
        appendOutputParser(parser);
    outputParser()->setWorkingDirectory(pp->effectiveWorkingDirectory());

    connect(m_ninjaParser, &NinjaParser::reportProgress, this, [this](int percent) {
        emit progress(percent, QString());
    });

    return AbstractProcessStep::init();
}

QString MesonBuildStep::defaultBuildTarget() const
{
    const ProjectExplorer::BuildStepList *const bsl = stepList();
    QTC_ASSERT(bsl, return {});
    const Core::Id parentId = bsl->id();
    if (parentId == ProjectExplorer::Constants::BUILDSTEPS_CLEAN)
        return Constants::Targets::clean;
    if (parentId == ProjectExplorer::Constants::BUILDSTEPS_DEPLOY)
        return Constants::Targets::install;
    return Constants::Targets::all;
}

void MesonBuildStep::doRun()
{
    AbstractProcessStep::doRun();
}

MesonBuildStepFactory::MesonBuildStepFactory()
{
    registerStep<MesonBuildStep>(Constants::MESON_BUILD_STEP_ID);
    setSupportedProjectType(Constants::Project::ID);
    setDisplayName(MesonBuildStep::tr("meson"));
}

void MesonProjectManager::Internal::MesonBuildStep::setBuildTarget(const QString &targetName)
{
    m_targetName = targetName;
}

void MesonBuildStep::setCommandArgs(const QString &args)
{
    m_commandArgs = args.trimmed();
}

QVariantMap MesonBuildStep::toMap() const
{
    QVariantMap map(AbstractProcessStep::toMap());
    map.insert(TARGETS_KEY, m_targetName);
    map.insert(TOOL_ARGUMENTS_KEY, m_commandArgs);
    return map;
}

bool MesonBuildStep::fromMap(const QVariantMap &map)
{
    m_targetName = map.value(TARGETS_KEY).toString();
    m_commandArgs = map.value(TOOL_ARGUMENTS_KEY).toString();
    return AbstractProcessStep::fromMap(map);
}

} // namespace Internal
} // namespace MesonProjectManager

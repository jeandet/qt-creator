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

#include "ninjabuildstep.h"
#include "mesonbuildconfiguration.h"
#include "BuildOptions/mesonbuildstepconfigwidget.h"
#include "mesonbuildsystem.h"
#include "mesonpluginconstants.h"
#include "OutputParsers/mesonoutputparser.h"
#include <Settings/Tools/KitAspect/ninjatoolkitaspect.h>
#include <Settings/General/settings.h>
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

NinjaBuildStep::NinjaBuildStep(ProjectExplorer::BuildStepList *bsl, Core::Id id)
    : ProjectExplorer::AbstractProcessStep{bsl, id}
{
    setDefaultDisplayName(tr("Meson Build"));
    if (m_targetName.isEmpty())
        setBuildTarget(defaultBuildTarget());
    setLowPriority();
    connect(target(), &ProjectExplorer::Target::parsingFinished, this, &NinjaBuildStep::update);
    connect(Settings::instance(),&Settings::verboseNinjaChanged,this,&NinjaBuildStep::commandChanged);
}

ProjectExplorer::BuildStepConfigWidget *NinjaBuildStep::createConfigWidget()
{
    return new MesonBuildStepConfigWidget{this};
}

Utils::CommandLine NinjaBuildStep::command()
{
    Utils::CommandLine cmd = [this] {
        auto tool = NinjaToolKitAspect::ninjaTool(target()->kit());
        if (tool)
            return Utils::CommandLine{tool->exe()};
        return Utils::CommandLine{};
    }();
    if (!m_commandArgs.isEmpty())
        cmd.addArgs(m_commandArgs, Utils::CommandLine::RawType::Raw);
    if(Settings::verboseNinja())
        cmd.addArg("--verbose");
    cmd.addArg(m_targetName);
    return cmd;
}

QStringList NinjaBuildStep::projectTargets()
{
    return static_cast<MesonBuildSystem *>(buildSystem())->targetList();
}

void NinjaBuildStep::update(bool parsingSuccessful)
{
    if (parsingSuccessful) {
        if (!projectTargets().contains(m_targetName)) {
            m_targetName = defaultBuildTarget();
        }
        emit targetListChanged();
    }
}

bool NinjaBuildStep::init()
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


    return AbstractProcessStep::init();
}

QString NinjaBuildStep::defaultBuildTarget() const
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

void NinjaBuildStep::doRun()
{
    AbstractProcessStep::doRun();
}

void NinjaBuildStep::setupOutputFormatter(Utils::OutputFormatter *formatter)
{
    auto mesonOutputParser = new MesonOutputParser;
    mesonOutputParser->setSourceDirectory(project()->projectDirectory());
    formatter->addLineParser(mesonOutputParser);
    m_ninjaParser = new NinjaParser;
    m_ninjaParser->setSourceDirectory(project()->projectDirectory());
    formatter->addLineParser(m_ninjaParser);
    auto additionalParsers = target()->kit()->createOutputParsers();
    std::for_each(std::cbegin(additionalParsers),std::cend(additionalParsers),[this](const auto parser)
                  {
                      parser->setRedirectionDetector(m_ninjaParser);
                  });
    formatter->addLineParsers(additionalParsers);
    formatter->addSearchDir(processParameters()->effectiveWorkingDirectory());
    AbstractProcessStep::setupOutputFormatter(formatter);

    connect(m_ninjaParser, &NinjaParser::reportProgress, this, [this](int percent) {
        emit progress(percent, QString());
    });
}

MesonBuildStepFactory::MesonBuildStepFactory()
{
    registerStep<NinjaBuildStep>(Constants::MESON_BUILD_STEP_ID);
    setSupportedProjectType(Constants::Project::ID);
    setDisplayName(NinjaBuildStep::tr("meson"));
}

void MesonProjectManager::Internal::NinjaBuildStep::setBuildTarget(const QString &targetName)
{
    m_targetName = targetName;
}

void NinjaBuildStep::setCommandArgs(const QString &args)
{
    m_commandArgs = args.trimmed();
}

QVariantMap NinjaBuildStep::toMap() const
{
    QVariantMap map(AbstractProcessStep::toMap());
    map.insert(TARGETS_KEY, m_targetName);
    map.insert(TOOL_ARGUMENTS_KEY, m_commandArgs);
    return map;
}

bool NinjaBuildStep::fromMap(const QVariantMap &map)
{
    m_targetName = map.value(TARGETS_KEY).toString();
    m_commandArgs = map.value(TOOL_ARGUMENTS_KEY).toString();
    return AbstractProcessStep::fromMap(map);
}

} // namespace Internal
} // namespace MesonProjectManager

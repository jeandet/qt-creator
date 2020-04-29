include(../../qtcreatorplugin.pri)

HEADERS = \
      ExeWrappers/mesontools.h \
      ExeWrappers/mesonwrapper.h \
      ExeWrappers/ninjawrapper.h \
      ExeWrappers/toolwrapper.h \
      KitHelper/kitdata.h \
      KitHelper/kithelper.h \
      MachineFiles/machinefilemanager.h \
      MachineFiles/nativefilegenerator.h \
      MesonActionsManager/mesonactionsmanager.h \
      MesonInfoParser/buildoptions.h \
      MesonInfoParser/mesoninfo.h \
      MesonInfoParser/mesoninfoparser.h \
      MesonInfoParser/parsers/buildoptionsparser.h \
      MesonInfoParser/parsers/buildsystemfilesparser.h \
      MesonInfoParser/parsers/common.h \
      MesonInfoParser/parsers/infoparser.h \
      MesonInfoParser/parsers/targetparser.h \
      MesonInfoParser/target.h \
      Project/BuildOptions/OptionsModel/arrayoptionlineedit.h \
      Project/BuildOptions/OptionsModel/buidoptionsmodel.h \
      Project/BuildOptions/mesonbuildsettingswidget.h \
      Project/BuildOptions/mesonbuildstepconfigwidget.h \
      Project/OutputParsers/mesonoutputparser.h \
      Project/OutputParsers/ninjaparser.h \
      Project/ProjectTree/mesonprojectnodes.h \
      Project/ProjectTree/projecttree.h \
      Project/mesonbuildconfiguration.h \
      Project/mesonbuildsystem.h \
      Project/mesonprocess.h \
      Project/mesonproject.h \
      Project/mesonprojectimporter.h \
      Project/mesonprojectparser.h \
      Project/mesonrunconfiguration.h \
      Project/ninjabuildstep.h \
      Settings/General/generalsettingspage.h \
      Settings/General/generalsettingswidget.h \
      Settings/General/settings.h \
      Settings/Tools/KitAspect/mesontoolkitaspect.h \
      Settings/Tools/KitAspect/ninjatoolkitaspect.h \
      Settings/Tools/KitAspect/toolkitaspectwidget.h \
      Settings/Tools/toolitemsettings.h \
      Settings/Tools/toolsmodel.h \
      Settings/Tools/toolssettingsaccessor.h \
      Settings/Tools/toolssettingspage.h \
      Settings/Tools/toolssettingswidget.h \
      Settings/Tools/tooltreeitem.h \
      mesonpluginconstants.h \
      mesonprojectplugin.h \
      versionhelper.h'


SOURCES = \
    ExeWrappers/mesonwrapper.cpp  \
    ExeWrappers/toolwrapper.cpp  \
    MachineFiles/machinefilemanager.cpp  \
    MachineFiles/nativefilegenerator.cpp  \
    MesonActionsManager/mesonactionsmanager.cpp  \
    Project/BuildOptions/OptionsModel/arrayoptionlineedit.cpp  \
    Project/BuildOptions/OptionsModel/buidoptionsmodel.cpp  \
    Project/BuildOptions/mesonbuildsettingswidget.cpp  \
    Project/BuildOptions/mesonbuildstepconfigwidget.cpp  \
    Project/OutputParsers/mesonoutputparser.cpp  \
    Project/OutputParsers/ninjaparser.cpp  \
    Project/ProjectTree/mesonprojectnodes.cpp  \
    Project/ProjectTree/projecttree.cpp  \
    Project/mesonbuildconfiguration.cpp  \
    Project/mesonbuildsystem.cpp  \
    Project/mesonprocess.cpp  \
    Project/mesonproject.cpp  \
    Project/mesonprojectimporter.cpp  \
    Project/mesonprojectparser.cpp  \
    Project/mesonrunconfiguration.cpp  \
    Project/ninjabuildstep.cpp  \
    Settings/General/generalsettingspage.cpp  \
    Settings/General/generalsettingswidget.cpp  \
    Settings/General/settings.cpp  \
    Settings/Tools/KitAspect/mesontoolkitaspect.cpp  \
    Settings/Tools/KitAspect/ninjatoolkitaspect.cpp  \
    Settings/Tools/KitAspect/toolkitaspectwidget.cpp  \
    Settings/Tools/toolitemsettings.cpp  \
    Settings/Tools/toolsmodel.cpp  \
    Settings/Tools/toolssettingsaccessor.cpp  \
    Settings/Tools/toolssettingspage.cpp  \
    Settings/Tools/toolssettingswidget.cpp  \
    Settings/Tools/tooltreeitem.cpp  \
    mesonprojectplugin.cpp

RESOURCES += ressources.qrc

FORMS += \
   Project/BuildOptions/mesonbuildsettingswidget.ui \
   Project/BuildOptions/mesonbuildstepconfigwidget.ui \
   Settings/General/generalsettingswidget.ui \
   Settings/Tools/toolitemsettings.ui \
   Settings/Tools/toolssettingswidget.ui


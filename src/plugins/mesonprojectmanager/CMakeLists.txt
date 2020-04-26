add_qtc_plugin(MesonProjectManager
  DEPENDS QmlJS
  PLUGIN_DEPENDS Core CppTools ProjectExplorer TextEditor QtSupport
  PLUGIN_RECOMMENDS Designer
  SOURCES
  mesonprojectplugin.cpp
  mesonprojectplugin.h
  versionhelper.h
  MesonActionsManager/mesonactionsmanager.h
  MesonActionsManager/mesonactionsmanager.cpp
  Settings/Tools/toolsmodel.cpp
  Settings/Tools/toolssettingswidget.h
  Settings/Tools/toolssettingswidget.ui
  Settings/Tools/toolssettingswidget.cpp
  Settings/Tools/toolssettingspage.cpp
  Settings/Tools/toolssettingspage.h
  Settings/Tools/toolitemsettings.ui
  Settings/Tools/toolitemsettings.cpp
  Settings/Tools/toolitemsettings.h
  Settings/Tools/tooltreeitem.cpp
  Settings/Tools/tooltreeitem.h
  Settings/Tools/toolsmodel.h
  Settings/Tools/KitAspect/ninjatoolkitaspect.cpp
  Settings/Tools/KitAspect/ninjatoolkitaspect.h
  Settings/Tools/KitAspect/toolkitaspectwidget.h
  Settings/Tools/KitAspect/toolkitaspectwidget.cpp
  Settings/Tools/KitAspect/mesontoolkitaspect.cpp
  Settings/Tools/KitAspect/mesontoolkitaspect.h
  Settings/Tools/toolssettingsaccessor.h
  Settings/Tools/toolssettingsaccessor.cpp
  Settings/General/generalsettingswidget.ui
  Settings/General/generalsettingswidget.cpp
  Settings/General/generalsettingswidget.h
  Settings/General/generalsettingspage.h
  Settings/General/generalsettingspage.cpp
  Settings/General/settings.h
  Settings/General/settings.cpp
  ExeWrappers/mesonwrapper.cpp
  ExeWrappers/mesonwrapper.h
  ExeWrappers/ninjawrapper.h
  ExeWrappers/toolwrapper.h
  ExeWrappers/toolwrapper.cpp
  ExeWrappers/mesontools.h
  MesonInfoParser/mesoninfoparser.h
  MesonInfoParser/mesoninfoparser.cpp
  KitHelper/kitdata.h
  KitHelper/kithelper.h
  Project/mesonproject.h
  Project/mesonproject.cpp
  Project/mesonprojectimporter.h
  Project/mesonprojectimporter.cpp
  Project/mesonbuildsystem.h
  Project/mesonbuildsystem.cpp
  Project/mesonprojectparser.h
  Project/mesonprojectparser.cpp
  Project/mesonbuildconfiguration.h
  Project/mesonbuildconfiguration.cpp
  Project/ninjabuildstep.h
  Project/ninjabuildstep.cpp
  Project/mesonbuildstepconfigwidget.ui
  Project/mesonbuildstepconfigwidget.h
  Project/mesonbuildstepconfigwidget.cpp
  Project/mesonbuildsettingswidget.ui
  Project/mesonbuildsettingswidget.h
  Project/mesonbuildsettingswidget.cpp
  Project/OptionsModel/buidoptionsmodel.h
  Project/OptionsModel/buidoptionsmodel.cpp
  Project/mesonprocess.h
  Project/mesonprocess.cpp
  Project/mesonoutputparser.h
  Project/mesonoutputparser.cpp
  Project/ninjaparser.h
  Project/ninjaparser.cpp
  Project/mesonrunconfiguration.h
  Project/mesonrunconfiguration.cpp
  Project/ProjectTree/projecttree.h
  Project/ProjectTree/projecttree.cpp
  Project/ProjectTree/mesonprojectnodes.h
  Project/ProjectTree/mesonprojectnodes.cpp
  MachineFiles/machinefilemanager.h
  MachineFiles/machinefilemanager.cpp
  MachineFiles/nativefilegenerator.h
  MachineFiles/nativefilegenerator.cpp
  ressources.qrc
)

if(WITH_TESTS)
add_qtc_test(tst_mesonwrapper
  INCLUDES
    BEFORE "."
  DEPENDS
    Qt5::Core Qt5::Test Core
    Utils
  DEFINES
     MESON_SAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Tests/resources"
     MESON_SAMPLES_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
  SOURCES
    Tests/testmesonwrapper.cpp
    ExeWrappers/mesonwrapper.cpp
    ExeWrappers/mesonwrapper.h
    ExeWrappers/ninjawrapper.h
    ExeWrappers/toolwrapper.h
    ExeWrappers/toolwrapper.cpp
    ExeWrappers/mesontools.h
)

add_qtc_test(tst_mesoninfoparser
  INCLUDES
    BEFORE "."
  DEPENDS
    Qt5::Core Qt5::Test Core
    Utils
  DEFINES
     MESON_SAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Tests/resources"
     MESON_SAMPLES_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
  SOURCES
    Tests/testmesoninfoparser.cpp
    ExeWrappers/mesonwrapper.cpp
    ExeWrappers/mesonwrapper.h
    ExeWrappers/ninjawrapper.h
    ExeWrappers/toolwrapper.h
    ExeWrappers/toolwrapper.cpp
    ExeWrappers/mesontools.h
    MesonInfoParser/mesoninfoparser.h
    MesonInfoParser/mesoninfoparser.cpp
)

add_qtc_test(tst_ninjaparser
  INCLUDES
    BEFORE "."
  DEPENDS
    Qt5::Core Qt5::Test Core
    Utils ProjectExplorer
  SOURCES
    Tests/testninjaparser.cpp
    Project/ninjaparser.cpp
)

add_qtc_test(tst_mesonparser
  INCLUDES
    BEFORE "."
  DEFINES
    MESONPARSER_DISABLE_TASKS_FOR_TESTS
  DEPENDS
    Qt5::Core Qt5::Test Core
    Utils ProjectExplorer
  SOURCES
    Tests/testmesonparser.cpp
    Project/mesonoutputparser.cpp
)


endif(WITH_TESTS)

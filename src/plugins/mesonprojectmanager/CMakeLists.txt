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
  Settings/mesonsettingpage.h
  Settings/mesonsettingpage.cpp
  Settings/mesontoolsettingswidget.ui
  Settings/mesontoolsettingswidget.h
  Settings/mesontoolsettingswidget.cpp
  Settings/mesontoolmodel.cpp
  Settings/mesontoolmodel.h
  Settings/mesonetooltreeitem.cpp
  Settings/mesonetooltreeitem.h
  Settings/mesontoolsettingaccessor.cpp
  Settings/mesontoolsettingaccessor.h
  Settings/mesontoolitemsettings.h
  Settings/mesontoolitemsettings.ui
  Settings/mesontoolitemsettings.cpp
  Settings/mesontoolkitaspect.h
  Settings/mesontoolkitaspect.cpp
  Settings/ninjatoolkitaspect.h
  Settings/ninjatoolkitaspect.cpp
  Settings/mesontoolkitaspectwidget.h
  Settings/mesontoolkitaspectwidget.cpp
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
  DEPENDS
    Qt5::Core Qt5::Test Core
    Utils ProjectExplorer
  SOURCES
    Tests/testmesonparser.cpp
    Project/mesonoutputparser.cpp
)


endif(WITH_TESTS)

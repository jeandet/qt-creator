add_qtc_plugin(MesonProjectManager
  DEPENDS QmlJS
  PLUGIN_DEPENDS Core CppTools ProjectExplorer TextEditor QtSupport
  PLUGIN_RECOMMENDS Designer
  SOURCES
  mesonprojectplugin.cpp
  mesonprojectplugin.h
  MesonToolSettings/mesonsettingpage.h
  MesonToolSettings/mesonsettingpage.cpp
  MesonToolSettings/mesontoolsettingswidget.ui
  MesonToolSettings/mesontoolsettingswidget.h
  MesonToolSettings/mesontoolsettingswidget.cpp
  MesonToolSettings/mesontoolmodel.cpp
  MesonToolSettings/mesontoolmodel.h
  MesonToolSettings/mesonetooltreeitem.cpp
  MesonToolSettings/mesonetooltreeitem.h
  MesonToolSettings/mesontoolsettingaccessor.cpp
  MesonToolSettings/mesontoolsettingaccessor.h
  MesonToolSettings/mesontoolitemsettings.h
  MesonToolSettings/mesontoolitemsettings.ui
  MesonToolSettings/mesontoolitemsettings.cpp
  MesonWrapper/mesonwrapper.cpp
  MesonWrapper/mesonwrapper.h
  MesonInfoParser/mesoninfoparser.h
  MesonInfoParser/mesoninfoparser.cpp
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
    MesonWrapper/mesonwrapper.cpp
    MesonWrapper/mesonwrapper.h
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
    MesonWrapper/mesonwrapper.cpp
    MesonWrapper/mesonwrapper.h
    MesonInfoParser/mesoninfoparser.h
    MesonInfoParser/mesoninfoparser.cpp
)
endif(WITH_TESTS)

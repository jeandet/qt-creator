import qbs

Project {
    name: "Plugins"

    references: [
        "android/android.qbs",
        "autotest/autotest.qbs",
        "autotoolsprojectmanager/autotoolsprojectmanager.qbs",
        "baremetal/baremetal.qbs",
        "bazaar/bazaar.qbs",
        "beautifier/beautifier.qbs",
        "bineditor/bineditor.qbs",
        "bookmarks/bookmarks.qbs",
        "boot2qt/boot2qt.qbs",
        "clangcodemodel/clangcodemodel.qbs",
        "clangformat/clangformat.qbs",
        "clangpchmanager/clangpchmanager.qbs",
        "clangrefactoring/clangrefactoring.qbs",
        "clangtools/clangtools.qbs",
        "classview/classview.qbs",
        "clearcase/clearcase.qbs",
        "cmakeprojectmanager/cmakeprojectmanager.qbs",
        "mesonprojectmanager/mesonprojectmanager.qbs",
        "compilationdatabaseprojectmanager/compilationdatabaseprojectmanager.qbs",
        "coreplugin/coreplugin.qbs",
        "coreplugin/images/logo/logo.qbs",
        "cpaster/cpaster.qbs",
        "cpaster/frontend/frontend.qbs",
        "cppcheck/cppcheck.qbs",
        "cppeditor/cppeditor.qbs",
        "cpptools/cpptools.qbs",
        "ctfvisualizer/ctfvisualizer.qbs",
        "cvs/cvs.qbs",
        "debugger/debugger.qbs",
        "debugger/ptracepreload.qbs",
        "designer/designer.qbs",
        "diffeditor/diffeditor.qbs",
        "fakevim/fakevim.qbs",
        "emacskeys/emacskeys.qbs",
        "genericprojectmanager/genericprojectmanager.qbs",
        "git/git.qbs",
        "glsleditor/glsleditor.qbs",
        "helloworld/helloworld.qbs",
        "help/help.qbs",
        "imageviewer/imageviewer.qbs",
        "ios/ios.qbs",
        "languageclient/languageclient.qbs",
        "macros/macros.qbs",
        "marketplace/marketplace.qbs",
        "mcusupport/mcusupport.qbs",
        "mercurial/mercurial.qbs",
        "modeleditor/modeleditor.qbs",
        "nim/nim.qbs",
        "perforce/perforce.qbs",
        "perfprofiler/perfprofiler.qbs",
        "projectexplorer/projectexplorer.qbs",
        "qbsprojectmanager/qbsprojectmanager.qbs",
        "python/python.qbs",
        "qmldesigner/qmldesigner.qbs",
        "qmljseditor/qmljseditor.qbs",
        "qmljstools/qmljstools.qbs",
        "qmlpreview/qmlpreview.qbs",
        "qmlprofiler/qmlprofiler.qbs",
        "qmlprojectmanager/qmlprojectmanager.qbs",
        "qnx/qnx.qbs",
        "qmakeprojectmanager/qmakeprojectmanager.qbs",
        "qtsupport/qtsupport.qbs",
        "remotelinux/remotelinux.qbs",
        "resourceeditor/resourceeditor.qbs",
        "scxmleditor/scxmleditor.qbs",
        "serialterminal/serialterminal.qbs",
        "silversearcher/silversearcher.qbs",
        "subversion/subversion.qbs",
        "tasklist/tasklist.qbs",
        "texteditor/texteditor.qbs",
        "todo/todo.qbs",
        "updateinfo/updateinfo.qbs",
        "valgrind/valgrind.qbs",
        "vcsbase/vcsbase.qbs",
        "webassembly/webassembly.qbs",
        "welcome/welcome.qbs",
        "winrt/winrt.qbs"
    ].concat(project.additionalPlugins)
}

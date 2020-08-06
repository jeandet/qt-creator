add_subdirectory(source)

get_clang_components(clangComponents clangQuery clangIndex clangTooling)

add_qtc_executable(clangpchmanagerbackend
  DEPENDS
    clangrefactoringbackend_lib clangpchmanagerbackend_lib
    libclang Sqlite ClangSupport ${clangComponents}
  SOURCES
    clangpchmanagerbackendmain.cpp
)

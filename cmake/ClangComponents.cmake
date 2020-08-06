find_library(CLAN_CPP_FOUND NAMES clang-cpp)

function(get_clang_components varName components)
  if (CLAN_CPP_FOUND)
    set(${varName} clang-cpp PARENT_SCOPE)
  else()
    set(${varName} ${components} PARENT_SCOPE)
  endif()
endfunction()

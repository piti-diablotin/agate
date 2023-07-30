set(Readline_PREFIX "" CACHE PATH "Readline install prefix")
# Search for the path containing library's headers
find_path(Readline_ROOT
  NAMES 
  include/readline/readline.h
  include/readline/history.h
  HINTS ${Readline_PREFIX}
)

# Search for include directory
find_path(Readline_INCLUDE_DIR
  NAMES 
  readline/readline.h
  readline/history.h
  HINTS ${Readline_ROOT}/include
)

# Search for library
find_library(Readline_LIBRARY
  NAMES readline
  HINTS ${Readline_ROOT}/lib
)

# Conditionally set READLINE_FOUND value
if(Readline_INCLUDE_DIR AND Readline_LIBRARY)
  set(Readline_FOUND TRUE)
else()
  find_library(Readline_LIBRARY NAMES readline)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Readline DEFAULT_MSG 
    Readline_INCLUDE_DIR Readline_LIBRARY )
  mark_as_advanced(Readline_INCLUDE_DIR Readline_LIBRARY)
endif()

# Hide these variables in cmake GUIs
mark_as_advanced(
  Readline_ROOT
  Readline_INCLUDE_DIR
  Readline_LIBRARY
)

if(Readline_FOUND)
  file(STRINGS "${Readline_INCLUDE_DIR}/readline/readline.h" READLINE_H REGEX "define")
  foreach(line ${READLINE_H})
    if(line MATCHES "RL_VERSION_MAJOR[ \t]+([0-9]+)")
      set(Readline_VERSION_MAJOR ${CMAKE_MATCH_1})
    elseif(line MATCHES "RL_VERSION_MINOR[ \t]+([0-9]+)")
      set(Readline_VERSION_MINOR ${CMAKE_MATCH_1})
    endif()
  endforeach()
  set(Readline_VERSION "${Readline_VERSION_MAJOR}.${Readline_VERSION_MINOR}")
  if(Readline_VERSION VERSION_LESS Readline_FIND_VERSION)
    message(FATAL_ERROR "Found version ${Readline_VERSION} < ${Readline_FIND_VERSION}")
  else()
    message(STATUS "Found Readline: ${Readline_LIBRARY} (found version \"${Readline_VERSION}\")")
  endif()
endif()

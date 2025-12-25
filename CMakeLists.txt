cmake_minimum_required(VERSION 3.14)

project(
  agate
  VERSION 1.5.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile_commands.json" FORCE)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-Weffc++ CXX_HAS_WEFF)
  check_cxx_compiler_flag(-Wextra CXX_HAS_WEXTRA)
  check_cxx_compiler_flag(-Wall CXX_HAS_WALL)
  check_cxx_compiler_flag(-Wno-uninitialized CXX_HAS_WNOINITIALIZED)
  check_cxx_compiler_flag(-Werror CXX_HAS_WERROR)
  if(CXX_HAS_WEFF)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Weffc++")
  endif()
  if(CXX_HAS_WEXTRA)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra")
  endif()
  if(CXX_HAS_WALL)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
  endif()
  if(CXX_HAS_WNOINITIALIZED)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-uninitialized")
  endif()
  if(CXX_HAS_WERROR)
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror")
  endif()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(FetchContent)
include(GNUInstallDirs)

find_package(Git)
if(Git_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
endif()

find_package(Threads) # Threads::Threads
find_package(OpenMP 4.0 COMPONENTS CXX) # OpenMP::OpenMP_CXX

find_package(Readline 6.3) # Readline_INCLUDE_DIR Readline_LIBRARY
find_package(CURL) # CURL::libcurl

find_package(netCDF) # netCDF::netcdf
if(NOT netCDF_FOUND)
  FetchContent_Declare(
    netCDF 
    GIT_REPOSITORY https://github.com/Unidata/netcdf-c.git
    GIT_TAG v4.9.2
  )
  FetchContent_MakeAvailable(netCDF)
endif()

find_package(LibXml2) # LibXml2::LibXml2
find_package(Freetype) # Freetype::Freetype
find_package(Eigen3 NO_MODULE) # EIGEN3_INCLUDE_DIR
if(Eigen3_VERSION VERSION_LESS "3.3")
  message(WARNING "Eigen ≥ 3.3 required (found ${Eigen3_VERSION})")
    set(Eigen3_FOUND FALSE)
endif()
if(NOT Eigen3_FOUND)
  message(STATUS "Fetching eigen3")
  FetchContent_Declare(
    Eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 3.4
  )
  FetchContent_MakeAvailable(Eigen3)
endif()
find_package(Spglib 2.0 QUIET) # Spglib::symspg 
if(NOT Spglib_FOUND)
  message(STATUS "Fetching Spglib")
  FetchContent_Declare(
    Spglib
    GIT_REPOSITORY https://github.com/spglib/spglib.git
    GIT_TAG v2.1.0
  )
  set(SPGLIB_SHARED_LIBS OFF)
  FetchContent_MakeAvailable(Spglib)
endif()
find_package(yaml-cpp) # yaml-cpp::yaml-cpp
if(NOT yaml-cpp_FOUND)
  message(STATUS "Fetching yaml-cpp")
  FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG  yaml-cpp-0.7.0
  )
  set(YAML_BUILD_SHARED_LIBS OFF)
  set(YAML_CPP_INSTALL OFF)
  FetchContent_MakeAvailable(yaml-cpp)
endif()

#find_package(FFTW3 COMPONENTS DOUBLE) # FFTW3::fftw3q Not working on old ubuntu
pkg_check_modules(FFTW_DOUBLE REQUIRED fftw3)
add_library(FFTW::fftw3 INTERFACE IMPORTED)
target_include_directories(FFTW::fftw3 INTERFACE ${FFTW_DOUBLE_INCLUDE_DIRS})
target_link_libraries(FFTW::fftw3 INTERFACE ${FFTW_DOUBLE_LIBRARIES})
target_compile_options(FFTW::fftw3 INTERFACE ${FFTW_DOUBLE_CFLAGS_OTHER})
set(FFTW3_FOUND TRUE)

find_package(libssh) # ssh:ssh

find_package(OpenGL COMPONENTS OpenGL) # OpenGL::GL
find_package(glfw3 3.3) # glfw
find_package(JPEG) # JPEG::JPEG
find_package(PNG) # PNG::PNG

get_property(importTargetsAfter DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
list(REMOVE_ITEM importTargetsAfter ${importTargets})

message("${importTargetsAfter}")

include(CheckIncludeFile)
check_include_file_cxx("getopt.h" GETOPT_H)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)

add_executable(agate
  bin/agate.cpp
  bin/winglfw3.cpp 
  bin/winfake.cpp 
)

target_link_libraries(agate
  PRIVATE
  agate::agate
  $<$<BOOL:${glfw3_FOUND}>:glfw>
)

target_compile_definitions(agate
  PRIVATE
  PACKAGE="${PROJECT_NAME}"
  PACKAGE_STRING="${PROJECT_NAME} ${PROJECT_VERSION}"
  $<$<BOOL:${glfw3_FOUND}>:HAVE_GLFW3>
  $<$<BOOL:${glfw3_FOUND}>:HAVE_GLFW3_DROP>
)

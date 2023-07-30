cmake_minimum_required(VERSION 3.14)

project(
  agate
  VERSION 1.3.4
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

find_program(GIT_EXE git)
if(GIT_EXE)
  execute_process(
    COMMAND ${GIT_EXE} log -1 --format=%h
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
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
find_package(Eigen3 3.3 NO_MODULE) # EIGEN3_INCLUDE_DIR
if(NOT Eigen3_FOUND)
  FetchContent_Declare(
    Eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 3.4
  )
  FetchContent_MakeAvailable(Eigen3)
endif()
find_package(Spglib 1.15.0 COMPONENTS static QUIET) # Spglib::symspg 
if(NOT Spglib_FOUND)
  FetchContent_Declare(
    Spglib
    GIT_REPOSITORY https://github.com/spglib/spglib.git
    GIT_TAG v2.0.2
  )
  set(SPGLIB_SHARED_LIBS OFF)
  FetchContent_MakeAvailable(Spglib)
endif()
find_package(yaml-cpp) # yaml-cpp::yaml-cpp
if(NOT yaml-cpp_FOUND)
  FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG  yaml-cpp-0.7.0
  )
  set(YAML_BUILD_SHARED_LIBS OFF)
  set(YAML_CPP_INSTALL OFF)
  FetchContent_MakeAvailable(yaml-cpp)
endif()
find_package(FFTW3) # FFTW3::fftw3l FFTW3::fftw3l_omp
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

set(LIBAGATE_SRC
  src/base/exception.cpp
  src/base/fraction.cpp
  src/base/geometry.cpp
  src/base/mendeleev.cpp
  src/base/phys.cpp
  src/base/ssh.cpp
  src/base/unitconverter.cpp
  src/base/uriparser.cpp
  src/base/utils.cpp
  src/bind/findsym.cpp
  src/bind/snake.cpp
  src/bind/tdep.cpp
  src/canvas/canvas.cpp
  src/canvas/canvasdensity.cpp
  src/canvas/canvaslocal.cpp
  src/canvas/canvasphonons.cpp
  src/canvas/canvaspos.cpp
  src/conducti/abiopt.cpp
  src/conducti/conducti.cpp
  src/graphism/imagesaver.cpp
  src/graphism/render.cpp
  src/graphism/textrender.cpp
  src/graphism/triarrow.cpp
  src/graphism/tricloud.cpp
  src/graphism/tricube.cpp
  src/graphism/tricylinder.cpp
  src/graphism/trimap.cpp
  src/graphism/triobj.cpp
  src/graphism/trisphere.cpp
  src/hist/histcustommodes.cpp
  src/hist/histdata.cpp
  src/hist/histdatadtset.cpp
  src/hist/histdatagsr.cpp
  src/hist/histdataheff.cpp
  src/hist/histdatamd.cpp
  src/hist/histdatanc.cpp
  src/hist/histdataoutnc.cpp
  src/hist/histdatapolydtset.cpp
  src/hist/histdataxyz.cpp
  src/hist/multibinit.cpp
  src/hist/vaspxml.cpp
  src/io/abibin.cpp
  src/io/abihdr.cpp
  src/io/cifparser.cpp
  src/io/configparser.cpp
  src/io/ddbabinit.cpp
  src/io/ddb.cpp
  src/io/ddboutcar.cpp
  src/io/ddbphonopy.cpp
  src/io/dtset.cpp
  src/io/eigparser.cpp
  src/io/eigparsereig.cpp
  src/io/eigparserelectrons.cpp
  src/io/eigparserfatbands.cpp
  src/io/eigparsergsr.cpp
  src/io/eigparserphbst.cpp
  src/io/eigparserphfrq.cpp
  src/io/eigparserphonons.cpp
  src/io/eigparserphonopy.cpp
  src/io/electrondos.cpp
  src/io/etsfnc.cpp
  src/io/outcar.cpp
  src/io/parser.cpp
  src/io/phonopydtset.cpp
  src/io/poscar.cpp
  src/io/procar.cpp
  src/phonons/dispdb.cpp
  src/phonons/phononmode.cpp
  src/phonons/phononmode.cpp.orig
  src/phonons/supercell.cpp
  src/plot/dosdb.cpp
  src/plot/gnuplot.cpp
  src/plot/graph.cpp
  src/shape/octaangles.cpp
  src/shape/octahedra.cpp
  src/shape/octalengths.cpp
  src/window/window.cpp
)

set(LIBAGATE_HEADERS
  include/plot/graph.hpp
  include/plot/dosdb.hpp
  include/plot/gnuplot.hpp
  include/base/utils.hpp
  include/base/eigen.hpp
  include/base/mendeleev.hpp
  include/base/fraction.hpp
  include/base/ssh.hpp
  include/base/uriparser.hpp
  include/base/exception.hpp
  include/base/phys.hpp
  include/base/win32.hpp
  include/base/typename.hpp
  include/base/unitconverter.hpp
  include/base/geometry.hpp
  include/phonons/dispdb.hpp
  include/phonons/phononmode.hpp
  include/phonons/supercell.hpp
  include/canvas/canvaspos.hpp
  include/canvas/canvasdensity.hpp
  include/canvas/canvaslocal.hpp
  include/canvas/canvasphonons.hpp
  include/canvas/canvas.hpp
  include/bind/tdep.hpp
  include/bind/findsym.hpp
  include/hist/histdatagsr.hpp
  include/hist/histcustommodes.hpp
  include/hist/histdataheff.hpp
  include/hist/histdatamd.hpp
  include/hist/histdatanc.hpp
  include/hist/histdata.hpp
  include/hist/histdatapolydtset.hpp
  include/hist/histdataxyz.hpp
  include/hist/multibinit.hpp
  include/hist/histdataoutnc.hpp
  include/hist/vaspxml.hpp
  include/hist/histdatadtset.hpp
  include/window/winfake.hpp
  include/window/window.hpp
  include/window/winglfw2.hpp
  include/window/winglfw3.hpp
  include/graphism/tricloud.hpp
  include/graphism/tricube.hpp
  include/graphism/imagesaver.hpp
  include/graphism/triarrow.hpp
  include/graphism/triobj.hpp
  include/graphism/trimap.hpp
  include/graphism/tricylinder.hpp
  include/graphism/render.hpp
  include/graphism/buffer.hpp
  include/graphism/trisphere.hpp
  include/graphism/textrender.hpp
  include/shape/octahedra.hpp
  include/shape/octaangles.hpp
  include/shape/octalengths.hpp
  include/conducti/abiopt.hpp
  include/conducti/conducti.hpp
  include/io/ddb.hpp
  include/io/procar.hpp
  include/io/eigparserphbst.hpp
  include/io/abihdr.hpp
  include/io/ddbphonopy.hpp
  include/io/configparser.hpp
  include/io/eigparserphonopy.hpp
  include/io/abibin.hpp
  include/io/etsfnc.hpp
  include/io/eigparserfatbands.hpp
  include/io/electrondos.hpp
  include/io/cifparser.hpp
  include/io/poscar.hpp
  include/io/eigparsereig.hpp
  include/io/eigparserphfrq.hpp
  include/io/ddbabinit.hpp
  include/io/phonopydtset.hpp
  include/io/eigparser.hpp
  include/io/eigparsergsr.hpp
  include/io/outcar.hpp
  include/io/dtset.hpp
  include/io/ddboutcar.hpp
  include/io/eigparserelectrons.hpp
  include/io/parser.hpp
  include/io/yaml.hpp
  include/io/eigparserphonons.hpp
)

add_library(libagate 
  ${LIBAGATE_SRC}
)
add_library(agate::agate ALIAS libagate)
set_property(TARGET libagate PROPERTY OUTPUT_NAME agate)

target_include_directories(libagate
  PRIVATE
  $<$<BOOL:${Readline_FOUND}>:${Readline_INCLUDE_DIR}>
  ${EIGEN3_INCLUDE_DIR}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(libagate
  PRIVATE
  COMPILE_BUILD="${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}"
  COMPILE_HOST=""
  COMPILE_CPPFLAGS="${CMAKE_CXX_FLAGS}"
  COMPILE_CXXFLAGS="${CMAKE_CXX_FLAGS}"
  COMPILE_LDFLAGS="${CMAKE_CXX_FLAGS}"
  COMPILE_LIBS="${CMAKE_CXX_FLAGS}"
  DEFAULT_FONT="/usr/share/fonts/noto/NotoSans-Regular.ttf"
  PACKAGE_NAME="${PROJECT_NAME}"
  PACKAGE_VERSION="${PROJECT_VERSION}"
  PACKAGE_STRING="${PROJECT_NAME} ${PROJECT_VERSION}"
  VERSION="${PROJECT_VERSION}"
  HAVE_NETCDF
  HAVE_EIGEN
  HAVE_SPGLIB
  HAVE_YAMLCPP
  $<$<BOOL:${GIT_HASH}>:HASH_VERSION="${GIT_HASH}">
  $<$<BOOL:${GETOPT_H}>:HAVE_GETOPT_H>
  $<$<VERSION_GREATER_EQUAL:CURL_VERSION_STRING,"7.85.0">:CURL_7850>
  $<$<BOOL:${Threads_FOUND}>:HAVE_CPPTHREAD>
  $<$<BOOL:${OpenMP_FOUND}>:HAVE_OMP>
  $<$<BOOL:${Readline_FOUND}>:HAVE_READLINE>
  $<$<BOOL:${CURL_FOUND}>:HAVE_CURL>
  $<$<BOOL:${LibXml2_FOUND}>:HAVE_LIBXML2>
  $<$<BOOL:${Freetype_FOUND}>:HAVE_FREETYPE>
  $<$<BOOL:${FFTW3_FOUND}>:HAVE_FFTW3>
  $<$<BOOL:${libssh_FOUND}>:HAVE_SSH>
  $<$<BOOL:${JPEG_FOUND}>:HAVE_LIBJPEG>
  $<$<BOOL:${PNG_FOUND}>:HAVE_LIBPNG>
  PUBLIC
  $<$<BOOL:${OpenGL_FOUND}>:HAVE_GL>
  $<$<BOOL:${OpenGL_FOUND}>:HAVE_GLEXT>
  _NOEXCEPT=noexcept
)

target_link_libraries(libagate
  PUBLIC
  netCDF::netcdf
  symspg_static
  yaml-cpp
  $<$<BOOL:${Threads_FOUND}>:Threads::Threads>
  $<$<BOOL:${OpenMP_FOUND}>:OpenMP::OpenMP_CXX>
  $<$<BOOL:${Readline_FOUND}>:${Readline_LIBRARY}>
  $<$<BOOL:${CURL_FOUND}>:CURL::libcurl>
  $<$<BOOL:${LibXml2_FOUND}>:LibXml2::LibXml2>
  $<$<BOOL:${Freetype_FOUND}>:Freetype::Freetype>
  $<$<BOOL:${FFTW3_FOUND}>:FFTW3::fftw3q>
  $<$<BOOL:${libssh_FOUND}>:ssh>
  $<$<BOOL:${OpenGL_FOUND}>:OpenGL::GL>
  $<$<BOOL:${JPEG_FOUND}>:JPEG::JPEG>
  $<$<BOOL:${PNG_FOUND}>:PNG::PNG>
)

add_executable(agate
  bin/agate.cpp
  src/window/winfake.cpp
  src/window/winglfw3.cpp 
)

target_link_libraries(agate
  PRIVATE
  libagate
  symspg_static
  $<$<BOOL:${glfw3_FOUND}>:glfw>
)

target_compile_definitions(agate
  PUBLIC
  PACKAGE="${PROJECT_NAME}"
  PACKAGE_STRING="${PROJECT_NAME} ${PROJECT_VERSION}"
  $<$<BOOL:${glfw3_FOUND}>:HAVE_GLFW3>
)

set(LIBAGATE_SRC)

add_subdirectory(base)
add_subdirectory(bind)
add_subdirectory(canvas)
add_subdirectory(conducti)
add_subdirectory(graphism)
add_subdirectory(hist)
add_subdirectory(io)
add_subdirectory(phonons)
add_subdirectory(plot)
add_subdirectory(shape)
add_subdirectory(window)

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
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(libagate
  PUBLIC
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
  $<$<BOOL:${glfw3_FOUND}>:HAVE_GLFW3>
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
  $<$<BOOL:${glfw3_FOUND}>:glfw>
)


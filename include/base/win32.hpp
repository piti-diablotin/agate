/**
 * @file include/win32.hpp
 *
 * @brief Define some macros for windows ....
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of AbiOut.
 *
 * AbiOut is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AbiOut is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AbiOut.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef WIN32_HPP
#define WIN32_HPP

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#if defined(WIN32) || defined(_WIN32)
#  ifndef __func__
#    define __func__ __FUNCTION__
#  endif
#  define COMPILE_HOST "Win32"
#  define COMPILE_BUILD "Win32"
#  define COMPILE_CPPFLAGS "Undefined"
#  define COMPILE_CXXFLAGS "Undifined"
#  define COMPILE_LIBS "Undefined"
#  define COMPILE_LDFLAGS "Undefined"
#  ifndef PACKAGE
#    define PACKAGE "abiout"
#  endif
#  ifndef PACKAGE_NAME
#    define PACKAGE_NAME "AbiOut"
#  endif
#  ifndef PACKAGE_STRING
#    define PACKAGE_STRING "AbiOut 0.0.5"
#  endif
#  ifndef PACKAGE_VERSION
#    define PACKAGE_VERSION "0.0.5"
#  endif
//#  define HAVE_CURL
//#  define HAVE_EIGEN
//#  define HAVE_NETCDF
#  define HAVE_GL
#  define HAVE_GLEXT
#  define HAVE_GLU
//#  define HAVE_GLFW3
//#  define HAVE_GLFW3_DROP
//#  define HAVE_LIBPNG
//#  define HAVE_LIBJPEG
//#  define HAVE_LIBXML2
//#  define HAVE_CPPTHREAD
#  define HAVE_OMP
//#  define HAVE_SPGLIB
//#define HAVE_FREETYPE
#  define _CRT_SECURE_NO_WARNINGS
#  include <Windows.h>
typedef long ssize_t;
#define GL_UNSIGNED_INT_8_8_8_8 0x8035
#define GL_MULTISAMPLE 0x809D
#define DEFAULT_FONT "c:\\windows\\fonts\\calibri.ttf"

#define popen _popen
#define pclose _pclose
#endif


#endif  // WIN32_HPP

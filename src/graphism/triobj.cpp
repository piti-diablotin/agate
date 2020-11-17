/**
 * @file src/triobj.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of Agate.
 *
 * Agate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Agate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Agate.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "graphism/triobj.hpp"
#include "base/utils.hpp"
#include <cstring>
#include <algorithm>

#if defined(HAVE_GL) && defined(HAVE_GLEXT)
# ifdef __APPLE__
#  include <OpenGL/glext.h>
# else
#  include <GL/glext.h>
# endif
#endif

//
void TriObj::rgb(double value, TriObj::_float& r, TriObj::_float& g, TriObj::_float& b) {
  r = 0;
  g = 0;
  b = 0;
  if ( value < 0 ) {
    b = 1+4*value;
    if (b<0) b = 0;
  }
  else if ( value >= 0 && value <= 0.25 ) {
    b = 1.;
    g = 4*value;
  }
  else if ( value > 0.25 && value <= 0.5 ) {
    b = 1-4*(value-0.25);
    g = 1;
  }
  else if ( value > 0.5 && value <= 0.75 ) {
    g = 1.;
    r = 4*(value-0.5);

  }
  else if ( value > 0.75 && value <= 1 ) {
    g = 1-4*(value-0.75);
    r = 1;
  }
  else {
    r = 1;
    g = 4*(value-1);
    b = 4*(value-1);
    if (b>1) b = 1;
    if (g>1) g = 1;
  }
}

TriObj::TriObj(bool opengl) :
  _opengl(opengl),
  _unitVertex(nullptr),
  _unitIndex(nullptr),
  _vbos(),
  _division(10),
  _nvertex(0),
  _nindices(0),
  _drawing(FILL),
  _mode(VERTEX)
{
  _vbos[0] = (_uint) -1;
  _vbos[1] = (_uint) -1;
  if ( !_opengl ) return;
#ifdef HAVE_GL
  const GLubyte *cstrver = glGetString(GL_VERSION);
  char strver[500]; 
  int major = -1, minor =-1;
  if ( cstrver != nullptr ) {
    strcpy(strver,(char*)cstrver);
    char *pch;
    pch = strtok(strver,".");
    major = utils::stoi(std::string(pch));
    pch = strtok(nullptr,".");
    minor = utils::stoi(std::string(pch));
  }

  if ( major > 1 ) {
#ifdef HAVE_GLEXT
    _mode = VBO;
    glGenBuffers(2,&_vbos[0]);
#else
    _mode = ARRAY;
#endif
  }
  else {
    if ( minor > 4 ) {
#ifdef HAVE_GLEXT
      _mode = VBO;
      glGenBuffers(2,&_vbos[0]);
#else
      _mode = ARRAY;
#endif
    }
    else {
      _mode = VERTEX;
      _division = 4;
    }
  }
#endif
}

TriObj::TriObj(TriObj &&obj) : 
  _opengl(obj._opengl),
  _unitVertex(obj._unitVertex),
  _unitIndex(obj._unitIndex),
  _vbos(),
  _division(obj._division),
  _nvertex(obj._nvertex),
  _nindices(obj._nindices),
  _drawing(obj._drawing),
  _mode(obj._mode)
{
  _vbos[0] = obj._vbos[0];
  _vbos[1] = obj._vbos[1];
  obj._unitVertex = nullptr;
  obj._unitIndex = nullptr;
  obj._vbos[0] = (_uint) -1;
}

TriObj::TriObj(const TriObj &obj) : 
  _opengl(obj._opengl),
  _unitVertex(nullptr),
  _unitIndex(nullptr),
  _vbos(),
  _division(obj._division),
  _nvertex(obj._nvertex),
  _nindices(obj._nindices),
  _drawing(obj._drawing),
  _mode(obj._mode)
{
  _vbos[0] = (_uint) -1;
  _vbos[1] = (_uint) -1;
  if ( !_opengl ) return;
#ifdef HAVE_GL
  const GLubyte *cstrver = glGetString(GL_VERSION);
  char strver[500]; 
  int major = -1, minor =-1;
  if ( cstrver != nullptr ) {
    strcpy(strver,(char*)cstrver);
    char *pch;
    pch = strtok(strver,".");
    major = utils::stoi(std::string(pch));
    pch = strtok(nullptr,".");
    minor = utils::stoi(std::string(pch));
  }

  if ( major > 1 ) {
#ifdef HAVE_GLEXT
    _mode = VBO;
    glGenBuffers(2,&_vbos[0]);
#else
    _mode = ARRAY;
#endif
  }
  else {
    if ( minor > 4 ) {
#ifdef HAVE_GLEXT
      _mode = VBO;
      glGenBuffers(2,&_vbos[0]);
#else
      _mode = ARRAY;
#endif
    }
    else {
      _mode = VERTEX;
      _division = 4;
    }
  }
#endif
}

//
TriObj::~TriObj() {
  if ( _unitVertex != nullptr ) {
    delete[] _unitVertex;
    _unitVertex = nullptr;
  }
  if ( _unitIndex != nullptr ) {
    delete[] _unitIndex;
    _unitIndex = nullptr;
  }
#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  if ( _mode == VBO && _vbos[0] != (_uint) -1)
    glDeleteBuffers(2,&_vbos[0]);
#endif
}


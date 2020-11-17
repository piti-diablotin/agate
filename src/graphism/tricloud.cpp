/**
 * @file src/trisphere.cpp
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
 * the Free Software Foundation, either version 3 of the License, or * (at your option) any later version.
 *
 * Agate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Agate.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "graphism/tricloud.hpp"
#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>

#if defined(HAVE_GL) && defined(HAVE_GLEXT)
# ifdef __APPLE__
#  include <OpenGL/glext.h>
# else
#  include <GL/glext.h>
# endif
#endif

//
TriCloud::TriCloud(bool opengl) : TriSphere(opengl),
  _unitColor(nullptr),
  _vboColor(-1)
{
#if defined(HAVE_GL) 
  glEnable(GL_POINT_SMOOTH);
#if defined(HAVE_GLEXT)
  glGenBuffers(1,&_vboColor);
#endif
#endif
  this->genUnit();
}

//
TriCloud::TriCloud(TriCloud &&cloud) : TriSphere(std::move(cloud)),
  _unitColor(cloud._unitColor),
  _vboColor(cloud._vboColor)
{
  cloud._vboColor = (_uint) -1;
  cloud._unitColor = nullptr;
}

void TriCloud::genUnit() {
  if ( _unitVertex != nullptr ) {
    delete[] _unitVertex;
  }
  if ( _unitIndex != nullptr ) {
    delete[] _unitIndex;
  }
  if ( _unitColor != nullptr ) {
    delete[] _unitColor;
  }
  if ( !_opengl ) return;

  _nvertex = 0;
  _nindices = _nmax;
  _unitVertex = new _float[3*_nmax]; 
  _unitIndex = new _uint[_nmax];
  _unitColor = new _float[3*_nmax];

  for ( unsigned i = 0 ; i < _nmax ; ++i )
    _unitIndex[i] = i;

#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  if ( _mode == VBO ) {

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(_uint)*_nindices,_unitIndex,GL_STATIC_DRAW);

  }
#endif
}

//
TriCloud::~TriCloud() {
#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  if ( _mode == VBO && _vboColor == (_uint) -1)
    glDeleteBuffers(1,&_vboColor);
#endif
}

void TriCloud::draw(const _float pos[3], const _float color[3], const _float radius) {
  if ( !_opengl ) return;
  _unitVertex[3*_nvertex  ] = pos[0];
  _unitVertex[3*_nvertex+1] = pos[1];
  _unitVertex[3*_nvertex+2] = pos[2];
  _unitColor[3*_nvertex  ] = color[0];
  _unitColor[3*_nvertex+1] = color[1];
  _unitColor[3*_nvertex+2] = color[2];
  ++_nvertex;
  if ( _nvertex >= _nmax ) {
    this->pop();
  }
  (void) radius;
}


void TriCloud::pop() {
  if ( !_opengl ) return;
  //_float distancef[3];
  //_float eye;
  //glGetFloatv(GL_POINT_DISTANCE_ATTENUATION,&distancef[0]);
  //glGetFloatv(GL_CURRENT_RASTER_DISTANCE,&eye);
  //std::clog << "eye" << eye << std::endl;
  //_float dist = (distancef[0]+distancef[1]*eye+distancef[3]*eye*eye);
  //std::clog << distancef[0] << " " << distancef[1] << " " << distancef[2] << std::endl;
  //std::clog << dist << std::endl;
  //glPointSize(40.*std::sqrt(dist));
#ifdef HAVE_GL
  glPointSize(5);
  glDisable(GL_LIGHTING);
  switch(_mode) {
    case VERTEX :
      glBegin(GL_POINTS);
      for( unsigned t = 0 ; t < _nvertex ; ++t ) {
        if ( t==0 || (  _unitColor[3*t] != _unitColor[3*(t-1)] ||_unitColor[3*t+1] != _unitColor[3*(t-1)+1] || _unitColor[3*t+2] != _unitColor[3*(t-1)+2] ) ) 
          glColor3f(_unitColor[3*t],_unitColor[3*t+1],_unitColor[3*t+2]);
        glVertex3f(_unitVertex[3*t],_unitVertex[3*t+1],_unitVertex[3*t+2]);
      }
      glEnd();
      break;
    case ARRAY :
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, _unitVertex);
      glColorPointer(3,GL_FLOAT, 0, _unitColor);
      glDrawElements(GL_POINTS, _nvertex, GL_UNSIGNED_INT, _unitIndex);
      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      break;
#ifdef HAVE_GLEXT
    case VBO :
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);

      glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
      glBufferData(GL_ARRAY_BUFFER,sizeof(_float)*3*_nvertex,_unitVertex,GL_STREAM_DRAW);
      glVertexPointer(3, GL_FLOAT, 0, 0);

      glBindBuffer(GL_ARRAY_BUFFER,_vboColor);
      glBufferData(GL_ARRAY_BUFFER,sizeof(_float)*3*_nvertex,_unitColor,GL_STREAM_DRAW);
      glColorPointer(3,GL_FLOAT, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
      glIndexPointer(GL_UNSIGNED_INT, 0, 0);

      glDrawElements(GL_POINTS, _nvertex, GL_UNSIGNED_INT, 0);

      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);

      glBindBuffer(GL_ARRAY_BUFFER,0); // Release VBO
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
      break;
#endif
  }
  glEnable(GL_LIGHTING);
  _nvertex = 0;
#endif
}

//
void TriCloud::push() {
  if ( !_opengl ) return;
  _nvertex = 0;
}

/**
 * @file src/trisphere.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of AbiOut.
 *
 * AbiOut is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or * (at your option) any later version.
 *
 * AbiOut is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AbiOut.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "graphism/trisphere.hpp"
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
TriSphere::TriSphere(bool opengl) : TriObj(opengl)
{
  this->genUnit();
}

//
TriSphere::TriSphere(TriSphere &&sphere) : TriObj(std::move(sphere))
{
}

void TriSphere::genUnit() {
  const unsigned int nV = (_division+1)*(_division+2)/2;
  const unsigned int quarterV = nV*3;
  const unsigned int quarterI = _division*_division*3;

  if ( _unitVertex != nullptr ) {
    delete[] _unitVertex;
  }
  if ( _unitIndex != nullptr ) {
    delete[] _unitIndex;
  }
  if ( !_opengl ) return;

  _nvertex = nV*8;
  _nindices = _division*_division*3*8;
  _unitVertex = new _float[3*_nvertex]; // 8 quarter of sphere z up and z down
  _unitIndex = new _uint[_nindices];

  const _float dz = 1.0f/(_float)(_division);
  const _float dy = 1.0f/(_float)(_division);

  _uint vertex = 0;
  for( unsigned z = 0 ; z < _division+1 ; ++z ) {
    for ( unsigned y = 0 ; y < _division+1-z ; ++y ){
      _float sx = (_float)1.f-(_float)z*dz-(_float)y*dy; 
      _float sy = (_float)y*dy;
      _float sz = (_float)z*dz;
      const _float inv_norm = 1.f/sqrt(sx*sx+sy*sy+sz*sz);

      _unitVertex[3*vertex  ] = (sx *= inv_norm);
      _unitVertex[3*vertex+1] = (sy *= inv_norm);
      _unitVertex[3*vertex+2] = (sz *= inv_norm);

      _unitVertex[quarterV+3*vertex  ] = -sx;
      _unitVertex[quarterV+3*vertex+1] = sy;
      _unitVertex[quarterV+3*vertex+2] = sz;

      _unitVertex[2*quarterV+3*vertex  ] = -sx;
      _unitVertex[2*quarterV+3*vertex+1] = -sy;
      _unitVertex[2*quarterV+3*vertex+2] = sz;

      _unitVertex[3*quarterV+3*vertex  ] = sx;
      _unitVertex[3*quarterV+3*vertex+1] = -sy;
      _unitVertex[3*quarterV+3*vertex+2] = sz;

      _unitVertex[4*quarterV+3*vertex  ] = sx;
      _unitVertex[4*quarterV+3*vertex+1] = -sy;
      _unitVertex[4*quarterV+3*vertex+2] = -sz;

      _unitVertex[5*quarterV+3*vertex  ] = -sx;
      _unitVertex[5*quarterV+3*vertex+1] = -sy;
      _unitVertex[5*quarterV+3*vertex+2] = -sz;

      _unitVertex[6*quarterV+3*vertex  ] = -sx;
      _unitVertex[6*quarterV+3*vertex+1] = sy;
      _unitVertex[6*quarterV+3*vertex+2] = -sz;

      _unitVertex[7*quarterV+3*vertex  ] = sx;
      _unitVertex[7*quarterV+3*vertex+1] = sy;
      _unitVertex[7*quarterV+3*vertex+2] = -sz;
      ++vertex;
    }
  }
  _uint triangle = 0;
  for( unsigned z = 0 ; z < _division ; ++z ) {
    const _uint line=(z)*(_division+1)-z*(z-1)/2;
    const _uint nextLine=(z+1)*(_division+1)-z*(z+1)/2;
    for ( unsigned y = 0 ; y < _division-z-1 ; ++y ){
      for ( unsigned q = 0 ; q < 8 ; ++q ) {
        _unitIndex[q*quarterI+3*triangle  ] = q*nV+line+y;
        _unitIndex[q*quarterI+3*triangle+1] = q*nV+line+y+1;
        _unitIndex[q*quarterI+3*triangle+2] = q*nV+nextLine+y;
      }
      ++triangle;
      for ( unsigned q = 0 ; q < 8 ; ++q ) {
        _unitIndex[q*quarterI+3*triangle  ] = q*nV+nextLine+y;
        _unitIndex[q*quarterI+3*triangle+1] = q*nV+nextLine+y+1;
        _unitIndex[q*quarterI+3*triangle+2] = q*nV+line+y+1;
      }

      ++triangle;
    }
    for ( unsigned q = 0 ; q < 8 ; ++q ) {
      _unitIndex[q*quarterI+3*triangle  ] = q*nV+nextLine-2;
      _unitIndex[q*quarterI+3*triangle+1] = q*nV+nextLine-1;
      _unitIndex[q*quarterI+3*triangle+2] = q*nV+nextLine+(_division-z-1);
    }
    ++triangle;
  }

#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  if ( _mode == VBO ) {
    glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(_float)*3*_nvertex,_unitVertex,GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(_uint)*_nindices,_unitIndex,GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,0); // Release VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    delete[] _unitVertex;
    delete[] _unitIndex;
    _unitVertex = nullptr;
    _unitIndex = nullptr;
  }
#endif

}

//
TriSphere::~TriSphere() {
}

void TriSphere::draw(const _float pos[3], const _float color[3], const _float radius) {
  static _float prev_color[3] = {-1.,-1.,-1.};
  if ( !_opengl ) return;
#if defined(HAVE_GL)

  glPushMatrix();
  glTranslatef(pos[0],pos[1],pos[2]);
  glScalef(radius,radius,radius);
  if ( color[0]!=prev_color[0] || color[1]!=prev_color[1] || color[2]!=prev_color[2])
    glColor3f(color[0],color[1],color[2]);

  switch(_mode) {
    case VERTEX :
      glBegin(GL_TRIANGLES);
      for( unsigned t = 0 ; t < 8*_division*_division ; ++t ) {
        glNormal3f(_unitVertex[3*_unitIndex[3*t  ]  ],_unitVertex[3*_unitIndex[3*t  ]+1],_unitVertex[3*_unitIndex[3*t  ]+2]);
        glVertex3f(_unitVertex[3*_unitIndex[3*t  ]  ],_unitVertex[3*_unitIndex[3*t  ]+1],_unitVertex[3*_unitIndex[3*t  ]+2]);
        glNormal3f(_unitVertex[3*_unitIndex[3*t+1]  ],_unitVertex[3*_unitIndex[3*t+1]+1],_unitVertex[3*_unitIndex[3*t+1]+2]);
        glVertex3f(_unitVertex[3*_unitIndex[3*t+1]  ],_unitVertex[3*_unitIndex[3*t+1]+1],_unitVertex[3*_unitIndex[3*t+1]+2]);
        glNormal3f(_unitVertex[3*_unitIndex[3*t+2]  ],_unitVertex[3*_unitIndex[3*t+2]+1],_unitVertex[3*_unitIndex[3*t+2]+2]);
        glVertex3f(_unitVertex[3*_unitIndex[3*t+2]  ],_unitVertex[3*_unitIndex[3*t+2]+1],_unitVertex[3*_unitIndex[3*t+2]+2]);
      }
      glEnd();
      break;
    case ARRAY :
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, _unitVertex);
      glNormalPointer(GL_FLOAT, 0, _unitVertex);
      glDrawElements(GL_TRIANGLES, _nindices, GL_UNSIGNED_INT, _unitIndex);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      break;
#ifdef HAVE_GLEXT
    case VBO :
      /*
      glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, 0);
      glNormalPointer(GL_FLOAT, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
      glIndexPointer(GL_UNSIGNED_INT, 0, 0);
      */

      glDrawElements(GL_TRIANGLES, _nindices, GL_UNSIGNED_INT, 0);

      /*
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);

      glBindBuffer(GL_ARRAY_BUFFER,0); // Release VBO
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
      */
      break;
#endif

  }

  glPopMatrix();
#else
  (void) radius;
#endif
}

//
void TriSphere::pop() {
  if ( !_opengl ) return;
#if defined(HAVE_GL)
  switch(_mode) {
#ifdef HAVE_GLEXT
    case VBO :
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);

      glBindBuffer(GL_ARRAY_BUFFER,0); // Release VBO
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
      break;
#endif
    default :
      break;
  }
#endif
}

//
void TriSphere::push() {
  if ( !_opengl ) return;
#if defined(HAVE_GL)
  switch(_mode) {
#ifdef HAVE_GLEXT
    case VBO :
      glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, 0);
      glNormalPointer(GL_FLOAT, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
      glIndexPointer(GL_UNSIGNED_INT, 0, 0);
      break;
#endif
    default :
      break;
  }
#endif
}

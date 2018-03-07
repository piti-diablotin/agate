/**
 * @file src/tricylinder.cpp
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


#include "graphism/tricylinder.hpp"
#include "base/phys.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

#if defined(HAVE_GL) && defined(HAVE_GLEXT)
# ifdef __APPLE__
#  include <OpenGL/glext.h>
# else
#  include <GL/glext.h>
# endif
#endif

//
TriCylinder::TriCylinder(bool opengl) : TriObj(opengl)
{
  this->genUnit();
}

//
TriCylinder::TriCylinder(TriCylinder &&cylinder) : TriObj(std::move(cylinder))
{
}

//
void TriCylinder::genUnit() {
  const unsigned int slices = 4*_division;
  const unsigned int stacks = _division;
  const _float pi = acos(-1.0f);

  if ( _unitVertex != nullptr ) {
    delete[] _unitVertex;
  }
  if ( _unitIndex != nullptr ) {
    delete[] _unitIndex;
  }
  if ( !_opengl ) return;

  _nvertex = slices*(stacks+1);
  _nindices = stacks*2*(slices+1);
  _unitVertex = new _float[2*3*_nvertex]; // 2* for normals;
  _unitIndex = new _uint[_nindices];

  const _float dz = 1.0f/(_float)(stacks);
  const _float dtheta = 2*pi/(_float)(slices);

  for( _uint iz = 0, ivertex = 0; iz < stacks+1 ; ++iz ) {
    const _float z = iz*dz;
    for ( _uint itheta = 0 ; itheta < slices ; ++itheta ){
      const _float theta = itheta*dtheta;
      _float sx = cos(theta);
      _float sy = sin(theta);

      _unitVertex[ivertex*3  ] = sx;
      _unitVertex[ivertex*3+1] = sy;
      _unitVertex[ivertex*3+2] = z;
      ++ivertex;
      _unitVertex[ivertex*3  ] = sx;
      _unitVertex[ivertex*3+1] = sy;
      _unitVertex[ivertex*3+2] = (_float) 0.0;
      ++ivertex;
    }
  }
  for( _uint iz = 0, iindex = 0 ; iz < stacks ; ++iz ) {
    for ( _uint itheta = 0 ; itheta < slices ; ++itheta ){
      _unitIndex[iindex++] = iz*slices+itheta;
      _unitIndex[iindex++] = (iz+1)*slices+itheta;
    }
    _unitIndex[iindex++] = iz*slices;
    _unitIndex[iindex++] = (iz+1)*slices;
  }

#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  if ( _mode == VBO ) {
    glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(_float)*2*3*_nvertex,_unitVertex,GL_STATIC_DRAW);

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
TriCylinder::~TriCylinder() {
  if ( _unitVertex != nullptr ) {
    delete[] _unitVertex;
    _unitVertex = nullptr;
  }
  if ( _unitIndex != nullptr ) {
    delete[] _unitIndex;
    _unitIndex = nullptr;
  }
#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  if ( _mode == VBO )
    glDeleteBuffers(2,&_vbos[0]);
#endif
}

void TriCylinder::draw(const _float radius, const _float height) {
  if ( !_opengl ) return;
#if defined(HAVE_GL)
  glPushMatrix();
  glScalef(radius,radius,height);

  switch(_mode) {
    case VERTEX :
      glBegin(GL_QUAD_STRIP);
      for( _uint index = 0 ; index < _nindices ; ++index ) {
        glNormal3fv(&_unitVertex[6*_unitIndex[index]+3]);
        glVertex3fv(&_unitVertex[6*_unitIndex[index]]);
      }
      glEnd();
      break;
    case ARRAY :
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      glVertexPointer(3, GL_FLOAT, 6*sizeof(_float), _unitVertex);
      glNormalPointer(GL_FLOAT, 6*sizeof(_float), _unitVertex+3);
      glDrawElements(GL_QUAD_STRIP, _nindices, GL_UNSIGNED_INT, _unitIndex);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      break;
#ifdef HAVE_GLEXT
    case VBO :
      /*
      glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      glVertexPointer(3, GL_FLOAT, 6*sizeof(_float), 0);
      //glNormalPointer(GL_FLOAT, 6*sizeof(_float), sizeof(_float)*3);
      glNormalPointer(GL_FLOAT,  6*sizeof(_float), (GLvoid*) (sizeof(_float)*3));

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
      glIndexPointer(GL_UNSIGNED_INT, 0, 0);
      */

      glDrawElements(GL_QUAD_STRIP, _nindices, GL_UNSIGNED_INT, 0);

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
  (void) height;
#endif
}

void TriCylinder::draw(const double start[3], const double end[3], const _float radius) {
  const float n0 = end[0]-start[0];
  const float n1 = end[1]-start[1];
  const float n2 = end[2]-start[2];
  const float nn = sqrt(n0*n0+n1*n1+n2*n2);
  const float inv_nn = 1.f/nn;
  const float nprod = sqrt(n1*n1+n0*n0);
  const float angle = ( (n2<0) ? 180.f+180.f/phys::pi*asin(nprod*inv_nn) : -180.f/phys::pi*asin(nprod*inv_nn) );
  glPushMatrix();
  glTranslatef(start[0],start[1],start[2]);
  if ( nprod > 1e-6 ) {
    glRotatef(angle,n1,-n0,0.0f);
  }
  else {
    glRotatef(angle,1.f,0.f,0.0f);
  }

  this->draw(radius,nn);
  glPopMatrix();
}

//
void TriCylinder::pop() {
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
void TriCylinder::push() {
  if ( !_opengl ) return;
#if defined(HAVE_GL)
  switch(_mode) {
#ifdef HAVE_GLEXT
    case VBO :
      glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      glVertexPointer(3, GL_FLOAT, 6*sizeof(_float), 0);
      //glNormalPointer(GL_FLOAT, 6*sizeof(_float), sizeof(_float)*3);
      glNormalPointer(GL_FLOAT,  6*sizeof(_float), (GLvoid*) (sizeof(_float)*3));

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
      glIndexPointer(GL_UNSIGNED_INT, 0, 0);
      break;
#endif
    default :
      break;
  }
#endif
}

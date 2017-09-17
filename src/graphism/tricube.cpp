/**
 * @file src/tricube.cpp
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


#include "graphism/tricube.hpp"
#include <algorithm>

#if defined(HAVE_GL) && defined(HAVE_GLEXT)
# ifdef __APPLE__
#  include <OpenGL/glext.h>
# else
#  include <GL/glext.h>
# endif
#endif

//
TriCube::TriCube(bool opengl) : TriObj(opengl),
  _nvface((_division+1)*(_division+1)*3*3),
  _niface(_division*((_division+1)*2+2))
{
  this->genUnit();
}

TriCube::TriCube(TriCube &&cube) : TriObj(std::move(cube)),
  _nvface((_division+1)*(_division+1)*3*3),
  _niface(_division*((_division+1)*2+2))
{
}

void TriCube::genUnit() {
  const unsigned int div = _division; 
  _nvface = (_division+1)*(_division+1)*3*3;
  _niface = _division*((_division+1)*2+2);
  _nvertex = 6*_nvface;
  _nindices = 6*_niface;

  if ( _unitVertex != nullptr ) {
    delete[] _unitVertex;
  }
  if ( _unitIndex != nullptr ) {
    delete[] _unitIndex;
  }
  if ( !_opengl ) return;

  const float dl = 1./div;
  const unsigned nvface = _nvface;
  _unitVertex  = new _float[6*_nvface]; // 6 faces of (div+1)**2 vertex 3 coords for Vertex Normal, Color
  _unitIndex = new _uint[6*_niface];

  // Z faces
  for ( unsigned vertex = 0, ix = 0 ; ix < div+1 ; ++ix ) {
    for ( unsigned iy = 0 ; iy < div+1 ; ++iy ) {
      // Vertex
      _unitVertex[9*vertex  ] = -0.5f+(float)ix*dl;
      _unitVertex[9*vertex+1] = -0.5f+(float)iy*dl;
      _unitVertex[9*vertex+2] = -0.5f;
      // Normal
      _unitVertex[9*vertex+3] = 0.f;
      _unitVertex[9*vertex+4] = 0.f;
      _unitVertex[9*vertex+5] = -1.f;
      // Color
      _unitVertex[9*vertex+6] = 0.f;
      _unitVertex[9*vertex+7] = 0.f;
      _unitVertex[9*vertex+8] = 0.f;

      // Vertex
      _unitVertex[nvface+9*vertex  ] = -0.5f+(float)ix*dl;
      _unitVertex[nvface+9*vertex+1] = -0.5f+(float)iy*dl;
      _unitVertex[nvface+9*vertex+2] = 0.5f;
      // Normal
      _unitVertex[nvface+9*vertex+3] = 0.f;
      _unitVertex[nvface+9*vertex+4] = 0.f;
      _unitVertex[nvface+9*vertex+5] = 1.f;
      // Color
      _unitVertex[nvface+9*vertex+6] = 0.f;
      _unitVertex[nvface+9*vertex+7] = 0.f;
      _unitVertex[nvface+9*vertex+8] = 0.f;
      ++vertex;
    }
  }

  // X faces
  for ( unsigned vertex = 0, iz = 0 ; iz < div+1 ; ++iz ) {
    for ( unsigned iy = 0 ; iy < div+1 ; ++iy ) {
      // Vertex
      _unitVertex[nvface*2+9*vertex  ] = -0.5f;
      _unitVertex[nvface*2+9*vertex+1] = -0.5f+(float)iy*dl;
      _unitVertex[nvface*2+9*vertex+2] = -0.5f+(float)iz*dl;
      // Normal
      _unitVertex[nvface*2+9*vertex+3] = -1.f;
      _unitVertex[nvface*2+9*vertex+4] = 0.f;
      _unitVertex[nvface*2+9*vertex+5] = 0.f;
      // Color
      _unitVertex[nvface*2+9*vertex+6] = 0.f;
      _unitVertex[nvface*2+9*vertex+7] = 0.f;
      _unitVertex[nvface*2+9*vertex+8] = 0.f;

      // Vertex
      _unitVertex[nvface*3+9*vertex  ] = 0.5f;
      _unitVertex[nvface*3+9*vertex+1] = -0.5f+(float)iy*dl;
      _unitVertex[nvface*3+9*vertex+2] = -0.5f+(float)iz+dl;
      // Normal
      _unitVertex[nvface*3+9*vertex+3] = 1.f;
      _unitVertex[nvface*3+9*vertex+4] = 0.f;
      _unitVertex[nvface*3+9*vertex+5] = 0.f;
      // Color
      _unitVertex[nvface*3+9*vertex+6] = 0.f;
      _unitVertex[nvface*3+9*vertex+7] = 0.f;
      _unitVertex[nvface*3+9*vertex+8] = 0.f;
      ++vertex;
    }
  }

  // Y faces
  for ( unsigned vertex = 0, ix = 0 ; ix < div+1 ; ++ix ) {
    for ( unsigned iz = 0 ; iz < div+1 ; ++iz ) {
      // Vertex
      _unitVertex[nvface*4+9*vertex  ] = -0.5f+(float)ix*dl;
      _unitVertex[nvface*4+9*vertex+1] = -0.5f;
      _unitVertex[nvface*4+9*vertex+2] = -0.5f+(float)iz*dl;
      // Normal
      _unitVertex[nvface*4+9*vertex+3] = 0.f;
      _unitVertex[nvface*4+9*vertex+4] = -1.f;
      _unitVertex[nvface*4+9*vertex+5] = 0.f;
      // Color
      _unitVertex[nvface*4+9*vertex+6] = 0.f;
      _unitVertex[nvface*4+9*vertex+7] = 0.f;
      _unitVertex[nvface*4+9*vertex+8] = 0.f;

      // Vertex
      _unitVertex[nvface*5+9*vertex  ] = -0.5f+(float)ix*dl;
      _unitVertex[nvface*5+9*vertex+1] = 0.5f;
      _unitVertex[nvface*5+9*vertex+2] = -0.5f+(float)iz*dl;
      // Normal
      _unitVertex[nvface*5+9*vertex+3] = 0.f;
      _unitVertex[nvface*5+9*vertex+4] = 1.f;
      _unitVertex[nvface*5+9*vertex+5] = 0.f;
      // Color
      _unitVertex[nvface*5+9*vertex+6] = 0.f;
      _unitVertex[nvface*5+9*vertex+7] = 0.f;
      _unitVertex[nvface*5+9*vertex+8] = 0.f;
      ++vertex;
    }
  }

  // Construct the index
  unsigned ii = 0;
  // -Z
  for ( unsigned line = 0 ; line < div ; ++line ) {
    for ( unsigned v = 0 ; v < div+1 ; ++v ) {
      _unitIndex[ii++] = line*(div+1)+v;
      _unitIndex[ii++] = (line+1)*(div+1)+v;
    }
    _unitIndex[ii++] = (line+1)*(div+1)+div;
    _unitIndex[ii++] = (line+1)*(div+1);
  }
  // -Y
  _unitIndex[ii-1] = nvface*4;
  for ( unsigned line = 0 ; line < div ; ++line ) {
    for ( unsigned v = 0 ; v < div+1 ; ++v ) {
      _unitIndex[ii++] = nvface*4+line*(div+1)+v;
      _unitIndex[ii++] = nvface*4+(line+1)*(div+1)+v;
    }
    _unitIndex[ii++] = nvface*4+(line+1)*(div+1)+div;
    _unitIndex[ii++] = nvface*4+(line+1)*(div+1);
  }
  // -X
  _unitIndex[ii-1] = nvface*2;
  for ( unsigned line = 0 ; line < div ; ++line ) {
    for ( unsigned v = 0 ; v < div+1 ; ++v ) {
      _unitIndex[ii++] = nvface*2+line*(div+1)+v;
      _unitIndex[ii++] = nvface*2+(line+1)*(div+1)+v;
    }
    _unitIndex[ii++] = nvface*2+(line+1)*(div+1)+div;
    _unitIndex[ii++] = nvface*2+(line+1)*(div+1);
  }
  // +Y
  _unitIndex[ii-1] = nvface*5;
  for ( unsigned line = 0 ; line < div ; ++line ) {
    for ( unsigned v = 0 ; v < div+1 ; ++v ) {
      _unitIndex[ii++] = nvface*5+line*(div+1)+v;
      _unitIndex[ii++] = nvface*5+(line+1)*(div+1)+v;
    }
    _unitIndex[ii++] = nvface*5+(line+1)*(div+1)+div;
    _unitIndex[ii++] = nvface*5+(line+1)*(div+1);
  }
  // +Z
  _unitIndex[ii-1] = nvface*1;
  for ( unsigned line = 0 ; line < div ; ++line ) {
    for ( unsigned v = 0 ; v < div+1 ; ++v ) {
      _unitIndex[ii++] = nvface*1+line*(div+1)+v;
      _unitIndex[ii++] = nvface*1+(line+1)*(div+1)+v;
    }
    _unitIndex[ii++] = nvface*1+(line+1)*(div+1)+div;
    _unitIndex[ii++] = nvface*1+(line+1)*(div+1);
  }
  // +X
  _unitIndex[ii-1] = nvface*3;
  for ( unsigned line = 0 ; line < div ; ++line ) {
    for ( unsigned v = 0 ; v < div+1 ; ++v ) {
      _unitIndex[ii++] = nvface*3+line*(div+1)+v;
      _unitIndex[ii++] = nvface*3+(line+1)*(div+1)+v;
    }
    _unitIndex[ii++] = nvface*3+(line+1)*(div+1)+div;
    _unitIndex[ii++] = nvface*3+(line+1)*(div+1);
  }

#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  if ( _mode == VBO ) {
    glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(_float)*6*_nvface,_unitVertex,GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(_uint)*6*_niface,_unitIndex,GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,0); // Release VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    delete[] _unitIndex;
    _unitIndex = nullptr;
  }
#endif
}

//
TriCube::~TriCube() {
}

//
void TriCube::draw(_float *c1, _float *c2, _float x, _float y, _float z) {
  if ( !_opengl ) return;
#ifdef HAVE_GL
  _float color[3];
  // X
  color[0] = c1[0]*(1.f-x)*0.5+c2[0]*(1.f+x)*0.5;
  color[1] = c1[1]*(1.f-x)*0.5+c2[1]*(1.f+x)*0.5;
  color[2] = c1[2]*(1.f-x)*0.5+c2[2]*(1.f+x)*0.5;
  glColor3fv(&color[0]);
  glBegin(GL_TRIANGLE_STRIP);
  glNormal3f(-1.f,0.f,0.f);
  glVertex3f(-0.5f,0.5f,0.5f);
  glVertex3f(-0.5f,0.5f,-0.5f);
  glVertex3f(-0.5f,-0.5f,0.5f);
  glVertex3f(-0.5f,-0.5f,-0.5f);
  glVertex3f(-0.5f,-0.5f,-0.5f);
  glNormal3f(1.f,0.f,0.f);
  glVertex3f(0.5f,-0.5f,-0.5f);
  glVertex3f(0.5f,-0.5f,-0.5f);
  glVertex3f(0.5f,-0.5f,0.5f);
  glVertex3f(0.5f,0.5f,-0.5f);
  glVertex3f(0.5f,0.5f,0.5f);
  glEnd();
  // Y
  color[0] = c1[0]*(1.f-y)*0.5+c2[0]*(1.f+y)*0.5;
  color[1] = c1[1]*(1.f-y)*0.5+c2[1]*(1.f+y)*0.5;
  color[2] = c1[2]*(1.f-y)*0.5+c2[2]*(1.f+y)*0.5;
  glColor3fv(&color[0]);
  glBegin(GL_TRIANGLE_STRIP);
  glNormal3f(0.f,-1.f,0.f);
  glVertex3f(0.5f,-0.5f,0.5f);
  glVertex3f(0.5f,-0.5f,-0.5f);
  glVertex3f(-0.5f,-0.5f,0.5f);
  glVertex3f(-0.5f,-0.5f,-0.5f);
  glVertex3f(-0.5f,-0.5f,-0.5f);
  glNormal3f(0.f,1.f,0.f);
  glVertex3f(-0.5f,0.5f,-0.5f);
  glVertex3f(-0.5f,0.5f,-0.5f);
  glVertex3f(-0.5f,0.5f,0.5f);
  glVertex3f(0.5f,0.5f,-0.5f);
  glVertex3f(0.5f,0.5f,0.5f);
  glEnd();
  // Z
  color[0] = c1[0]*(1.f-z)*0.5+c2[0]*(1.f+z)*0.5;
  color[1] = c1[1]*(1.f-z)*0.5+c2[1]*(1.f+z)*0.5;
  color[2] = c1[2]*(1.f-z)*0.5+c2[2]*(1.f+z)*0.5;
  glColor3fv(&color[0]);
  glBegin(GL_TRIANGLE_STRIP);
  glNormal3f(0.f,0.f,-1.f);
  glVertex3f(0.5f,0.5f,-0.5f);
  glVertex3f(-0.5f,0.5f,-0.5f);
  glVertex3f(0.5f,-0.5f,-0.5f);
  glVertex3f(-0.5f,-0.5f,-0.5f);
  glVertex3f(-0.5f,-0.5f,-0.5f);
  glNormal3f(0.f,0.f,1.f);
  glVertex3f(-0.5f,-0.5f,0.5f);
  glVertex3f(-0.5f,-0.5f,0.5f);
  glVertex3f(0.5f,-0.5f,0.5f);
  glVertex3f(-0.5f,0.5f,0.5f);
  glVertex3f(0.5f,0.5f,0.5f);
  glEnd();
#else
  (void) c1;
  (void) c2;
  (void) x;
  (void) y;
  (void) z;
#endif

}

//
void TriCube::pop() {
  ;
}

//
void TriCube::push() {
  ;
}

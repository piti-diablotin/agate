/**
 * @file src/./trimap.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2018 Jordan Bieder
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


#include "graphism/trimap.hpp"
#include "base/exception.hpp"
#include <cmath>
#include <algorithm>

//
TriMap::TriMap(bool opengl) : TriObj(opengl),
  _unitColor(nullptr),
  _vboColor(-1),
  _currentUpoint(-1),
  _currentVpoint(-1),
  _refresh(true)
{
#if defined(HAVE_GL) 
  glEnable(GL_POINT_SMOOTH);
#if defined(HAVE_GLEXT)
  glGenBuffers(1,&_vboColor);
#endif
#endif
}

//
TriMap::~TriMap() {
  if ( _unitVertex != nullptr ) delete[] _unitVertex;
  _unitVertex = nullptr;
  if ( _unitColor != nullptr ) delete[] _unitColor;
  _unitColor = nullptr;
  if ( _unitIndex != nullptr ) delete[] _unitIndex;
  _unitIndex = nullptr;
}

void TriMap::genUnit(double *origin, double *udir, double *vdir, int upoint, int vpoint) {
  if ( !_opengl ) return;
  if ( upoint < 2 ) 
    throw EXCEPTION("upoint < 2", ERRABT);
  if ( vpoint < 2 ) 
    throw EXCEPTION("vpoint < 2", ERRABT);

  _nvertex = (upoint)*(vpoint);
  
  if ( _unitVertex != nullptr ) delete[] _unitVertex;
  _unitVertex = new _float[3*_nvertex];
  if ( _unitColor != nullptr ) delete[] _unitColor;
  _unitColor = new _float[3*_nvertex];

  _float uvec[3];
  _float vvec[3];
  for ( int i = 0 ; i < 3 ; ++i ) {
    uvec[i] = udir[i]/(upoint-1);
    vvec[i] = vdir[i]/(vpoint-1);
  }
  _nindices = 4*((upoint-1)*(vpoint-1)); // We draw squares so 4 vertex for 1 quad)
  if ( _unitIndex != nullptr ) delete[] _unitIndex;
  _unitIndex = new _uint[_nindices];

  for ( int i = 0 ; i < (upoint) ; ++i ) {
    for ( int j = 0 ; j < (vpoint) ; ++j ) {
      int ivertex = 3*(i * (vpoint) + j);
      _unitVertex[ivertex+0] = origin[0]+uvec[0]*i+vvec[0]*j;
      _unitVertex[ivertex+1] = origin[1]+uvec[1]*i+vvec[1]*j;
      _unitVertex[ivertex+2] = origin[2]+uvec[2]*i+vvec[2]*j;
    }
  }
  _refresh = true;

  // Generate quads
  if ( upoint != _currentUpoint || vpoint != _currentVpoint ) {
    int quad = 0;
    for ( int i = 0 ; i < (upoint-1) ; ++i ) {
      for ( int j = 0 ; j < (vpoint-1) ; ++j ) {
        int ivertex = (i * (vpoint) + j);
        _unitIndex[quad+0] = ivertex;
        _unitIndex[quad+1] = ivertex+1;
        ivertex = ((i+1) * (vpoint) + j);
        _unitIndex[quad+2] = ivertex+1;
        _unitIndex[quad+3] = ivertex;
        quad+=4;
      }
    }
    _currentUpoint = upoint;
    _currentVpoint = vpoint;
#if defined(HAVE_GL) && defined(HAVE_GLEXT)
    if ( _mode == VBO ) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(_uint)*_nindices,_unitIndex,GL_STREAM_DRAW);
    }
#endif
  }
}


void TriMap::draw(std::vector<double> &values, float zero[3], float plus[3], float minus[3],bool refresh) {
  if ( !_opengl ) return;

#ifdef HAVE_GL
  if ( refresh || _refresh ) {
    if ( values.size() != _nvertex )
      throw EXCEPTION("Array size is not the same as the map grid",ERRABT);
    //double inv_max = 1./(*std::max_element(values.begin(),values.end()));
    //double inv_min = 1./(*std::min_element(values.begin(),values.end()));
    //inv_max = std::min(std::abs(inv_max),std::abs(inv_min));
    for ( unsigned ivertex = 0 ; ivertex < _nvertex ; ++ivertex ) {
      double value = values[ivertex];//*inv_max;
      /*
      if ( value >= 0 ) {
        _unitColor[3*ivertex+0] = zero[0] + (plus[0]-zero[0])*value;
        _unitColor[3*ivertex+1] = zero[1] + (plus[1]-zero[1])*value;
        _unitColor[3*ivertex+2] = zero[2] + (plus[2]-zero[2])*value;
      }
      else {
        _unitColor[3*ivertex+0] = zero[0] - (minus[0]-zero[0])*value;
        _unitColor[3*ivertex+1] = zero[1] - (minus[1]-zero[0])*value; 
        _unitColor[3*ivertex+2] = zero[2] - (minus[2]-zero[0])*value;
      }
      */
      TriObj::rgb(value,_unitColor[3*ivertex+0],_unitColor[3*ivertex+1],_unitColor[3*ivertex+2]);
      //TriObj::rgb((double)ivertex/(double)_nvertex,_unitColor[3*ivertex+0],_unitColor[3*ivertex+1],_unitColor[3*ivertex+2]);
    }
  }

  glDisable(GL_LIGHTING);
  switch(_mode) {
    case VERTEX :
      glBegin(GL_QUADS);
      for ( unsigned i = 0 ; i < (_nindices) ; ++i ) {
        glColor3fv(&_unitColor[3*_unitIndex[i]]);
        glVertex3fv(&_unitVertex[3*_unitIndex[i]]);
      }
      glEnd();
      break;
    case ARRAY :
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, _unitVertex);
      glColorPointer(3,GL_FLOAT, 0, _unitColor);
      glDrawElements(GL_QUADS, _nindices, GL_UNSIGNED_INT, _unitIndex);
      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      break;
#ifdef HAVE_GLEXT
    case VBO :
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);

      glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
      if ( _refresh )
        glBufferData(GL_ARRAY_BUFFER,sizeof(_float)*3*_nvertex,_unitVertex,GL_STREAM_DRAW);
      glVertexPointer(3, GL_FLOAT, 0, 0);

      glBindBuffer(GL_ARRAY_BUFFER,_vboColor);
      if ( refresh || _refresh)
        glBufferData(GL_ARRAY_BUFFER,sizeof(_float)*3*_nvertex,_unitColor,GL_STREAM_DRAW);
      glColorPointer(3,GL_FLOAT, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
      glIndexPointer(GL_UNSIGNED_INT, 0, 0);

      glDrawElements(GL_QUADS, _nindices, GL_UNSIGNED_INT, 0);

      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);

      glBindBuffer(GL_ARRAY_BUFFER,0); // Release VBO
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

      break;
#endif
  }
  glEnable(GL_LIGHTING);
  _refresh = false;
#else
  (void) values;
  (void) plus;
  (void) minus;
  (void) refresh;
#endif
}

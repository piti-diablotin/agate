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

//
TriMap::TriMap(bool opengl) : TriObj(opengl)
{
  ;
}

//
TriMap::~TriMap() {
  ;
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

  for ( int i = 0 ; i < (upoint) ; ++i ) {
    for ( int j = 0 ; j < (vpoint) ; ++j ) {
      int ivertex = 3*(i * (vpoint) + j);
      _unitVertex[ivertex+0] = origin[0]+uvec[0]*i+vvec[0]*j;
      _unitColor[ivertex+0] = 0.5*(1+std::cos(i*2*3.14/(upoint-1)));
      _unitColor[ivertex+2] = 0.5*(1+std::sin(j*2*3.14/(vpoint-1)));
    }
  }
  
}

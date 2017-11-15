/**
 * @file src/octahedra.cpp
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


#include "shape/octahedra.hpp"
#include "base/geometry.hpp"
#include <vector>
#include <utility>
#include <algorithm>
#include <cmath>
#include "base/exception.hpp"

#ifdef HAVE_GL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
#  ifdef HAVE_GLEXT
#    include <OpenGL/glext.h>
#  endif
# else
#  include <GL/gl.h>
#  ifdef HAVE_GLEXT
#    include <GL/glext.h>
#  endif
# endif
#endif

#ifdef HAVE_OMP
#include<omp.h>
#endif

//
Octahedra::Octahedra(int atom, int natom, const double *xred, const double *xcart, const double *rprim, bool opengl) : TriObj(opengl),
  _build(false),
  _vboLine(-1),
  _lines(),
  _drawAtoms(),
  _center(atom),
  _positions(),
  _shifts(),
  _basis()
{
  using namespace geometry;
  if ( det(rprim) < 1.e-6 )
    throw EXCEPTION("Octahedra cannot be built without translations",ERRDIV);

  vec3d center = {{ xcart[atom*3], xcart[atom*3+1], xcart[atom*3+2] }};
  std::vector< std::pair<int, float> > distance;
  if ( natom < 4 ) return;

  typedef struct {
    unsigned id;
    std::array<float,3>  shift;
    vec3d  xcart;
    float    distance;
  } atomInfo;
  std::vector<atomInfo> allAtoms;

  const float mx =6.f/static_cast<float>(std::sqrt(rprim[0]*rprim[0]+rprim[3]*rprim[3]+rprim[6]*rprim[6]));
  const float my =6.f/static_cast<float>(std::sqrt(rprim[1]*rprim[1]+rprim[4]*rprim[4]+rprim[7]*rprim[7]));
  const float mz =6.f/static_cast<float>(std::sqrt(rprim[2]*rprim[2]+rprim[5]*rprim[5]+rprim[8]*rprim[8]));
  const float nx = (0.5f < mx ? 0.5f : mx);
  const float ny = (0.5f < my ? 0.5f : my);
  const float nz = (0.5f < mz ? 0.5f : mz);
#ifdef HAVE_OMP
  using std::max;
  int nthreads = omp_get_num_threads();
  int chunk = max(1,natom/nthreads);
  if ( chunk > 50 ) chunk /= (chunk/50);
#endif

  // Build  the full set of atoms between -0.5 and 1.5 in reduce coordinates
#pragma omp parallel for shared(allAtoms,xred,xcart,rprim), schedule(dynamic,chunk)
  for ( int iatom = 0 ; iatom < natom ; ++iatom ) {
    atomInfo info;
    info.id = iatom;
    for ( int plusX = -1 ; plusX < 2 ; ++plusX ) {
      if ( xred[iatom*3] + plusX >= -nx || xred[iatom*3] + plusX <= 1.f+nx ) {
        for ( int plusY = -1 ; plusY < 2 ; ++plusY ) {
          if ( xred[iatom*3+1] + plusY >= -ny || xred[iatom*3+1] + plusY <= 1.f+ny ) {
            for ( int plusZ = -1 ; plusZ < 2 ; ++plusZ ) {
              if ( xred[iatom*3+2] + plusZ >= -nz || xred[iatom*3+2] + plusZ <= 1.f+nz ) {
                info.xcart[0] = xcart[iatom*3  ] + plusX*rprim[0] + plusY*rprim[1] + plusZ*rprim[2];
                info.xcart[1] = xcart[iatom*3+1] + plusX*rprim[3] + plusY*rprim[4] + plusZ*rprim[5];
                info.xcart[2] = xcart[iatom*3+2] + plusX*rprim[6] + plusY*rprim[7] + plusZ*rprim[8];
                info.distance = (float) norm(info.xcart-center);
                if (info.distance < 6.) {
                  info.shift[0] = (float)plusX;
                  info.shift[1] = (float)plusY;
                  info.shift[2] = (float)plusZ;
#pragma omp critical 
                  {
                    allAtoms.push_back(info);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Sort to get the closest
  std::sort(allAtoms.begin(), allAtoms.end(),
      [](atomInfo a1, atomInfo a2){
      return a1.distance < a2.distance;
      }
      );

  // Remove the center
  allAtoms.erase(allAtoms.begin());

  // Only keep the first 6 unitVertex;
  allAtoms.resize(6);

  // if the last distance is to large then we don't have an octahedra
  _build = ((allAtoms[5].distance-allAtoms[0].distance)/allAtoms[1].distance > 0.50 ? false : true);


  // Sort by Z
  std::sort(allAtoms.begin(),allAtoms.end(),
      [](atomInfo a1,atomInfo a2){
      return a1.xcart[2] < a2.xcart[2];
      }
      );

  {
    // Find _top1 (always 0) and _top2
    vec3d vec0 = allAtoms[0].xcart;
    vec3d vec1 = allAtoms[1].xcart-vec0;
    vec3d vec2 = allAtoms[2].xcart-vec0;
    vec3d vec3 = allAtoms[3].xcart-vec0;
    vec3d vec4 = allAtoms[4].xcart-vec0;
    vec3d vec5 = allAtoms[5].xcart-vec0;
    std::vector< std::pair<int, float> > length;
    length.push_back(std::make_pair(1,norm(vec1)));
    length.push_back(std::make_pair(2,norm(vec2)));
    length.push_back(std::make_pair(3,norm(vec3)));
    length.push_back(std::make_pair(4,norm(vec4)));
    length.push_back(std::make_pair(5,norm(vec5)));
    std::sort(length.begin(),length.end(),
        [](std::pair<int, float> p1,std::pair<int, float> p2){
        return p1.second < p2.second;
        }
        );
    _positions[_top1] = allAtoms[0].id;
    _shifts[_top1*3  ] = allAtoms[0].shift[0];
    _shifts[_top1*3+1] = allAtoms[0].shift[1];
    _shifts[_top1*3+2] = allAtoms[0].shift[2];
    _positions[_top2] = allAtoms[length[4].first].id;
    _shifts[_top2*3  ] = allAtoms[length[4].first].shift[0];
    _shifts[_top2*3+1] = allAtoms[length[4].first].shift[1];
    _shifts[_top2*3+2] = allAtoms[length[4].first].shift[2];
    allAtoms.erase(allAtoms.begin()+length[4].first);
    allAtoms.erase(allAtoms.begin());

    // Sort by X
    std::sort(allAtoms.begin(),allAtoms.end(),
        [](atomInfo a1,atomInfo a2){
        return a1.xcart[0] < a2.xcart[0];
        }
        );
    if ( allAtoms[1].xcart[1] < allAtoms[0].xcart[1] ) {
      auto tmp = allAtoms[1];
      allAtoms[1] = allAtoms[0];
      allAtoms[0] = tmp;
    }
    vec0 = allAtoms[0].xcart;
    vec1 = allAtoms[1].xcart-vec0;
    vec2 = allAtoms[2].xcart-vec0;
    vec3 = allAtoms[3].xcart-vec0;
    length.resize(3);
    length[0]=(std::make_pair(1,norm(vec1)));
    length[1]=(std::make_pair(2,norm(vec2)));
    length[2]=(std::make_pair(3,norm(vec3)));
    std::sort(length.begin(),length.end(),
        [](std::pair<int, float> p1,std::pair<int, float> p2){
        return p1.second < p2.second;
        }
        );

    _positions[_atom1 ] = allAtoms[0].id;
    _shifts[_atom1*3  ] = allAtoms[0].shift[0];
    _shifts[_atom1*3+1] = allAtoms[0].shift[1];
    _shifts[_atom1*3+2] = allAtoms[0].shift[2];

    _positions[_atom3 ] = allAtoms[length[2].first].id;
    _shifts[_atom3*3  ] = allAtoms[length[2].first].shift[0];
    _shifts[_atom3*3+1] = allAtoms[length[2].first].shift[1];
    _shifts[_atom3*3+2] = allAtoms[length[2].first].shift[2];

    allAtoms.erase(allAtoms.begin()+length[2].first);
    allAtoms.erase(allAtoms.begin());

  // Sort by Y
    std::sort(allAtoms.begin(),allAtoms.end(),
        [](atomInfo a1,atomInfo a2){
        return a1.xcart[1] < a2.xcart[1];
        }
        );

    _positions[_atom2 ] = allAtoms[0].id;
    _shifts[_atom2*3  ] = allAtoms[0].shift[0];
    _shifts[_atom2*3+1] = allAtoms[0].shift[1];
    _shifts[_atom2*3+2] = allAtoms[0].shift[2];

    _positions[_atom4 ] = allAtoms[1].id;
    _shifts[_atom4*3  ] = allAtoms[1].shift[0];
    _shifts[_atom4*3+1] = allAtoms[1].shift[1];
    _shifts[_atom4*3+2] = allAtoms[1].shift[2];
  }

  std::array<vec3d,6> _xcart;

  for ( _uint elt = 0 ; elt < 6 ; ++elt ) {
    _xcart[elt] = {{
      xcart[_positions[elt]*3  ] + _shifts[elt*3  ]*rprim[0] + _shifts[elt*3+1]*rprim[1] + _shifts[elt*3+2]*rprim[2],
      xcart[_positions[elt]*3+1] + _shifts[elt*3  ]*rprim[3] + _shifts[elt*3+1]*rprim[4] + _shifts[elt*3+2]*rprim[5],
      xcart[_positions[elt]*3+2] + _shifts[elt*3  ]*rprim[6] + _shifts[elt*3+1]*rprim[7] + _shifts[elt*3+2]*rprim[8]
    }};
  }

  _basis[0] = _xcart[_atom3]-_xcart[_atom1];
  _basis[1] = _xcart[_atom4]-_xcart[_atom2];
  _basis[2] = _xcart[_top2]-_xcart[_top1];
  /*
  _basis[0] = _basis[0]*(1./norm(_basis[0]));
  _basis[1] = _basis[1]*(1./norm(_basis[1]));
  _basis[2] = _basis[2]*(1./norm(_basis[2]));
  */

  for ( unsigned a = 0 ; a < 6 ; ++a ) 
    if ( _shifts[a*3] != 0 || _shifts[a*3+1] != 0 || _shifts[a*3+2] != 0 ) 
      _drawAtoms.push_back(a);

  _unitIndex = new _uint[24];
  _unitVertex = new _float[72];
  for ( _uint i = 0 ; i < 8*3 ; ++ i )
    _unitIndex[i] = i;

  _lines[ 0] = 1;
  _lines[ 1] = 2;
  _lines[ 2] = 5;
  _lines[ 3] = 8;
  _lines[ 4] = 1;
  _lines[ 5] = 0;
  _lines[ 6] = 4;
  _lines[ 7] = 5;
  _lines[ 8] = 0;
  _lines[ 9] = 8;
  _lines[10] = 12;
  _lines[11] = 1;
  _lines[12] = 2;
  _lines[13] = 12;
  _lines[14] = 5;
#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  if ( _build && _mode == VBO && _opengl) {
    glGenBuffers(1,&_vboLine);
    glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(_float)*8*3*3,nullptr,GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(_uint)*8*3,(void*)&_unitIndex[0],GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vboLine);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(_uint)*15,(void*)&_lines[0],GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,0); // Release VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
  }

#endif
}

Octahedra::Octahedra(const Octahedra& octa) : TriObj(octa),
  _build(octa._build),
  _vboLine(octa._vboLine),
  _lines(),
  _drawAtoms(octa._drawAtoms),
  _center(octa._center),
  _positions(octa._positions),
  _shifts(octa._shifts),
  _basis(octa._basis)
{
  _lines[ 0] = 1;
  _lines[ 1] = 2;
  _lines[ 2] = 5;
  _lines[ 3] = 8;
  _lines[ 4] = 1;
  _lines[ 5] = 0;
  _lines[ 6] = 4;
  _lines[ 7] = 5;
  _lines[ 8] = 0;
  _lines[ 9] = 8;
  _lines[10] = 12;
  _lines[11] = 1;
  _lines[12] = 2;
  _lines[13] = 12;
  _lines[14] = 5;
  _unitIndex = new _uint[24];
  _unitVertex = new _float[72];
  for ( _uint i = 0 ; i < 8*3 ; ++ i )
    _unitIndex[i] = i;
#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  if ( _build && _mode == VBO && _opengl) {
    glGenBuffers(1,&_vboLine);
    glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(_float)*8*3*3,nullptr,GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(_uint)*8*3,(void*)&_unitIndex[0],GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vboLine);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(_uint)*15,(void*)&_lines[0],GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER,0); // Release VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
  }

#endif
}

Octahedra::Octahedra(Octahedra&& octa) : TriObj(std::move(octa)),
  _build(octa._build),
  _vboLine(octa._vboLine),
  _lines(),
  _drawAtoms(octa._drawAtoms),
  _center(octa._center),
  _positions(octa._positions),
  _shifts(octa._shifts),
  _basis(octa._basis)
{
  _lines[ 0] = 1;
  _lines[ 1] = 2;
  _lines[ 2] = 5;
  _lines[ 3] = 8;
  _lines[ 4] = 1;
  _lines[ 5] = 0;
  _lines[ 6] = 4;
  _lines[ 7] = 5;
  _lines[ 8] = 0;
  _lines[ 9] = 8;
  _lines[10] = 12;
  _lines[11] = 1;
  _lines[12] = 2;
  _lines[13] = 12;
  _lines[14] = 5;
  octa._vboLine = (_uint) -1;
}

//
Octahedra::~Octahedra() {
#if defined(HAVE_GL) && defined(HAVE_GLEXT)
  if ( _build && _mode == VBO && _vboLine != (_uint) -1 && _opengl) 
    glDeleteBuffers(1,&_vboLine);
#endif
}

//
void Octahedra::build(const double *rprim, const double *xcart, u3f &new_atoms) {
  if ( xcart != nullptr ) {
    const _uint order[24] = {
      _top1, _atom1, _atom2,
      _top1, _atom2, _atom3,
      _top1, _atom3, _atom4,
      _top1, _atom4, _atom1,
      _top2, _atom1, _atom2,
      _top2, _atom2, _atom3,
      _top2, _atom3, _atom4,
      _top2, _atom4, _atom1
    };
    for ( _uint elt = 0 ; elt < 24 ; ++elt ) {
      _unitVertex[elt*3  ] = static_cast<_float>(xcart[_positions[order[elt]]*3  ] + _shifts[order[elt]*3  ]*rprim[0] + _shifts[order[elt]*3+1]*rprim[1] + _shifts[order[elt]*3+2]*rprim[2]);
      _unitVertex[elt*3+1] = static_cast<_float>(xcart[_positions[order[elt]]*3+1] + _shifts[order[elt]*3  ]*rprim[3] + _shifts[order[elt]*3+1]*rprim[4] + _shifts[order[elt]*3+2]*rprim[5]);
      _unitVertex[elt*3+2] = static_cast<_float>(xcart[_positions[order[elt]]*3+2] + _shifts[order[elt]*3  ]*rprim[6] + _shifts[order[elt]*3+1]*rprim[7] + _shifts[order[elt]*3+2]*rprim[8]);
    }
#if defined(HAVE_GL) && defined(HAVE_GLEXT)
    if ( _mode == VBO && _opengl) {
      glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);
      glBufferData(GL_ARRAY_BUFFER,sizeof(_float)*8*3*3,(void*)&_unitVertex[0],GL_STREAM_DRAW);
    }
#endif
  }
  const _uint rorder[6] = {
    0, 12, 1, 2, 5, 8
  };
  for ( auto a : _drawAtoms ) {
    std::array<float,3> pos = {{ static_cast<float>(_unitVertex[rorder[a]*3  ]),
      static_cast<float>(_unitVertex[rorder[a]*3+1]),
      static_cast<float>(_unitVertex[rorder[a]*3+2])}};
    new_atoms.push_back(std::make_pair(_positions[a],pos)); 
  }
}

//
void Octahedra::draw( float color[4] ) {
  if ( !(_build && _opengl) ) return;
#ifdef HAVE_GL

  glColor4fv(&color[0]);
  //( _drawing == SILHOUETTE ) ? glPolygonMode(GL_FRONT_AND_BACK,GL_LINE) :
  //glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  const GLfloat linecolor[3]={0.7f,0.7f,0.7f};
  switch ( _mode ) {
#ifdef HAVE_GLEXT
    case VBO :
      glBindBuffer(GL_ARRAY_BUFFER,_vbos[0]);

      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vbos[1]);
      glIndexPointer(GL_UNSIGNED_INT, 0, 0);

      glDrawElements(GL_TRIANGLES,24, GL_UNSIGNED_INT, 0);

      glColor3fv(&linecolor[0]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,_vboLine);
      glIndexPointer(GL_UNSIGNED_INT, 0, 0);

      glDrawElements(GL_LINE_STRIP,15, GL_UNSIGNED_INT, 0);

      glDisableClientState(GL_VERTEX_ARRAY);

      glBindBuffer(GL_ARRAY_BUFFER,0); // Release VBO
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
      break;
#endif
    case ARRAY :
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, &_unitVertex[0]);
      glDrawElements(GL_TRIANGLES,24, GL_UNSIGNED_INT, &_unitIndex[0]);

      glColor3fv(&linecolor[0]);
      glDrawElements(GL_LINE_STRIP,15, GL_UNSIGNED_INT, &_lines[0]);
      glDisableClientState(GL_VERTEX_ARRAY);
      break;
    case VERTEX :
      glBegin(GL_TRIANGLES);
      for( unsigned v = 0 ; v < 24 ; ++v ) {
        glVertex3fv(&_unitVertex[3*_unitIndex[v]]);
      }
      glEnd();
      glColor3fv(&linecolor[0]);
      glDrawElements(GL_LINE_STRIP,15, GL_UNSIGNED_INT, &_lines[0]);
      glBegin(GL_LINE_STRIP);
      for( unsigned v = 0 ; v < 15 ; ++v ) {
        glVertex3fv(&_unitVertex[3*_lines[v]]);
      }
      glEnd();
      break;
  }
#else
  (void) color;
#endif
}

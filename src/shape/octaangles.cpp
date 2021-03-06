/**
 * @file src/octaangles.cpp
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


#include "shape/octaangles.hpp"
#include "base/exception.hpp"
#include "base/geometry.hpp"
#include <cmath>

//
OctaAngles::OctaAngles(int iatom, int natom, const double *xred, const double *xcart, const double *rprim, bool opengl) :
  Octahedra(iatom, natom, xred, xcart, rprim, opengl),
  _angles({{0.,0.,0.}}),
  _savedBasis(_basis)
{
}

OctaAngles::OctaAngles(const OctaAngles& octa) : Octahedra(octa),
  _angles({{0.,0.,0.}}),
  _savedBasis(octa._savedBasis)
{
}

OctaAngles::OctaAngles(OctaAngles&& octa) : Octahedra(std::move(octa)),
  _angles({{0.,0.,0.}}),
  _savedBasis(octa._savedBasis)
{
}

OctaAngles::OctaAngles(const Octahedra& octa) : Octahedra(octa),
  _angles({{0.,0.,0.}}),
  _savedBasis(_basis)
{
}

OctaAngles::OctaAngles(Octahedra&& octa) : Octahedra(std::move(octa)),
  _angles({{0.,0.,0.}}),
  _savedBasis()
{
  _savedBasis = _basis;
}


//
OctaAngles::~OctaAngles() {
  ;
}


void OctaAngles::build(const double *rprim, const double *xcart, u3f &new_atoms ) {
  if ( xcart == nullptr ) throw EXCEPTION("You must provide xcart",ERRABT);
  using namespace geometry;
  double x1, x2, y1, y2, z1, z2 = 0.;
  double norm1, norm2 = 0.;
  const double rad2deg = 180. / std::acos(-1.0);
  vec3d vec1, vec2, basisNorm;
  std::array<vec3d,6> _xcart;

  u3f dummy;
  Octahedra::build(rprim,xcart,dummy);

  for ( _uint elt = 0 ; elt < 6 ; ++elt ) {
    _xcart[elt] = {{
      xcart[_positions[elt]*3  ] + _shifts[elt*3  ]*rprim[0] + _shifts[elt*3+1]*rprim[1] + _shifts[elt*3+2]*rprim[2],
      xcart[_positions[elt]*3+1] + _shifts[elt*3  ]*rprim[3] + _shifts[elt*3+1]*rprim[4] + _shifts[elt*3+2]*rprim[5],
      xcart[_positions[elt]*3+2] + _shifts[elt*3  ]*rprim[6] + _shifts[elt*3+1]*rprim[7] + _shifts[elt*3+2]*rprim[8]
    }};
  }

  basisNorm[0] = norm(_basis[0]);
  basisNorm[1] = norm(_basis[1]);
  basisNorm[2] = norm(_basis[2]);

  // Z-Axis
  using namespace geometry; 
  vec1 = _xcart[_atom3]-_xcart[_atom1];
  norm1 = norm(vec1);
  vec2 = _xcart[_atom4]-_xcart[_atom2];
  norm2 = norm(vec2);
  y1 = dot(vec1,_basis[1])/(norm1*basisNorm[1]);
  x2 = dot(vec2,_basis[0])/(norm2*basisNorm[0]);
  //std::cerr << "a1: " << std::asin(-y1)*rad2deg << " a2: " << std::asin(x2)*rad2deg << std::endl;
  _angles[2] = (std::asin(-y1)+std::asin(x2))*0.5f*rad2deg;

  // X-Axis 
  vec1 = _xcart[_atom4]-_xcart[_atom2];
  norm1 = norm(vec1);
  vec2 = _xcart[_top2]-_xcart[_top1];
  norm2 = norm(vec2);
  z1 = dot(vec1,_basis[2])/(norm1*basisNorm[2]);
  y2 = dot(vec2,_basis[1])/(norm2*basisNorm[1]);
  //std::cerr << "b1: " << std::asin(-z1)*rad2deg << " b2: " << std::asin(y2)*rad2deg << std::endl;
  _angles[0] = (std::asin(-z1)+std::asin(y2))*0.5f*rad2deg;

  // Y-Axis
  vec1 = _xcart[_top2]-_xcart[_top1];
  norm1 = norm(vec1);
  vec2 = _xcart[_atom3]-_xcart[_atom1];
  norm2 = norm(vec2);
  x1 = dot(vec1,_basis[0])/(norm1*basisNorm[0]);
  z2 = dot(vec2,_basis[2])/(norm2*basisNorm[2]);
  //std::cerr << "c1: " << std::asin(-x1)*rad2deg << " c2: " << std::asin(z2)*rad2deg << std::endl;
  _angles[1] = (std::asin(-x1)+std::asin(z2))*0.5f*rad2deg;

  auto tmp(_angles);
  //std::cerr << _angles[0] << " " << _angles[1] << " " << _angles[2] << std::endl;
  new_atoms.push_back(std::make_pair(_center,tmp));
}

void OctaAngles::buildCart(const double *rprim, const double *xcart, u3f &new_atoms, bool cartBasis ) {
  //std::array<geometry::vec3d,3> save(_basis);
  _basis[0] = {{ 1,0,0}};
  _basis[1] = {{ 0,1,0}};
  _basis[2] = {{ 0,0,1}};
  this->build(rprim,xcart,new_atoms);
  if ( !cartBasis )
    _basis = _savedBasis;
}

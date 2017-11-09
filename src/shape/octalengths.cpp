/**
 * @file src/octaangles.cpp
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


#include "shape/octalengths.hpp"
#include "base/exception.hpp"
#include "base/geometry.hpp"
#include <cmath>

//
OctaLengths::OctaLengths(int iatom, int natom, const double *xred, const double *xcart, const double *rprim, bool opengl) :
  Octahedra(iatom, natom, xred, xcart, rprim, opengl),
  _lengths({{0.,0.,0.}})
{
}

OctaLengths::OctaLengths(const OctaLengths& octa) : Octahedra(octa),
  _lengths({{0.,0.,0.}})
{
}

OctaLengths::OctaLengths(OctaLengths&& octa) : Octahedra(std::move(octa)),
  _lengths({{0.,0.,0.}})
{
}

OctaLengths::OctaLengths(const Octahedra& octa) : Octahedra(octa),
  _lengths({{0.,0.,0.}})
{
}

OctaLengths::OctaLengths(Octahedra&& octa) : Octahedra(std::move(octa)),
  _lengths({{0.,0.,0.}})
{
}

//
OctaLengths::~OctaLengths() {
  ;
}


void OctaLengths::build(const double *rprim, const double *xcart, u3f &new_atoms ) {
  if ( xcart == nullptr ) throw EXCEPTION("You must provide xcart",ERRABT);
  using namespace geometry;
  vec3d vec1, vec2, vec3; 
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

  vec1 = _xcart[_atom3]-_xcart[_atom1];
  vec2 = _xcart[_atom4]-_xcart[_atom2];
  vec3 = _xcart[_top2]-_xcart[_top1];

  /*
  _lengths[0] = (norm(vec1)-norm(_basis[0]))*0.5;
  _lengths[1] = (norm(vec2)-norm(_basis[1]))*0.5;
  _lengths[2] = (norm(vec3)-norm(_basis[2]))*0.5;
  */

  _lengths[0] = norm(vec1)*0.5;
  _lengths[1] = norm(vec2)*0.5;
  _lengths[2] = norm(vec3)*0.5;


  auto tmp(_lengths);
  //std::cerr << _angles[0] << " " << _angles[1] << " " << _angles[2] << std::endl;
  new_atoms.push_back(std::make_pair(_center,tmp));
}

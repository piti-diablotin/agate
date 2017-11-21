/**
 * @file geometry.cpp
 *
 * @brief Some basic functions for 3D geometry.
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


#include "base/geometry.hpp"
#include <iostream>
#include <utility>
#include <iomanip>
#include <locale>
#include <cctype>
#include <algorithm>
#include "base/utils.hpp"
#include "base/exception.hpp"
#include "base/phys.hpp"

namespace geometry {

  //
  void print(const mat3d& mat, std::ostream& out) {
    out.precision(2);
    out.setf(std::ios::right, std::ios::adjustfield);
    out.setf(std::ios::scientific, std::ios::floatfield);
    out << "[ " 
      << "[ "
      << std::setw(14) << mat[0] 
      << std::setw(14) << mat[1] 
      << std::setw(14) << mat[2] 
      << " ]" << std::endl
      << std::setw(4) << "[ "
      << std::setw(14) << mat[3] 
      << std::setw(14) << mat[4] 
      << std::setw(14) << mat[5] 
      << " ]" << std::endl
      << std::setw(4) << "[ "
      << std::setw(14) << mat[6] 
      << std::setw(14) << mat[7] 
      << std::setw(14) << mat[8] 
      << " ] ]" << std::endl;
  }

  //
  void print(const vec3d& vec, std::ostream& out) {
    out.precision(6);
    out.setf(std::ios::right, std::ios::adjustfield);
    out << "  [ " 
      << std::setw(14) << vec[0] 
      << std::setw(14) << vec[1] 
      << std::setw(14) << vec[2] 
      << " ]" << std::endl;
  }

  //
  mat3d invert(const mat3d& mat) {
    mat3d inverse;
    double inv_det = 1e0 / det(mat);
    inverse[0] = inv_det * ( mat[4]*mat[8]-mat[7]*mat[5] );
    inverse[1] = inv_det * ( mat[2]*mat[7]-mat[1]*mat[8] );
    inverse[2] = inv_det * ( mat[1]*mat[5]-mat[4]*mat[2] );
    inverse[3] = inv_det * ( mat[6]*mat[5]-mat[3]*mat[8] );
    inverse[4] = inv_det * ( mat[0]*mat[8]-mat[6]*mat[2] );
    inverse[5] = inv_det * ( mat[2]*mat[3]-mat[0]*mat[5] );
    inverse[6] = inv_det * ( mat[3]*mat[7]-mat[6]*mat[4] );
    inverse[7] = inv_det * ( mat[1]*mat[6]-mat[0]*mat[7] );
    inverse[8] = inv_det * ( mat[0]*mat[4]-mat[3]*mat[1] );
    return inverse;

  }

  
  //
  mat3d invert(const double mat[9]) {
    mat3d inverse;
    double inv_det = 1e0 / det(mat);
    inverse[0] = inv_det * ( mat[4]*mat[8]-mat[7]*mat[5] );
    inverse[1] = inv_det * ( mat[2]*mat[7]-mat[1]*mat[8] );
    inverse[2] = inv_det * ( mat[1]*mat[5]-mat[4]*mat[2] );
    inverse[3] = inv_det * ( mat[6]*mat[5]-mat[3]*mat[8] );
    inverse[4] = inv_det * ( mat[0]*mat[8]-mat[6]*mat[2] );
    inverse[5] = inv_det * ( mat[2]*mat[3]-mat[0]*mat[5] );
    inverse[6] = inv_det * ( mat[3]*mat[7]-mat[6]*mat[4] );
    inverse[7] = inv_det * ( mat[1]*mat[6]-mat[0]*mat[7] );
    inverse[8] = inv_det * ( mat[0]*mat[4]-mat[3]*mat[1] );
    return inverse;

  }
  //
  mat3d sc_mult(const mat3d& mat, double a) {
  mat3d sc_mult;
    sc_mult[0] = mat[0] * a; 
    sc_mult[1] = mat[1] * a;
    sc_mult[2] = mat[2] * a;
    sc_mult[3] = mat[3] * a;
    sc_mult[4] = mat[4] * a;
    sc_mult[5] = mat[5] * a;
    sc_mult[6] = mat[6] * a;
    sc_mult[7] = mat[7] * a;
    sc_mult[8] = mat[8] * a;
    sc_mult[9] = mat[9] * a;		
    return sc_mult;    
  } 
  //
  mat3d invertTranspose(const mat3d& mat) {
    mat3d inverse;
    double inv_det = 1e0 / det(mat);
    inverse[0] = inv_det * ( mat[4]*mat[8]-mat[7]*mat[5] );
    inverse[3] = inv_det * ( mat[2]*mat[7]-mat[1]*mat[8] );
    inverse[6] = inv_det * ( mat[1]*mat[5]-mat[4]*mat[2] );
    inverse[1] = inv_det * ( mat[6]*mat[5]-mat[3]*mat[8] );
    inverse[4] = inv_det * ( mat[0]*mat[8]-mat[6]*mat[2] );
    inverse[7] = inv_det * ( mat[2]*mat[3]-mat[0]*mat[5] );
    inverse[2] = inv_det * ( mat[3]*mat[7]-mat[6]*mat[4] );
    inverse[5] = inv_det * ( mat[1]*mat[6]-mat[0]*mat[7] );
    inverse[8] = inv_det * ( mat[0]*mat[4]-mat[3]*mat[1] );
    return inverse;

  }
  
  
  
  //
  mat3d invertTranspose(const double mat[9]) {
    mat3d inverse;
    double inv_det = 1e0 / det(mat);
    inverse[0] = inv_det * ( mat[4]*mat[8]-mat[7]*mat[5] );
    inverse[3] = inv_det * ( mat[2]*mat[7]-mat[1]*mat[8] );
    inverse[6] = inv_det * ( mat[1]*mat[5]-mat[4]*mat[2] );
    inverse[1] = inv_det * ( mat[6]*mat[5]-mat[3]*mat[8] );
    inverse[4] = inv_det * ( mat[0]*mat[8]-mat[6]*mat[2] );
    inverse[7] = inv_det * ( mat[2]*mat[3]-mat[0]*mat[5] );
    inverse[2] = inv_det * ( mat[3]*mat[7]-mat[6]*mat[4] );
    inverse[5] = inv_det * ( mat[1]*mat[6]-mat[0]*mat[7] );
    inverse[8] = inv_det * ( mat[0]*mat[4]-mat[3]*mat[1] );
    return inverse;

  }


  //
  std::vector<vec3d> changeBasis(const mat3d& basis, const std::vector<vec3d>& vecs) {
    std::vector<vec3d> newcoords(vecs.size());
    mat3d invBasis = invertTranspose(basis);
    for( unsigned ivec=0 ; ivec < vecs.size() ; ++ivec ) {
      newcoords[ivec] = {{
        invBasis[0]*vecs[ivec][0] + invBasis[1]*vecs[ivec][1] + invBasis[2]*vecs[ivec][2] ,
        invBasis[3]*vecs[ivec][0] + invBasis[4]*vecs[ivec][1] + invBasis[5]*vecs[ivec][2] ,
        invBasis[6]*vecs[ivec][0] + invBasis[7]*vecs[ivec][1] + invBasis[8]*vecs[ivec][2] ,
      }};
    }

    return newcoords;
  }


  //
  double angle(const vec3d& vec1, const vec3d& vec2) {
    const double n1 = norm(vec1); 
    const double n2 = norm(vec2); 
    static const double rad2deg = 180.0 / phys::pi;
    return std::acos(dot(vec1,vec2)/(n1*n2))*rad2deg;
  }


  //
  vec3d angle(const mat3d& rprim) {
    const vec3d x {{ rprim[0], rprim[3], rprim[6] }} ;
    const vec3d y {{ rprim[1], rprim[4], rprim[7] }} ;
    const vec3d z {{ rprim[2], rprim[5], rprim[8] }} ;
    const double nx = norm(x); 
    const double ny = norm(y); 
    const double nz = norm(z); 
    static const double rad2deg = 180.0 / phys::pi;
    vec3d angle {{
        std::acos(dot(y,z)/(ny*nz))*rad2deg,
        std::acos(dot(x,z)/(nx*nz))*rad2deg,
        std::acos(dot(x,y)/(nx*ny))*rad2deg
        }} ;
    return angle;
  }


  //
  void getSymmetry(const std::string& operations, mat3d& rotation, vec3d& translation) {
    // Clear rotation
    rotation.fill(0.0e0);
    // Clear translation
    translation.fill(0.0e0);

    std::vector<std::string> op = utils::explode(utils::trim(operations,"'"),',');
    if ( op.size() != 3 ) throw EXCEPTION("Invalid symmetry string",ERRDIV);

    unsigned vec = 0;
    std::locale loc;
    char c2 = ' ' ; // Declare c2 here for valgrind comprehension
    for ( auto& axe: op ) {

      utils::trim(axe);
      size_t pos = 0;
      double sign = 1.0;
      while ( pos < axe.size() ){
        const char c1 = std::tolower(axe[pos++],loc);
        if ( c1 == '-' ) {
          sign = -1.0;
          continue;
        }
        else if ( c1 == '+' ) {
          sign = 1.0;
          continue;
        }
        else if ( c1 == 'x' || c1 == 'y' || c1 == 'z' ) {
          rotation[vec*3+(c1-'x')] = sign;
          sign = 1.0;
          continue;
        }
        else {
          double numerator = 0.0;
          double denominator = 1.0;
          --pos; // go backward so that c2=c1;
          while ( pos < axe.size() && std::isdigit(c2=axe[pos++]) ) {
            numerator *= 10.0;
            numerator += utils::stod(std::string(1,c2));
          }
          if ( c2 == '/' ) {
            denominator = 0.0;
            while ( pos < axe.size() && std::isdigit(c2=axe[pos++]) ) {
              denominator *= 10.0;
              denominator += utils::stod(std::string(1,c2));
            }
          }
          try {
            translation[vec] = sign*numerator/denominator;
          }
          catch (...) {
            throw EXCEPTION("Divided by zero",ERRDIV);
          }

          sign = 1.0;
        }
      }
      ++vec;
    }
  }

  double getWignerSeitzRadius(const mat3d &rprimd) {
    const vec3d a = {{ rprimd [0], rprimd[3], rprimd[6] }};
    const vec3d b = {{ rprimd [1], rprimd[4], rprimd[7] }};
    const vec3d c = {{ rprimd [2], rprimd[5], rprimd[8] }};
    const vec3d ab = a+b;
    const vec3d ac = a+c;
    const vec3d bc = b+c;
    const vec3d abc = ab+c;
    std::vector<double> length(7);
    length[0] = 0.5*norm(a);
    length[1] = 0.5*norm(b);
    length[2] = 0.5*norm(c);
    length[3] = 0.5*norm(ab);
    length[4] = 0.5*norm(ac);
    length[5] = 0.5*norm(bc);
    length[6] = 0.5*norm(abc);
    return *std::min_element(length.begin(),length.end());
  }

  double getWignerSeitzRadius(const double rprimd[9]) {
    const vec3d a = {{ rprimd [0], rprimd[3], rprimd[6] }};
    const vec3d b = {{ rprimd [1], rprimd[4], rprimd[7] }};
    const vec3d c = {{ rprimd [2], rprimd[5], rprimd[8] }};
    const vec3d ab = a+b;
    const vec3d ac = a+c;
    const vec3d bc = b+c;
    const vec3d abc = ab+c;
    std::vector<double> length(7);
    length[0] = 0.5*norm(a);
    length[1] = 0.5*norm(b);
    length[2] = 0.5*norm(c);
    length[3] = 0.5*norm(ab);
    length[4] = 0.5*norm(ac);
    length[5] = 0.5*norm(bc);
    length[6] = 0.5*norm(abc);
    return *std::min_element(length.begin(),length.end());
  }
}


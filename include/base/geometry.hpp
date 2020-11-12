/**
 * @file geometry.hpp
 *
 * @brief Some basic functions for 3D geometry.
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

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <array>
#include <vector>
#include <iostream>
#include <cmath>


namespace geometry {

  /**
   * Simple definition of a 3D vector.
   */
  typedef std::array<double,3> vec3d;

  /**
   * Simple definition of a 3D Matrix.
   */
  typedef std::array<double,9> mat3d;


  /**
   * Simple function to acces vector,axe in a mat3d.
   * @param vec vector (1=a, 2=b or 3=c).
   * @param axe axe(1=x, 2=y, 3=z).
   * @return The indice in the mat3d array.
   */
  inline unsigned mat3dind(unsigned vec, unsigned axe) {
    return (vec-1)+(axe-1)*3;
  }


  /**
   * Print a mat3d
   * @param mat mat3d to print.
   * @param stream stream to print in.
   */
  void print(const mat3d& mat, std::ostream& out = std::cout);

  /**
   * Print a vec3d
   * @param vec vec3d to print.
   * @param stream stream to print in.
   */
  void print(const vec3d& vec, std::ostream& out = std::cout);

  /**
   * Return a qpt as string
   * @param vec vec3d to transform in string
   * @return qpt as nicely formated string
   */
  std::string to_string(const vec3d& vec, bool nice = true);


  /**
   * Calculate the determinant of the matrix.
   * @param mat The matrix.
   * @return The determinant
   */
  inline double det(const mat3d& mat) {
    return ( 
         (mat[2]*mat[3] -mat[0]*mat[5]) *mat[7]
        +(mat[0]*mat[4] -mat[1]*mat[3]) *mat[8]
        +(mat[1]*mat[5] -mat[2]*mat[4]) *mat[6] );
  }


  /**
   * Calculate the determinant of the matrix.
   * @param mat The matrix.
   * @return The determinant
   */
  inline double det(const double mat[]) {
    return (
         (mat[2]*mat[3] -mat[0]*mat[5]) *mat[7]
        +(mat[0]*mat[4] -mat[1]*mat[3]) *mat[8]
        +(mat[1]*mat[5] -mat[2]*mat[4]) *mat[6] );
  }


  /**
   * Invert a mat3d matrix.
   * @param mat The matrix to inverse.
   * @return the inverse of the matrix.
   */
  mat3d invert(const mat3d& mat);


  /**
   * Invert a mat3d matrix.
   * @param mat The matrix to inverse.
   * @return the inverse of the matrix.
   */
  mat3d invert(const double mat[9]);


  /**
   * Invert a mat3d matrix and transpose the inverse.
   * @param mat The matrix to inverse.
   * @return the inverse of the matrix.
   */
  mat3d invertTranspose(const mat3d& mat);


  /**
   * Invert a mat3d matrix and transpose the inverse.
   * @param mat The matrix to inverse.
   * @return the inverse of the matrix.
   */
  mat3d invertTranspose(const double mat[9]);

  /**
   * Transpose a matrix
   * @param mat The matrix to transpose
   * @return the transpose of the matrix.
   */
  mat3d transpose(const double mat[9]);


  /**
   * Transpose a matrix
   * @param mat The matrix to transpose
   * @return the transpose of the matrix.
   */
  mat3d transpose(const mat3d& mat);

  /**
   * Change some 3D vectors from basis.
   * @param rprim new basis
   * @param cart coordinates in cartesian coordinates
   * @oaram red coordinates in rprim basis
   */
  void changeBasis(const mat3d& rprim, std::vector<vec3d>& cart, std::vector<vec3d>& red, const bool C2R );


  /** 
   * Scale a vector by a double number.
   * @param vec The vec3d
   * @param scale Scaling factor
   * @return The scaled vec3d
   */
  inline vec3d operator*(const vec3d& vec, const double scale) {
    return vec3d({{vec[0]*scale, vec[1]*scale, vec[2]*scale}});
  }


  /** 
   * Add 2 vect3d
   * @param vec1 first operand
   * @param vec2 second operand
   * @return Sum of vec1+vec2
   */
  inline vec3d operator+(const vec3d& vec1, const vec3d& vec2) {
    return vec3d({{ vec1[0]+vec2[0], vec1[1]+vec2[1], vec1[2]+vec2[2]
        }});
  }


  /** 
   * Add 2 vect3d
   * @param vec1 first operand
   * @param vec2 second operand
   * @return Sum of vec1+vec2
   */
  inline vec3d& operator+=(vec3d& vec1, const vec3d& vec2) {
    vec1[0]+=vec2[0];
    vec1[1]+=vec2[1];
    vec1[2]+=vec2[2];
    return vec1;
  }


  /** 
   * Substract 2 vect3d
   * @param vec1 first operand
   * @param vec2 second operand
   * @return difference vec1-vec2
   */
  inline vec3d operator-(const vec3d& vec1, const vec3d& vec2) {
    return vec3d({{ vec1[0]-vec2[0], vec1[1]-vec2[1], vec1[2]-vec2[2]
        }});
  }

  /** 
   * Euclidean norm of the vector
   * @param vec vec3d to use
   * @return sqrt(x*x+y*y+z*z)
   */
  inline double norm(const vec3d& vec) {
    return std::sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
  }

  /** 
   * Check if 2 vectors are the same at 1e-10
   * @param vector to check against
   * @return true if the 2 vectors are the same
   */
  inline bool operator==(const vec3d& vec1, const vec3d& vec2) {
    const vec3d diff = vec1-vec2;
    return std::abs(diff[0])<1e-10 && std::abs(diff[1])<1e-10 && std::abs(diff[2])<1e-10;
  }

  /**
   * Check if 2 vectors are different at 1e-10
   * @param vector to check against
   * @return true if the 2 vectors are different
   */
  inline bool operator!=(const vec3d& vec1, const vec3d& vec2) {
    const vec3d diff = vec1-vec2;
    return std::abs(diff[0])>=1e-10 || std::abs(diff[1])>=1e-10 || std::abs(diff[2])>=1e-10;
  }

  /** 
   * Dot product of two vectors.
   * @param vec1 vec3d to use
   * @param vec2 vec3d to use
   * @return x*x+y*y+z*z
   */
  inline double dot(const vec3d& vec1, const vec3d vec2) {
    return vec1[0]*vec2[0]+vec1[1]*vec2[1]+vec1[2]*vec2[2];
  }


  /** 
   * Scale a mat3d by a double number.
   * @param mat The mat3d
   * @param scale Scaling factor
   * @return The scaled mat3d
   */
  inline mat3d operator*(const mat3d& mat, const double scale) {
    return mat3d({{ mat[0]*scale, mat[1]*scale, mat[2]*scale,
        mat[3]*scale, mat[4]*scale, mat[5]*scale,
        mat[6]*scale, mat[7]*scale, mat[8]*scale
        }});
  }


  /** 
   * Multiply a matrix by a vector
   * @param mat The mat3d
   * @param scale Scaling factor
   * @return The scaled mat3d
   */
  inline vec3d operator*(const mat3d& mat, const vec3d& vec) {
    return vec3d({{
        mat[0]*vec[0]+mat[1]*vec[1]+mat[2]*vec[2],
        mat[3]*vec[0]+mat[4]*vec[1]+mat[5]*vec[2],
        mat[6]*vec[0]+mat[7]*vec[1]+mat[8]*vec[2],
        }});
  }


  /** 
   * Multiply a matrix by a vector
   * @param mat The mat3d
   * @param scale Scaling factor
   * @return The scaled mat3d
   */
  inline vec3d operator*(const vec3d& vec, const mat3d& mat) {
    return vec3d({{
        mat[0]*vec[0]+mat[3]*vec[1]+mat[6]*vec[2],
        mat[1]*vec[0]+mat[4]*vec[1]+mat[7]*vec[2],
        mat[2]*vec[0]+mat[5]*vec[1]+mat[8]*vec[2],
        }});
  }

  /** 
   * Cross product of 2 vectors
   * @param vec1 first vector
   * @param vec2 second vector
   * @return vec3 the cross product v1 x v2
   */
  inline vec3d operator*(const vec3d& v1, const vec3d& v2) {
    return vec3d({{
        v1[1]*v2[2]-v1[2]*v2[1],
       -v1[0]*v2[2]+v1[2]*v2[0],
        v1[0]*v2[1]-v1[1]*v2[0],
        }});
  }

  /** 
  * Multiply a matrix by a scalar(double) 
  * @param mat The mat3d 
  * @param a the double scalar 
  */ 
   mat3d sc_mult(const mat3d& mat, double a);
  
   /** 
   * Multiply a matrix by a matrix
   * @param mat1 The mat3d
   * @param mat2 The mat3d
   * @return The produc of mat1*mat2
   */
  inline mat3d operator*(const mat3d& mat1, const mat3d& mat2) {
    return mat3d({{
        mat1[0]*mat2[0]+mat1[1]*mat2[3]+mat1[2]*mat2[6], mat1[0]*mat2[1]+mat1[1]*mat2[4]+mat1[2]*mat2[7], mat1[0]*mat2[2]+mat1[1]*mat2[5]+mat1[2]*mat2[8],
        mat1[3]*mat2[0]+mat1[4]*mat2[3]+mat1[5]*mat2[6], mat1[3]*mat2[1]+mat1[4]*mat2[4]+mat1[5]*mat2[7], mat1[3]*mat2[2]+mat1[4]*mat2[5]+mat1[5]*mat2[8],
        mat1[6]*mat2[0]+mat1[7]*mat2[3]+mat1[8]*mat2[6], mat1[6]*mat2[1]+mat1[7]*mat2[4]+mat1[8]*mat2[7], mat1[6]*mat2[2]+mat1[7]*mat2[5]+mat1[8]*mat2[8],
        }});
  }

   /** 
   * Recenter the coordinated so the "reduced coordinates" are between ]-0.5;0.5];
   * @param vec1 vector of reduced coordinated
   */
  inline void recenter(vec3d& vec) {
    for (auto &v : vec) 
    { 
      while ( v <= -0.5 ) ++v;
      while ( v > 0.5 ) --v;
    }
  }


  /**
   * Compute the angles between 2 vectors
   * @param vec1 vector 1.
   * @param vec2 vector 2.
   * @return The angle between the two vectors
   */
  double angle(const vec3d& vec1, const vec3d& vec2);


  /**
   * Compute the angles between the axis (column) 1 -> (2,3) 2->(1,3) 3->(1,2)
   * @param rprim The axis vectors
   * @return The angles between all three vectors
   */
  vec3d angle(const mat3d& rprim);


  /**
   * Get the rotation matrix and translation vector for a symmetry like -x,-y,z+1/2
   * +/-1/2 should only follow the +/(x/y/z)
   * @param operations The string representing the symmerty operation
   * @param rotation The rotation matrix
   * @param translation The translation vector
   */
  void getSymmetry(const std::string& operations, mat3d& rotation, vec3d& translation);

  /**
   * Trye to find out the bigest sphere contains in the rprim cell
   * @param rprim 3 lattice vectors
   * @return the ratdius of the sphere
   */
  double getWignerSeitzRadius(const mat3d &rprimd);

  /**
   * Trye to find out the bigest sphere contains in the rprim cell
   * @param rprim 3 lattice vectors
   * @return the ratdius of the sphere
   */
  double getWignerSeitzRadius(const double rprimd[9]);

  /**
   * Compute the rotation matrix of a rotation of angle angle around the axis axe
   * @param angle the angle of the rotation
   * @param axe the axe of the rotation
   */
  inline mat3d matRotation(double angle, const vec3d &axe) {
    const double renorm = 1./norm(axe);
    const double x = axe[0]*renorm;
    const double y = axe[1]*renorm;
    const double z = axe[2]*renorm;
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    const double dc = 1.-c;
    return mat3d({{
       x*x*dc+c  , x*y*dc-z*s, x*z*dc+y*s,
       x*y*dc+z*s, y*y*dc+c  , y*z*dc-x*s,
       x*z*dc-y*s, z*y*dc+x*s, z*z*dc+c
    }});
  }

  /*
   * Compute the rotation matrix of 3 euler angles
   * The angles are in radian
   * @param psi The angle around x
   * @param thtea The angle around y
   * @param phi The angle around z
   * @return The full rotation matrix which corresponds to the multiplication of
   * the rotation matrix around x then y then z
   */
  inline mat3d matEuler(double psi, double theta, double phi) {
    const double a = std::cos(psi);
    const double b = std::sin(psi);
    const double c = std::cos(theta);
    const double d = std::sin(theta);
    const double e = std::cos(phi);
    const double f = std::sin(phi);
    const double ad = a*d;
    const double bd = b*d;
    return mat3d( {{
        c*e, -c*f, d,
        bd*e+a*f, -bd*f+a*e, -b*c,
        -ad*e+b*f, ad*f+b*e, a*c
        }});
  }

  /**
   * From a rotation matrix, compute the three angles around x y and z according to the definition of the
   * euler matrix in matEuler function.
   * @param euler the rotation matrix
   * @return angles in radian around x y and z
   */
  inline vec3d anglesEuler(mat3d euler) {
    const double theta = std::asin(euler[2]);
    const double c = std::cos(theta);
    double psi = 0;
    double phi = 0;
    if ( std::abs(c) > 1e-4 ){
      psi = std::atan2(-euler[5]/c,euler[8]/c);
      phi = std::atan2(-euler[1]/c,euler[0]/c);
    }
    else {
      psi = 0;
      phi = std::atan2(euler[3],euler[4]);
    }
    return vec3d({{ psi,theta,phi }});
  }
}
#endif // GEOMETRY_HPP


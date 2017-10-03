/**
 * @file geometry.hpp
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

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
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
   * @param vec vector (1, 2 or 3).
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
   * Change some 3D vectors from basis.
   * @param base destination basis in the current basis.
   * @param vecs All the vectors to change.
   * @result vecs in the destination basis.
   */
  std::vector<vec3d> changeBasis(const mat3d& base, const std::vector<vec3d>& vecs);


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
        mat1[6]*mat2[0]+mat1[6]*mat2[3]+mat1[8]*mat2[6], mat1[6]*mat2[1]+mat1[6]*mat2[4]+mat1[8]*mat2[7], mat1[6]*mat2[2]+mat1[6]*mat2[5]+mat1[8]*mat2[8],
        }});
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
}
#endif // GEOMETRY_HPP


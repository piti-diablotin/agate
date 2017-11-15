/**
 * @file include/octalengths.hpp
 *
 * @brief Calculate the octahedra around an atom and compute the different
 * lengths along each axis
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


#ifndef OCTALENGTHS_HPP
#define OCTALENGTHS_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "shape/octahedra.hpp"
#include <array>

/** 
 * This class, although it can be used instead of its parent
 * is designed to calculate the angles around the 3 cartesian axis.
 * Be careful that it does not take into account the primitive cell
 */
class OctaLengths : public Octahedra {

  private :
    
    std::array<float,3> _lengths; ///< The angles around the three axis.

  protected :

  public :

    /**
     * Construct an octahedra around the atom iatom
     * @param iatom Id of the atom that is at the center of the octahedra
     * @param natom Total number of atoms to consider.
     * @param xred The full set of reduce coordinates (should of size 3*natom at least)
     * @param xcart The full set of cartesian coordinates (should of size 3*natom at least)
     * @param rprim The primitive vectors
     */
    OctaLengths(int iatom, int natom, const double *xred, const double *xcart, const double *rprim, bool opengl);

    /**
     * Copy
     * @param octa The octahedra to copy
     */
    OctaLengths(const OctaLengths& octa);

    /** 
     * Move
     * @param octa The octahedra to move
     */
    OctaLengths(OctaLengths&& octa);

    /** 
     * Copy
     * @param octa The octahedra to move
     */
    OctaLengths(const Octahedra& octa);

    /** 
     * Move
     * @param octa The octahedra to move
     */
    OctaLengths(Octahedra&& octa);

    /**
     * Destructor.
     */
    virtual ~OctaLengths();

    /**
     * Calculate the length of each axis
     * @param rprim The primitive vectors used for translation.
     * @param xcart The full set of coordinates of all atoms.
     * @param new_atoms A set of the center atom id and 3 floats giving the angles around x,y and z
     */
    virtual void build(const double *rprim, const double *xcart, u3f &new_atoms) ;

    /**
     * Access the value of the rotation around x
     * @return the rotation angle around the x cartesian axis in degree
     */
    float a() const { return _lengths[0]; }
     
    /**
     * Access the value of the rotation around y
     * @return the rotation angle around the y cartesian axis in degree
     */
    float b() const { return _lengths[1]; }

    /**
     * Access the value of the rotation around z
     * @return the rotation angle around the z cartesian axis in degree
     */
    float c() const { return _lengths[2]; }
};

#endif  // OCTALENGTHS_HPP

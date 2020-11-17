/**
 * @file include/octaangles.hpp
 *
 * @brief Calculate the octahedra around an atom and compute the different
 * rotation along each axis
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


#ifndef OCTAANGLES_HPP
#define OCTAANGLES_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "shape/octahedra.hpp"
#include <array>

/** 
 * This class, although it can be used instead of its parent
 * is designed to calculate the angles around the 3 cartesian axis.
 * Be careful that it does not take into account the primitive cell
 */
class OctaAngles : public Octahedra {

  private :
    
    std::array<float,3> _angles; ///< The angles around the three axis.
    std::array<geometry::vec3d,3> _savedBasis; ///< Saved reference basis of the octahedra

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
    OctaAngles(int iatom, int natom, const double *xred, const double *xcart, const double *rprim, bool opengl);

    /**
     * Copy
     * @param octa The octahedra to copy
     */
    OctaAngles(const OctaAngles& octa);

    /** 
     * Move
     * @param octa The octahedra to move
     */
    OctaAngles(OctaAngles&& octa);

    /** 
     * Move
     * @param octa The octahedra to move
     */
    OctaAngles(Octahedra&& octa);

    /** 
     * Copy
     * @param octa The octahedra to move
     */
    OctaAngles(const Octahedra& octa);

    /**
     * Destructor.
     */
    virtual ~OctaAngles();

    /**
     * Calculate the rotation around each axis
     * @param rprim The primitive vectors used for translation.
     * @param xcart The full set of coordinates of all atoms.
     * @param new_atoms A set of the center atom id and 3 floats giving the angles around x,y and z
     */
    virtual void build(const double *rprim, const double *xcart, u3f &new_atoms) ;

    /**
     * Compute the angle with resepect to cartesian axis if set to true
     * @param rprim The primitive vectors used for translation.
     * @param xcart The full set of coordinates of all atoms.
     * @param new_atoms A set of the center atom id and 3 floats giving the angles around x,y and z
     * @param cartBasis Do we keep the cartesian basis or not ?
     */
    virtual void buildCart(const double *rprim, const double *xcart, u3f &new_atoms, bool cartBasis) ;

    /**
     * Access the value of the rotation around x
     * @return the rotation angle around the x cartesian axis in degree
     */
    float alpha() const { return _angles[0]; }
     
    /**
     * Access the value of the rotation around y
     * @return the rotation angle around the y cartesian axis in degree
     */
    float beta() const { return _angles[1]; }

    /**
     * Access the value of the rotation around z
     * @return the rotation angle around the z cartesian axis in degree
     */
    float gamma() const { return _angles[2]; }
};

#endif  // OCTAANGLES_HPP

/**
 * @file include/octahedra.hpp
 *
 * @brief Draw an octahedra around an atom.
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


#ifndef OCTAHEDRA_HPP
#define OCTAHEDRA_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include <array>
#include "graphism/triobj.hpp"
#include "base/geometry.hpp"


/** 
 * Class that define an octahedra around an atom and plot it in openGL.
 */
class Octahedra : public TriObj {

  private :

    bool                 _build;        ///< true if the octahedra must be display (supposely a true octahedra)
    _uint                _vboLine;      ///< VBO for vertex and indices
    _uint                _lines[15];
    std::vector<unsigned>_drawAtoms;    ///< Additionnal atoms to draw

  protected :

    static const int     _top1  = 0;    ///< Top
    static const int     _top2  = 1;    ///< Bottom
    static const int     _atom1 = 2;    ///< quadrilatere 1
    static const int     _atom2 = 3;    ///< quadrilatere 2
    static const int     _atom3 = 4;    ///< quadrilatere 3 (oposite to 1)
    static const int     _atom4 = 5;    ///< quadrilatere 1 (oposite to 2)

    unsigned             _center;       ///< Id of the atom at the center of the octahedra
    std::array<int,6>    _positions;    ///< Position to form an oc.
    std::array<float,18> _shifts;       ///< Shift to apply to get the octahedra
    std::array<geometry::vec3d,3> _basis; ///< Reference basis.

  public :

    /**
     * A simple type definition to store an index and 3 floats.
     * Used when building the octahedra for additioinal stuff to be defined...
     */
    typedef std::vector< std::pair< unsigned, std::array<float,3> > > u3f;

    /**
     * Construct an octahedra around the atom iatom
     * @param iatom Id of the atom that is at the center of the octahedra
     * @param natom Total number of atoms to consider.
     * @param xred The full set of reduce coordinates (should of size 3*natom at least)
     * @param xcart The full set of cartesian coordinates (should of size 3*natom at least)
     * @param rprim The primitive vectors
     */
    Octahedra(int iatom, int natom, const double *xred, const double *xcart, const double *rprim, bool opengl);

    /** 
     * Copy
     * @param octa The octahedra to copy
     */
    Octahedra(const Octahedra& octa);

    /**
     * Move
     * @param octa The octahedra to move
     */
    Octahedra(Octahedra&& octa);

    /**
     * Destructor.
     */
    virtual ~Octahedra();

    /**
     * Build the positions of the octahedra and put it in the VBO if we use GLEXT
     * @param rprim The primitive vectors used for translation.
     * @param xcart The full set of coordinates of all atoms.
     * @param new_atoms A set of atom id and 3 floats of atom position to draw atoms around the octahedra
     */
    virtual void build(const double *rprim, const double *xcart, u3f &new_atoms) ;

    /** 
     * Draw the octahedra with the positions found into the xcart array
     * @param color The color to draw the octahedra in RGBA
     */
    void draw(float color[4]);

    /**
     * Get the atom id at the center of the octahedra
     * @return _center
     */
    int center() const { return _center; }

    /**
     * Pop method to do something if needed
     * It flush data if some are not drawn and release VBO if used
     */
    virtual void pop(){;}

    /**
     * Push method to do something if needed
     * Load the VBO if used.
     */
    virtual void push(){;}

};

#endif  // OCTAHEDRA_HPP

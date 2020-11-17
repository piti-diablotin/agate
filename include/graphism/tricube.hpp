/**
 * @file include/tricube.hpp
 *
 * @brief Draw a cube with OpenGL The faces can be with different colours 
 * to allow better visualization of properties.
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


#ifndef TRICUBE_HPP
#define TRICUBE_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#ifdef HAVE_GL
# ifdef HAVE_GLEXT
#  define GL_GLEXT_PROTOTYPES
# endif
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
# endif
#endif

#include "graphism/triobj.hpp"

/** 
 * This class draw a very basic cube with two colours available for 
 * +x and -x faces. The difference of color is set by a value between +1;-1 
 * when calling the draw function
 */
class TriCube : public TriObj {

  private :

    _uint  _nvface;   ///< Number of vertex per face of the cube.
    _uint  _niface;   ///< Number of indice to use to draw one face of the cube.


  protected :

  public :

    /**
     * Constructor.
     */
    TriCube(bool opengl);

    /**
     * Copy
     * It copies everything and creat new vbos if needed.
     * @param cube The Cube to copy for the construction
     */
    TriCube(const TriCube& cube) = delete;

    /**
     * Move
     * The VBO id are copied and everything is moved.
     * @param cube The Cube to move.
     */
    TriCube(TriCube&& cube);

    /**
     * Copy
     * It copies everything and creat new vbos if needed.
     * @param cube The Cube to copy for the construction
     * @return A perfect copy of cube
     */
    TriCube& operator = (const TriCube& cube) = delete;

    /**
     * Move
     * The VBO id are copied and everything is moved.
     * @param cube Cube to move.
     * @return Exactly cube which one is empty
     */
    TriCube& operator = (TriCube&& cube) = delete;

    /**
     * Destructor.
     */
    virtual ~TriCube();

    /**
     * Generate the template cube of length 1
     */
    void genUnit();

    /**
     * Draw a sphere or compute it and store it
     * Values of x,y,z should be between -1 and 1.
     * -1 means use full c1 color
     * 1 means use full c2 color 
     * @param c1 A 3 float array with the RGB values for the first color (0->1).
     * @param c2 A 3 float array with the RGB values for the second color (0->1).
     * @param x The mix of c1 and c2 to set the color of the +x/-x faces.
     * @param y The mix of c1 and c2 to set the color of the +x/-x faces.
     * @param z The mix of c1 and c2 to set the color of the +x/-x faces.
     */
    void draw(_float *c1, _float *c2, _float x, _float y, _float z);

    /**
     * Pop method to do something if needed
     * It flush data if some are not drawn and release VBO if used
     */
    virtual void pop();

    /**
     * Push method to do something if needed
     * Load the VBO if used.
     */
    virtual void push();

};

#endif  // TRICUBE_HPP

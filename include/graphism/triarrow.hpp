/**
 * @file include/triarrow.hpp
 *
 * @brief  Draw a arrow along the Z axis with a length of 1.
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


#ifndef TRIARROW_HPP
#define TRIARROW_HPP

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
 * Draw an arrow center around (0,0,1) with a length of 1.
 */
class TriArrow : public TriObj {

  private :

  protected :

  public :

    /**
     * Constructor.
     */
    TriArrow(bool opengl);

    /**
     * Copy
     * It copies everything and creat new vbos if needed.
     * @param arrow The Arrow to copy for the construction
     */
    TriArrow(const TriArrow& arrow) = delete;

    /**
     * Move
     * The VBO id are copied and everything is moved.
     * @param arrow The Arrow to move.
     */
    TriArrow(TriArrow&& arrow);

    /**
     * Copy
     * It copies everything and creat new vbos if needed.
     * @param arrow The Arrow to copy for the construction
     * @return A perfect copy of arrow 
     */
    TriArrow& operator = (const TriArrow& arrow) = delete;

    /**
     * Move
     * The VBO id are copied and everything is moved.
     * @param arrow Arrow to move.
     * @return Exactly arrow which one is empty
     */
    TriArrow& operator = (TriArrow&& arrow) = delete;

    /**
     * Destructor.
     */
    virtual ~TriArrow();

    /**
     * Generate the template arrow of 1.0 radius and 1.0 height.
     */
    void genUnit();

    /**
     * Draw a arrow or compute it and store it
     * @param radius The radius of the arrow base placed at (0,0,0)
     * @param height The height of the arrow placed at (0,0,0)
     */
    void draw(const _float radius, const _float height);

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

#endif  // TRIARROW_HPP

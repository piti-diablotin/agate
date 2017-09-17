/**
 * @file include/tricylinder.hpp
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


#ifndef TRICYLINDER_HPP
#define TRICYLINDER_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
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
 * Draw cylinders with VBO
 */
class TriCylinder : public TriObj {

  private :

  protected :

  public :

    /**
     * Constructor.
     */
    TriCylinder(bool opengl);

    /**
     * Copy
     */
    TriCylinder(const TriCylinder& cylinder) = delete;

    /**
     * Move
     */
    TriCylinder(TriCylinder&& cylinder);

    /**
     * Copy
     */
    TriCylinder& operator = (const TriCylinder& cylinder) = delete;

    /**
     * Move
     */
    TriCylinder& operator = (TriCylinder&& cylinder) = delete;

    /**
     * Destructor.
     */
    virtual ~TriCylinder();

    /**
     * Generate the template cylinder of 1.0 radius and 1.0 height.
     */
    void genUnit();

    /**
     * Draw a cylinder or compute it and store it
     * @param radius The radius of the cylinder base placed at (0,0,0)
     * @param height The height of the cylinder placed at (0,0,0)
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

#endif  // TRICYLINDER_HPP

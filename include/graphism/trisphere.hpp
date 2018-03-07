/**
 * @file include/trisphere.hpp
 *
 * @brief Draw spheres with triangles
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


#ifndef TRISPHERE_HPP
#define TRISPHERE_HPP

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
 *  Draw spheres with VBO
 */
class TriSphere : public TriObj{


  private :

  protected :

  public :

    /**
     * Constructor.
     */
    TriSphere(bool opengl);

    /**
     * Copy
     */
    TriSphere(const TriSphere& sphere) = delete;

    /**
     * Move
     */
    TriSphere(TriSphere&& sphere);

    /**
     * Copy
     */
    TriSphere& operator = (const TriSphere& sphere) = delete;

    /**
     * Move
     */
    TriSphere& operator = (TriSphere&& sphere) = delete;

    /**
     * Destructor.
     */
    virtual ~TriSphere();

    /**
     * Generate the template sphere of 1.0 radius.
     */
    void genUnit();

    /**
     * Draw a sphere or compute it and store it
     * @param pos The position of the sphere
     * @param color The color of the sphere
     * @param radius The radius of the sphere
     */
    virtual void draw(const _float pos[3], const _float color[3], const _float radius);

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

#endif  // TRISPHERE_HPP

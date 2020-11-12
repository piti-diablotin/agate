/**
 * @file include/tricloud.hpp
 *
 * @brief Draw a cloud of points
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


#ifndef TRICLOUD_HPP
#define TRICLOUD_HPP

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

#include "graphism/trisphere.hpp"

/** 
 *  Draw a cloud of points with VBO
 */
class TriCloud : public TriSphere {


  private :
    static const unsigned int _nmax = 500000;

  protected :
    _float  *_unitColor;     ///< Unit object indices
    _uint    _vboColor;       ///< VBO for vertex[0] and indices[1]

  public :

    /**
     * Constructor.
     */
    TriCloud(bool opengl);

    /**
     * Copy
     */
    TriCloud(const TriCloud& cloud) = delete;

    /**
     * Move
     */
    TriCloud(TriCloud&& cloud);

    /**
     * Copy
     */
    TriCloud& operator = (const TriCloud& cloud) = delete;

    /**
     * Move
     */
    TriCloud& operator = (TriCloud&& cloud) = delete;

    /**
     * Destructor.
     */
    virtual ~TriCloud();

    /**
     * Generate the template cloud of 1.0 radius.
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

#endif  // TRICLOUD_HPP

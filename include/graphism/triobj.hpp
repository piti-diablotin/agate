/**
 * @file include/triobj.hpp
 *
 * @brief Abstract class to draw OpenGL objects
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


#ifndef TRIOJB_HPP
#define TRIOJB_HPP

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

/** 
 * Abstract class for a geometric object
 */
class TriObj {

  public :

#ifdef HAVE_GL
  typedef GLfloat _float;      ///< Use the OpenGL float variable
  typedef GLuint  _uint;       ///< Use the OpenGL unisgned variable
#else
  typedef float _float;        ///< Use the OpenGL float variable
  typedef unsigned int _uint;  ///< Use the OpenGL unsigned variable
#endif
    
    /**
     * How to draw an object;
     */
    enum Drawing { FILL, SILHOUETTE };

    /**
     * What technic to use to draw
     */
    enum GLMode { VERTEX, ARRAY, VBO };

  private :

  protected :

    bool    _opengl;        ///< Do we use opengl ?
    _float *_unitVertex;    ///< Unit object vertex normals and color (depending on the object)
    _uint  *_unitIndex;     ///< Unit object indices
    _uint   _vbos[2];       ///< VBO for vertex[0] and indices[1]
    unsigned int _division; ///< Number of subdivisons for the object.
    unsigned int _nvertex;  ///< Total number of vertex. Should be the size of the _unitVertex
    unsigned int _nindices; ///< Total number of indices to draw obj. Should be the size of _unitIndex
    Drawing      _drawing;  ///< How the object should be drawn
    GLMode       _mode;     ///< How to draw (VBO,VAO,VERTEX)

  public :
    
    /**
     * Constructor.
     */
    TriObj(bool opengl);
    
    /**
     * Copy constructor
     * It copies everything and creat new vbos if needed.
     * @param obj The object to copy for the construction
     */
    TriObj(const TriObj& obj);

    /**
     * Move constructor
     * The VBO id are copied and everything is moved.
     * @param obj Object to move.
     */
    TriObj(TriObj &&obj);

    /**
     * Copy operator
     * It copies everything and creat new vbos if needed.
     * @param obj The object to copy for the construction
     */
    TriObj& operator = (const TriObj& obj) = delete;

    /**
     * Move operator
     * The VBO id are copied and everything is moved.
     * @param obj Object to move.
     */
    TriObj& operator = (TriObj &&obj) = delete;

    /**
     * Destructor.
     */
    virtual ~TriObj();

    /**
     * Generate the unit object (normalzied) to be use later
     */
    virtual void genUnit() {;}

    /**
     * Set the number of division
     * @param div The number of divions 
     */
    void division(const unsigned int div) {
      ( div == 0 ) ? _division = 1 : _division = div;
    }

    /**
     * Set the drawing method
     * @param d The way the rendering is done 
     */
    void drawing(Drawing d) {
      _drawing = d;
#ifdef HAVE_GL
      if ( _opengl )
        ( _drawing == SILHOUETTE ) ? glPolygonMode(GL_FRONT_AND_BACK,GL_LINE) :
          glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
#endif
    }

    /**
     * Pop method to do something if needed
     * It flush data if some are not drawn and release VBO if used
     */
    virtual void pop() = 0;

    /**
     * Push method to do something if needed
     * Load the VBO if used.
     */
    virtual void push() = 0;

};

#endif  // TRIOJB_HPP

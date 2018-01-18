/**
 * @file include/textrender.hpp
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


#ifndef TEXTRENDER_HPP
#define TEXTRENDER_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif


//#ifdef HAVE_GLEXT
//# ifdef __APPLE__
//#  include <OpenGL/glext.h>
//# else
//#  include <GL/glext.h>
//# endif
//#endif
//
//#ifdef HAVE_GL
//# ifdef __APPLE__
//#  include <OpenGL/gl.h>
//# else
//#  include <GL/gl.h>
//# endif
//#endif

#include "graphism/render.hpp"
#include "base/exception.hpp"
#include <iostream>

/** 
 *
 */
struct TextRender {

  private :

  protected :

  public :

    bool                 _isOk;     ///< Save the state of the render (true if can draw something, false otherwise)
    unsigned char        _color[3]; ///< Color for rendering text;
    Render               _render;   ///< Render to use;
    Render::BufferRender _buffer;   ///< buffer to acces data;

    /** 
     * Cosntructor
     */
    TextRender() : _isOk(false), _color(), _render(), _buffer(Render::BufferRender(200,200) ) {
      try {
        _render = Render(DEFAULT_FONT,20,RENDERGRAY);
        _isOk = true;
      }
      catch( Exception &e ) {
        e.ADD("Won't display any information on the screen",ERRWAR);
        std::cerr << e.fullWhat() << std::endl;
        _isOk = false;
      }
      _color[0] = 255;
      _color[1] = 255;
      _color[2] = 255;
    }

    /**
     * Quickly generate the buffer image of a string with the correct color
     * @param str The string to render
     * @param center True if we apply an offset to center the text around the current origin.
     */
    inline void render(const std::string str, bool center = false) {
      _render.render(str,_buffer,_color);
#ifdef HAVE_GL
      if ( center ) {
        GLfloat pos[4];
        GLfloat dist;
        glGetFloatv(GL_CURRENT_RASTER_POSITION,&pos[0]);
        glGetFloatv(GL_CURRENT_RASTER_DISTANCE,&dist);
        if ( std::abs(dist)  < 1e-2 ) return;
        pos[0]-=_buffer.cols()/2;
        pos[1]-=_buffer.rows()/2;
#ifdef HAVE_GLEXT
        glWindowPos3fv(&pos[0]);
#endif
      }
      glDrawPixels(_buffer.cols(), _buffer.rows(), GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, _buffer.getPtr());
#else
      (void) center;
#endif
    }

};

#endif  // TEXTRENDER_HPP

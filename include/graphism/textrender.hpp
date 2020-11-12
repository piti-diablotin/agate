/**
 * @file include/textrender.hpp
 *
 * @brief 
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


#ifndef TEXTRENDER_HPP
#define TEXTRENDER_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "graphism/render.hpp"

/** 
 *
 */
struct TextRender {

  private :

  protected :

  public :

    bool                 _doRender; ///< True if render will be displayed on screen
    bool                 _isOk;     ///< Save the state of the render (true if can draw something, false otherwise)
    unsigned char        _color[3]; ///< Color for rendering text;
    Render               _render;   ///< Render to use;
    Render::BufferRender _buffer;   ///< buffer to acces data;

    /** 
     * Cosntructor
     */
    TextRender();

    /**
     * Quickly generate the buffer image of a string with the correct color
     * @param str The string to render
     * @param center True if we apply an offset to center the text around the current origin.
     */
    void render(const std::string str, bool center = false);

};

#endif  // TEXTRENDER_HPP

/**
 * @file render.hpp
 *
 * @brief Definition of class to transform text into bitmap
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

#ifndef RENDERER_HPP
#define RENDERER_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif


#include <cstddef>
#include <string>
#include "graphism/buffer.hpp"

struct FT_LibraryRec_;
struct FT_FaceRec_;
struct FT_GlyphSlotRec_;

/// Class that allows to render a string in pixel.
class Render {
  public:
    enum class Mode {Mono,Gray};

  private :

#ifdef HAVE_FREETYPE
    FT_LibraryRec_*   _library;      ///< handle to library.
    FT_FaceRec_*      _face;         ///< handle to face object.
    FT_GlyphSlotRec_* _slot;         ///< handle the buffer of the face render.
#endif
    int            _size;         ///< size in pixel of the rendering (height).
    int            _pixelRatio;   ///< Height/widht of a pixel (can be 2 if in terminal).
    std::string    _fontfile;     ///< Font file to use for rendering.
    Mode           _renderMode;   ///< Render to load : gray or monochrome.
    unsigned char* _temp;         ///< Temporary array containing all glyph for on font/size.

    /// Regenerate the _temp attribut that contains all pre rendered glyph.
    void regenerate();

  public :

    /// Define the type of Buffer used in Render
    typedef Buffer<unsigned int> BufferRender;


    static const unsigned int CKBOARD = 255;  ///< Special character for fully colored pixel (available only in ncurse with exASCII ...)

    /// Construct an empty render.
    Render();

    /// Construct a render with font and size.
    /// @param filename The font use for the rendering.
    /// @param size The size in pixel for the rendering
    /// @param renderMode Mode to render the text.
    Render(const std::string& filename, const size_t size, const Mode renderMode = Mode::Mono);

    /// Copy a rendere if needed.
    /// @param render The render to copy.
    Render(const Render& render);

    /// Move an existing render into a new one.
    /// @param render The render to move.
    Render(Render&& render);

    /// Destroy a render (deallocate memory).
    ~Render();

    /// Free / Destroy all pointers.
    void free();

    /// Copy a rendere if needed.
    /// @param render The render to copy.
    /// @return The new render created from input.
    Render& operator = (const Render& render);

    /// Move an existing render into a new one.
    /// @param render The render to move.
    /// @result The new render moved from input.
    Render& operator = (Render&& render);


    /// Set the font to use for the rendering process.
    /// @param filename The path to the font file.
    void setFont(const std::string& filename);

    /// Set the size ( the height ) of the rendered characters.
    /// @param size The size in pixel of a letter/symbol
    void setSize(const size_t size);

    /// Set the ratio height/width of one pixel.
    /// @param ratio The ratio height/width of one pixel.
    void setPixelRatio(const int ratio);

    /// Increase or decrease the font size.
    /// @param toadd Number of pixels to add or remove from the font size
    void setSizeR(const int toadd);

    /// Change render mode.
    /// @param mode New mode to use.
    void setRender(const Mode mode);

    /// Render a string to buffer that has a correct size value
    /// @param string The text to render
    /// @param buffer The buffer to use to do the rendering.
    /// @param color the color of the buffer, text is render via alpha channel
    void render(const char* string, BufferRender& buffer, const unsigned char color[]);

    /// Wrapper for string to 'C' string
    /// @param string The text to render
    /// @param buffer The buffer to use to do the rendering.
    /// @param color the color of the buffer, text is render via alpha channel
    inline void render(const std::string& string, BufferRender& buffer, const unsigned char color[]) {
      this->render(string.c_str(),buffer,color);
    }

    /// Get the font size.
    /// @return the font size.
    inline int getSize() {
      return _size;
    }

    /// Allow to know if the render is set or not so it can be used.
    /// @return True if ready, false otherwise.
    inline bool isReady() {
      return (_temp != NULL);
    }

};

#endif // RENDERER_HPP

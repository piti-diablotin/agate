/**
 * @file render.cpp
 *
 * @brief Implementation of the Render class to render a string into a bitmap.
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


#include "graphism/render.hpp"
#include "base/exception.hpp"
#include <iostream>
#include <sstream>
#include <utility>
#include <cmath>
#include <bitset>


//
Render::Render() : 
#ifdef HAVE_FREETYPE
  _library(nullptr),
  _face(nullptr),
  _slot(nullptr),
  _size(0),
  _pixelRatio(1),
#endif
  _fontfile(""),
  _renderMode(RENDERMONO),
  _temp(nullptr)
{
#ifdef HAVE_FREETYPE
  int error = FT_Init_FreeType( &_library ); 
  if ( error ) 
    throw EXCEPTION("Error while initializing FT_library.",ERRDIV);
#else
  Exception e = EXCEPTION("FreeType not available.",ERRWAR);
  std::clog << e.fullWhat();
#endif
}


//
Render::Render(const std::string& filename, const size_t size, const long renderMode) : 
#ifdef HAVE_FREETYPE
  _library(nullptr),
  _face(nullptr),
  _slot(nullptr),
  _size(size),
  _pixelRatio(1),
#endif
  _fontfile(filename),
  _renderMode(renderMode),
  _temp(nullptr)
{
  try {
#ifdef HAVE_FREETYPE
    int error = FT_Init_FreeType( &_library ); 
    if ( error != 0 ) 
      throw EXCEPTION("Error while initializing FT_library.",ERRDIV);
    if ( _fontfile != "" ) {
      this->setFont(filename);
      this->setSize(size);
      this->setRender(renderMode);
    }
#else
    Exception e = EXCEPTION("FreeType not available.",ERRWAR);
    std::clog << e.fullWhat();
#endif
  }
  catch ( Exception& e ) {
    e.ADD("Constructor failed.",ERRDIV);
    throw e;
  }
}


//
Render::Render(const Render& render) :
#ifdef HAVE_FREETYPE
  _library(nullptr),
  _face(nullptr),
  _slot(nullptr),
  _size(render._size),
  _pixelRatio(render._pixelRatio),
#endif
  _fontfile(render._fontfile),
  _renderMode(render._renderMode),
  _temp(nullptr)
{
  try {
#ifdef HAVE_FREETYPE
    int error = FT_Init_FreeType( &_library ); 
    if ( error != 0 ) 
      throw EXCEPTION("Error while initializing FT_library.",ERRDIV);
    this->setFont(_fontfile);
    this->setSize(_size);
    this->setRender(_renderMode);
#else
    Exception e = EXCEPTION("FreeType not available.",ERRWAR);
    std::clog << e.fullWhat();
#endif
  }
  catch ( Exception& e ) {
    e.ADD("Constructor failed",ERRDIV);
  }
  //std::clog << "Render copied (constructor)" << std::endl;
}


//
Render::Render(Render&& render) :
#ifdef HAVE_FREETYPE
  _library(render._library),
  _face(render._face),
  _slot(render._slot),
  _size(render._size),
  _pixelRatio(render._pixelRatio),
#endif
  _fontfile(std::move(render._fontfile)),
  _renderMode(render._renderMode),
  _temp(render._temp)
{
#ifdef HAVE_FREETYPE
  render._library = nullptr;
  render._face = nullptr;
  render._slot = nullptr;
#endif
  render._temp = nullptr;
  //std::clog << "Render moved (constructor)" << std::endl;
}


//
Render& Render::operator = (const Render& render) {
  // Delete previous render if exists;
  this->free();

  _fontfile = render._fontfile;
  _renderMode = render._renderMode;

  try {
#ifdef HAVE_FREETYPE
    _size =render._size;
    _pixelRatio = render._pixelRatio;
    int error = FT_Init_FreeType( &_library ); 
    if ( error != 0 ) 
      throw EXCEPTION("Error while initializing FT_library.",ERRDIV);
    this->setFont(_fontfile);
    this->setSize(_size);
    this->setRender(_renderMode);
#else
    Exception e = EXCEPTION("FreeType not available.",ERRWAR);
    std::clog << e.fullWhat();
    int error = 1;
#endif
  }
  catch ( Exception& e ) {
    e.ADD("Assignement failed",ERRDIV);
  }
  //std::clog << "Render copied (assignement)" << std::endl;
  return *this;
}


//
Render& Render::operator = (Render&& render) {
  this->free();
#ifdef HAVE_FREETYPE
  _library = render._library;
  _face = render._face;
  _slot = render._slot;
  _size = render._size;
  _pixelRatio = render._pixelRatio;
  render._library = nullptr;
  render._face = nullptr;
  render._slot = nullptr;
#endif
  _fontfile = std::move(render._fontfile);
  _renderMode = render._renderMode;
  _temp = render._temp;
  render._temp = nullptr;
  //std::clog << "Render moved (assignement)" << std::endl;
  return *this;
}

//
Render::~Render() {
  this->free();
}


//
void Render::free() {
#ifdef HAVE_FREETYPE
  _size = 0;
  if ( _face != nullptr ) {
    FT_Done_Face    ( _face );
    _face = nullptr;
    _slot = nullptr; // _slot is a pointer to _face->slot.
  }
  if ( _library != nullptr ) {
    FT_Done_FreeType( _library );
    _library = nullptr;
  }
#endif
  if ( _temp != nullptr ) {
    delete[] _temp;
    _temp = nullptr;
  }
}


//
void Render::setFont(const std::string& filename) {
  std::string backup = _fontfile;
  _fontfile = filename;
  try{
#ifdef HAVE_FREETYPE
    int error = FT_New_Face( _library, filename.c_str(), 0, &_face ); 
    if ( error == FT_Err_Unknown_File_Format ) { 
      _fontfile.clear();
      throw EXCEPTION("Error font format unsupported.",ERRDIV);
    } 
    else if ( error ) { 
      _fontfile.clear();
      throw EXCEPTION("Error loading or reading font file.",ERRDIV);
    }
    _slot = _face->glyph;
    if ( _temp != nullptr ) {
      this->regenerate();
    }  
#endif
  }
  catch ( Exception& e ) {
    _fontfile = filename;
    e.ADD("Unable to generate font bitmaps.",ERRDIV);
    throw e;
  }
}



//
void Render::setSize(const size_t size) {
#ifdef HAVE_FREETYPE
  FT_Int backup = _size;
  _size = size;
  // Set pixel size
  try{
    if ( !_fontfile.empty() ) {
      int error = FT_Set_Pixel_Sizes( 
          _face, /* handle to face object */ 
          _pixelRatio*_size,    /* pixel_width (the pixel is X times taller than wide so make it square) */
          _size ); /* pixel_height */ 
      if ( error ) {
        std::ostringstream str_tmp;
        str_tmp << "Error setting pixel size " << _size*_pixelRatio << "px by " << _size << "px.";
        throw EXCEPTION(str_tmp.str(),ERRDIV);
      }
      if ( _temp != nullptr ) delete[] _temp;
      _temp = new unsigned char[(_pixelRatio*_size*_size+2)*(127-32)]; // Add 2B for storing advance value
      for ( int t = 0 ; t < (_pixelRatio*_size*_size+2)*(127-32) ; ++t )
        _temp[t] = 0;
      this->regenerate();
    }
  }
  catch ( Exception& e ) {
    _size = backup;
    e.ADD("Impossible to generate characters.",ERRDIV);
    throw e;
  }
#else
  (void) size;
#endif
  //_slot = _face->glyph;
}


//
void Render::setSizeR(const int toadd) {
#ifdef HAVE_FREETYPE
  if ( toadd < 0  &&  -toadd >= _size ) 
    _size=1;
  //throw EXCEPTION("Font size is going to be set to 0",ERRWAR);
  _size += toadd;
  this->setSize(_size);
#else
  (void) toadd;
#endif
}


//
void Render::render(const char* string, BufferRender& buffer, const unsigned char color[]) {
#ifdef HAVE_FREETYPE
  size_t num_chars = strlen(string);
  ssize_t line = _pixelRatio*_size;
  size_t fullsize = _size*line+2; // +2 for advance value;
  const unsigned int cc = static_cast<unsigned int>(color[0])*256*256*256+static_cast<unsigned int>(color[1])*256*256+static_cast<unsigned int>(color[2])*256;

  if ( _fontfile.empty() ) return;
  try {
    // Find the total number of pixel for the width.
    size_t fullLine = 0;
    for ( size_t c = 0; c < num_chars; c++ ) {
      //unsigned char b0 = _temp[(string[c]-32)*(2+_size*line)];//b0;
      //unsigned char b1 = _temp[(string[c]-32)*(2+_size*line)+1];//b1;
      //fullLine += (unsigned short)b0*256+(unsigned short)b1;
      fullLine += (size_t) *((unsigned short*) (_temp+(string[c]-32)*(2+_size*line)));
    }

    buffer.setSize(_size,fullLine);

    size_t offset = 0;
    for ( size_t c = 0; c < num_chars; c++ ) {
      unsigned short advance = *((unsigned short*) (_temp+(string[c]-32)*(2+_size*line)));
      size_t begin = (string[c]-32)*fullsize+2;
      for ( FT_Int i = _size-1, ii =0 ; i >= 0; --i, ++ii ) {
        for ( FT_Int j = 0; (j < line && j+offset < fullLine); ++j) {
          buffer(i,j+offset) = cc+(unsigned int)_temp[begin + ii*line + j];
        }
      }
      offset += advance;
    }
  }
  catch ( Exception& e ) {
    std::stringstream message;
    message << "Error in rendering string \"" << string << "\"";
    e.ADD(message.str(),ERRDIV);
    throw e; 
  }
#else
  (void) string;
  (void) buffer;
  (void) color;
#endif
}

void Render::regenerate() {
#ifdef HAVE_FREETYPE
  FT_Int pen_x = 0;
  FT_Int pen_y = _size-(_size>>2);
  FT_Bitmap*  bitmap = nullptr;
  FT_Int start_x, start_y, x_max;
  FT_Int line = _pixelRatio*_size;

  if (_size==0) return;
  if ( _fontfile.empty() ) return;

  for ( char c = 0, C=32; C < 127; ++C, ++c ) {
    using namespace std;
    try {
      /* load glyph image into the slot (erase previous one) */
      int error = FT_Load_Char( _face, C, _renderMode );
      if ( error ) {
        std::stringstream message ; message << "Error loading charachter " << C << ".";
        throw EXCEPTION(message.str(),ERRDIV);
      }

      bitmap = &_slot->bitmap;
      start_x = max(pen_x,pen_x + _slot->bitmap_left);
      start_y = (2*(c+1))+(c*_size + pen_y- _slot->bitmap_top)*line; // Leave 0 and 1 for first char advance value
      start_y = max(start_y,c*(2+_size*line)+2);

      *((unsigned short*) (_temp+c*(2+_size*line))) = ((_slot->advance.x)>>6);

      x_max = start_x + bitmap->width;

      switch (_renderMode) {
        case RENDERMONO : {
                            //*
                            for ( FT_Int i = start_y, q = 0; q < min((FT_Int)bitmap->rows,_size); i+=line, ++q ) { // each line
                              for ( FT_Int j = start_x, p = 0; p < bitmap->pitch ; ++p ) { // each byte 
                                unsigned char mybyte = bitmap->buffer[q * bitmap->pitch + p];
                                for ( FT_Int bit = 0; (bit < 8 && j <x_max) ; ++bit, ++j ) {
                                  _temp[i+j]= ( mybyte & (128>>bit) ) ? Render::CKBOARD : 0;
                                }
                              }
                            }
                            break;
                          }
        case RENDERGRAY : {
                            for ( FT_Int i = start_y, q = 0; q < min((FT_Int)bitmap->rows,_size); i+=line, ++q )
                              for ( FT_Int j = start_x, p = 0; j < x_max; ++j, ++p )
                                _temp[i+j] |= bitmap->buffer[q * bitmap->width + p];
                            break;
                          }
        default : {
                    throw EXCEPTION("Render mode not implemented.",ERRDIV);
                    break;
                  }
      }
    }
    catch (Exception& e) {
      if ( e.getReturnValue() == ERRDIV ) {
        e.ADD("Cancellation.",ERRDIV);
        throw e;
      }
      else if ( e.getReturnValue() == ERRWAR ) {
        std::cerr << e.what() << std::endl;
      }
    }
  }
#endif
}

/**
 * @file src/imagesaver.cpp
 *
 * @brief Implement the export functions
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


#include "graphism/imagesaver.hpp"
#include "base/exception.hpp"
#include <fstream>
#include <cstdio>
#include <cstring>
#ifdef HAVE_LIBJPEG
#include <jpeglib.h>
#endif
#ifdef HAVE_LIBPNG
#include <png.h>
#endif

//
ImageSaver::ImageSaver() :
  _quality(90),
  _basename(PACKAGE_NAME),
#ifdef HAVE_LIBPNG
  _format(png),
  _ext(".png")
#elif defined HAVE_LIBJPEG
  _format(jpeg),
  _ext(".jpeg")
#else
  _format(ppm),
  _ext(".ppm")
#endif
{
  ;
}

//
ImageSaver::~ImageSaver() {
  ;
}

//
void ImageSaver::setFormat(ImageType format) {
  switch(format) {
    case jpeg : {
#ifdef HAVE_LIBJPEG
                  _format = jpeg;
                  _ext = ".jpeg";
                  throw EXCEPTION("Image format set to jpeg.",ERRCOM);
#else
                  throw EXCEPTION("Jpeg format not available.\nConsider compiling with libjpeg.\nUsing ppm",ERRWAR);
#endif
                  break;
                }

    case png : {
#ifdef HAVE_LIBPNG
                 _format = png;
                 _ext = ".png";
                 throw EXCEPTION("Image format set to png.",ERRCOM);
#else
                 throw EXCEPTION("Png format not available.\nConsider compiling with libpng.\nUsing ppm.",ERRWAR);
#endif
                 break;
               }
    case ppm : {
                 _format = ppm;
                 _ext = ".ppm";
                 throw EXCEPTION("Image format set to ppm.",ERRCOM);
                 break;
               }
    default : {
                throw EXCEPTION("Unrecognize format (jpeg|png|ppm).\nUsing ppm",ERRDIV);
                break;
              }
  }
}

//
void ImageSaver::save(int width, int height, char *image, std::string& suff) {
  std::string filename = _basename+suff+_ext;

  try {
    switch (_format) {
      case ppm :{
                  toPpm(width, height,image,filename);
                  break;
                }
      case jpeg :{
                   toJpeg(width, height,image,filename);
                   break;
                 }
      case png :{
                  toPng(width, height,image,filename);
                  break;
                }
      default : {
                  throw EXCEPTION("Unknow format (jpeg|png|ppm)",ERRDIV);
                  break;
                }
    }
    std::clog << "Snapshot saved to " << filename << std::endl;
  }
  catch(Exception &e) {
    e.ADD("Error exporting image to "+filename,ERRDIV);
    throw e;
  }
}

void ImageSaver::toPpm(int width, int height, char *image, std::string filename){
  std::ofstream file;
  file.open(filename.c_str(),std::ios::out);
  if ( !file )
    throw EXCEPTION("Unable to open file "+filename+" for writing PPM image",ERRDIV);
  file << "P6" << std::endl; // Write in binary mode.
  file << width << " " << height << std::endl;
  file << "255" << std::endl;
  int row_stride = 3*width;
  try {
    for ( int row = height-1 ; row >= 0 ; --row ){
      for ( int col = 0 ; col < width ; ++ col )
        file << image[row*row_stride+col*3  ] << image[row*row_stride+col*3+1] << image[row*row_stride+col*3+2];
    }
    file.close();
  }
  catch(...) {
    Exception e;
    try {
      if ( file.is_open() ) file.close();
    }
    catch (...) {
      e.ADD("Something really bad happend...",ERRDIV);
    }
    e.ADD("An error happend during PPM export",ERRDIV);
    throw e;
  }
}

void ImageSaver::toJpeg(int width, int height, char *image, std::string filename){
#ifdef HAVE_LIBJPEG
  FILE *file = fopen(filename.c_str(), "wb");
  if ( !file )
    throw EXCEPTION("Unable to open file "+filename+" for writing JPEG image",ERRDIV);

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr       jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, file);

  cinfo.image_width      = width;
  cinfo.image_height     = height;
  cinfo.input_components = 3;
  cinfo.in_color_space   = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality (&cinfo, _quality, TRUE);
  jpeg_start_compress(&cinfo, TRUE);

  JSAMPROW row_pointer;          /* pointer to a single row */
  int row_stride = 3*width;
  for ( int row = height-1 ; row >= 0 ; --row ){
    row_pointer = (JSAMPROW) &image[row*(row_stride)];
    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);
  fclose(file);
#else
  throw EXCEPTION("Jpeg support is not included.",ERRDIV);
  std::cout << width << height << image[0] << filename << std::endl;
#endif
}

void ImageSaver::toPng(int width, int height, char *image, std::string filename){
#ifdef HAVE_LIBPNG
	FILE *file = fopen(filename.c_str(), "wb");
  if ( !file )
    throw EXCEPTION("Unable to open file "+filename+" for writing PNG image",ERRDIV);

  png_structp png_ptr = nullptr;
  png_infop info_ptr = nullptr;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (png_ptr == nullptr)
    throw EXCEPTION("Could not allocate png struct",ERRDIV);

  // Initialize info structure
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr) {
    png_destroy_write_struct(&png_ptr, nullptr);
    throw EXCEPTION("Could not allocate info struct",ERRDIV);
  }

  // Check error
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, nullptr);
    throw EXCEPTION("Error during PNG creation",ERRDIV);
  }

  // 0 means no compression; 9 max compression
  png_set_compression_level(png_ptr,(100-_quality)*9/100);

  png_init_io(png_ptr, file);

  // Write header (8 bit colour depth)
  png_set_IHDR(png_ptr, info_ptr, width, height,
      8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  // Set title and author
  char key[100];
  char value[100];

  strcpy(key,"Title");
  strcpy(value,filename.substr(0,filename.find("_")).c_str()); 

  png_text title_text;
  title_text.compression = PNG_TEXT_COMPRESSION_NONE;

  title_text.key = key;
  title_text.text = value;
  png_set_text(png_ptr, info_ptr, &title_text, 1);

  strcpy(key,"Author");
  strcpy(value,PACKAGE_NAME);
  title_text.key = key;
  title_text.text = value;
  png_set_text(png_ptr, info_ptr, &title_text, 1);

  png_write_info(png_ptr, info_ptr);


  // Allocate memory for one row (3 bytes per pixel - RGB)
  //row = new png_byte[3*width];

  // Write image data
  for (int row = height-1 ; row >= 0 ; --row) {
    png_write_row(png_ptr, (png_bytep) &image[row*3*width]);
  }

  // End write
  png_write_end(png_ptr, nullptr);
  fclose(file);
  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, nullptr);
#else
  throw EXCEPTION("Png support is not included.",ERRDIV);
  std::cout << width << height << image[0] << filename << std::endl;
#endif
}

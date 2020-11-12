/**
 * @file include/imagesaver.hpp
 *
 * @brief Transform a PPM image (raw bitmap picture) into different formats.
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


#ifndef IMAGESAVER_HPP
#define IMAGESAVER_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include<string>
#include "base/exception.hpp"

/** 
 * Save some parameters like, compression, basename to know how to 
 */
class ImageSaver {

  public :

    /**
     * Store the compression type desired.
     */
    enum ImageType { jpeg=0, png=1, ppm=2 };

  private :

    int         _quality;    ///< Between 0 and 100. Level of compression for export
    std::string _basename;   ///< Basename for saving the image.
    ImageType   _format;     ///< Format to use to export the image.
    std::string _ext     ;   ///< Extension for the file (.jpeg .png .ppm)

  protected :

    /**
     * Save a buffer into a PPM image format (raw data of the buffer)
     * @param width Number of pixel in width
     * @param height Number of pixel in heigth.
     * @param image The buffer containing all pixel (3*sizeof(char) per pixel RBG)
     * @param filename Name of the image to creat
     */
    void toPpm(int width, int height, char *image, std::string filename);

    /**
     * Save a buffer into a JPEG image format (raw data of the buffer)
     * @param width Number of pixel in width
     * @param height Number of pixel in heigth.
     * @param image The buffer containing all pixel (3*sizeof(char) per pixel RBG)
     * @param filename Name of the image to creat
     */
    void toJpeg(int width, int height, char *image, std::string filename);

    /**
     * Save a buffer into a PNG image format (raw data of the buffer)
     * @param width Number of pixel in width
     * @param height Number of pixel in heigth.
     * @param image The buffer containing all pixel (3*sizeof(char) per pixel RBG)
     * @param filename Name of the image to creat
     */
    void toPng(int width, int height, char *image, std::string filename);

  public :

    /**
     * Constructor.
     */
    ImageSaver();

    /**
     * Destructor.
     */
    ~ImageSaver();

    /**
     * Set the basename to use for filename
     * @param basename The basename to use + suffix during export.
     */
    inline void setBasename(const std::string& basename) 
    { _basename = basename; }

    /**
     * Set the compression level for library that compress data.
     * @param comp Compression level between 0 and 100.
     */
    inline void setQuality(int comp) { 
      if ( (_format!=ppm) && (comp > 100 || comp < 0) )
        throw EXCEPTION("Bad quality value.\nIgnoring.",ERRWAR);
      _quality = comp;
    }

    /**
     * Set the format for export
     * @param format The format to use. Can be ImageSaver::jpeg,ImageSaver::ppm,ImageSaver::png
     */
    void setFormat(ImageType format);

    /**
     * Save the image in the correct format
     * @param width Width of the image in number of pixel
     * @param height Height of the image in number of pixel
     * @param image The buffer image with 3 bytes per pixel
     * @param suff The suffix to append to the name of the file.
     */
    void save(int width, int height, char *image, std::string& suff);

    /**
     * Getter for the format
     * @return the format of the image (jpeg/png/ppm)
     */
    ImageType getFormat() const { return _format; };

    /**
     * Getter for the quality of the compressed image
     * It may not be relevant for non-compressed formats
     * @return the quality of the image (0 bad 100 very good).
     */
    int getQuality() const { return _quality; }

};

#endif  // IMAGESAVE_HPP

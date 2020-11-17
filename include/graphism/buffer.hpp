/**
 * @file include/buffer.hpp
 *
 * @brief easy buffer handle
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of Agate
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
 * along with Chronox.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BUFFER_HPP
#define BUFFER_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif


#include <cstring>
#include <ostream>
#include "base/exception.hpp"

/// @def MAX(i,j)
/// @brief Return the maximum value of i and j.
#define MAX(i,j) (i>j ? i : j)


/// @def MIN(i,j)
/// @brief Return the minimun value of i and j.
#define MIN(i,j) (i>j ? j : i)

/// Easy buffer handle.
template< class T >
class Buffer {

  private :

    T*      _buffer; ///< The buffer itself
    size_t  _rows;   ///< Number of rows -> height
    size_t  _cols;   ///< Numver of cols -> widht
    size_t  _size;   ///< allocated memory in case of reallocation. rows*cols <= size.
    char    _type;   ///< Type of buffer (text/image/...)

    /// Return the indice for row i col j .
    /// @param i the row to access.
    /// @param j the column to access.
    /// @return the indice to access _buffer.
    inline size_t elt(const size_t i, const size_t j) const {
      return i*_cols+j;
    }

  public :

    const static char TYPE_IMAGE = 1; ///< Type for printing what is in the buffer exactly
    const static char TYPE_TEXT = 2;  ///< Type for printing in B(>=1)&W(<1) the buffer.
    const static char TYPE_NUM = 3;  ///< Type for printing numbers.

    /// Construct an empty buffer;
    Buffer();

    /// Construct a buffer and allocate memory.
    /// @param rows Number of row for the buffer.
    /// @param cols Number of col for the buffer.
    Buffer(const size_t rows, const size_t cols);

    /// Copy an existing buffer. It will only allocate the strict minimum of memory.
    /// @param buffer The buffer to copy.
    Buffer(const Buffer& buffer);

    /// Move an existing buffer. It is much more efficient than a copy constructor if needed.
    /// @param buffer The buffer to move.
    Buffer( Buffer&& buffer);

    /// Deallocate the buffer and set everything to 0/NULL.
    ~Buffer();

    /// Access to an element of the buffer.
    /// @param row The row indice to access.
    /// @param col The col indice to access.
    /// @return The value at line row and column col of the buffer.
    inline const T operator() (const size_t row, const size_t col) const {
      size_t elt;
      if ( (elt=this->elt(row,col)) > _size )
        throw EXCEPTION("Reading out of memory.",ERRMEM);
      return _buffer[elt];
    }

    /// Set an element of the buffer.
    /// @param row The row indice to access.
    /// @param col The col indice to access.
    /// @return The reference of the element to modify/set.
    inline T& operator() (const size_t row, const size_t col) {
      size_t elt;
      if ( (elt=this->elt(row,col)) > _size )
        throw EXCEPTION("Writing out of memory.",ERRMEM);
      return _buffer[elt];
    }

    /// Get the number of lines of the buffer.
    /// @return The number of lines of the buffer.
    inline size_t rows() const {
      return _rows;
    }

    /// Get the number of coloumns of the buffer.
    /// @return The number of column of the buffer.
    inline size_t cols() const {
      return _cols;
    }

    /// Resize the buffer and reinitialize to 0.
    /// @param rows The new number of lines.
    /// @param cols The new number of cols.
    void setSize(const size_t rows, const size_t cols);
    
    /// Set all bytes to 0 in the buffer.
    inline void clear(unsigned char value=0){
      memset(_buffer,value,_size*sizeof(T));
    }

    /// Set the type of Buffer if need to print it
    /// @param type The type for output formatting
    /// @see Buffer::TYPE_IMAGE Buffer::TYPE_TEXT
    inline void setType(const char type){
      // Not very safe but just used for operator << so should not matter.
      _type = type;
    }
    
    /// Free memory.
    void free();

    /// Empty the buffer in the output stream.
    /// @param out The output stream to use.
    /// @param buffer The buffer to output.
    friend std::ostream& operator << (std::ostream& out, const Buffer<T>& buffer) {
      out << "Buffer: " << buffer._cols << "x" << buffer._rows << std::endl;
      out << '+';
      for (size_t j=0;j<buffer._cols;++j)
        out << '-';
      out << '+' << std::endl;
      for ( size_t i=0;i<buffer._rows;++i ){
        out << '|';
        switch ( buffer._type ) {
          case Buffer::TYPE_IMAGE : {
                                      for (size_t j=0;j<buffer._cols;++j)
                                        out << buffer._buffer[buffer.elt(i,j)];
                                      break;
                                    }
          case Buffer::TYPE_TEXT : {
                                     for (size_t j=0;j<buffer._cols;++j)
                                       out << (buffer._buffer[i*buffer._cols+j] < 1 ? ' ' : '@' );
                                     break;
                                   }
          case Buffer::TYPE_NUM : {
                                     for (size_t j=0;j<buffer._cols;++j) {
                                       out << (unsigned) buffer._buffer[buffer.elt(i,j)];
                                       if ( j < buffer._cols-1 ) out << ":";
                                     } 
                                     break;
                                   }
          default : {
                                     throw EXCEPTION("Unknown type for output rendering. Use Buffer<T>::setType(Buffer<T>::TYPE_XXX)",ERRDIV);
                                     break;
                                   }
        }
        out << '|' << std::endl;
      }
      out << '+';
      for (size_t j=0;j<buffer._cols;++j)
        out << '-';
      out << '+';
      out << std::endl;
      return out;
    }

    /// Copy a buffer to an other buffer.
    /// @param buffer Buffer source.
    /// @return The new buffer created from the input.
    Buffer<T>& operator = (const Buffer<T>& buffer);

    /// Move a buffer to an other buffer.
    /// @param buffer Buffer source.
    /// @return The new buffer which is the input buffer.
    Buffer<T>& operator = (Buffer<T>&& buffer);

    /// Set all non zero byte from buffer to this.
    /// @param buffer Buffer with data.
    Buffer<T>& operator += (const Buffer<T>& buffer);

    /// Get all different bytes fro buffer.
    /// @param buffer Buffer with data.
    Buffer<T>& operator -= (const Buffer<T>& buffer);

    /// Copy from an other buffer starting at a give (x,y) point.
    /// @param buffer Contains the data to copy.
    /// @param bStartY Starting row.
    /// @param bStartX Starting col.
    void copy(const Buffer<T>& buffer, const ssize_t bStartY, const ssize_t bStartX);

    /// Get direct access to the buffer
    /// @return the pointer to the buffer.
    const T* getPtr() {
      return _buffer;
    }

};

#include "graphism/buffer.hxx"

#endif // BUFFER_HPP

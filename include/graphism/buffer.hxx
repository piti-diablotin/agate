/**
 * @file include/buffer.hxx
 *
 * @brief Implementation of the template buffer class.
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of AbiOut
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
 * along with Chronox.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "graphism/buffer.hpp"
#include <algorithm>


//
template< class T >
Buffer<T>::Buffer() : 
  _buffer(nullptr), 
  _rows(0), 
  _cols(0), 
  _size(0),
  _type(0)
{
}


//
template< class T >
Buffer<T>::Buffer(const size_t rows, const size_t cols) :
  _buffer(nullptr),
  _rows(rows),
  _cols(cols),
  _size(rows*cols),
  _type(0)
{
  if ( _size > 0 ) {
    _buffer = new T[_size];
    if ( _buffer == nullptr ) {
      _size = 0;
      throw EXCEPTION("Failed to allocate buffer.",ERRMEM);
    }
    for ( size_t i = 0 ; i < _size ; ++i )
      _buffer[i]=0;
  }
}


//
template< class T >
Buffer<T>::Buffer(const Buffer<T>& buffer) :
  _buffer(nullptr),
  _rows(buffer._rows),
  _cols(buffer._cols),
  _size(buffer._rows*buffer._cols),
  _type(buffer._type)
{
  if ( _size > 0 ) {
    _buffer = new T[_size];
    if ( _buffer == nullptr ) {
      _size = 0;
      throw EXCEPTION("Failed to allocate buffer.",ERRMEM);
    }
    std::copy(buffer._buffer, buffer._buffer+(_rows*_cols), _buffer);
  }
  std::clog << "Buffer copied (constructor)" << std::endl;
}


//
template< class T >
Buffer<T>::Buffer(Buffer<T>&& buffer) :
  _buffer(buffer._buffer),
  _rows(buffer._rows),
  _cols(buffer._cols),
  _size(buffer._size),
  _type(buffer._type)
{
  buffer._buffer = nullptr;
  buffer._size = 0;
  buffer._rows = 0;
  buffer._cols = 0;
  std::clog << "Buffer moved (constructor)" << std::endl;
}

//
template< class T >
Buffer<T>::~Buffer() {
  this->free();
}


//
template< class T >
void Buffer<T>::free(){
  if ( _buffer != nullptr ) {
    delete[] _buffer;
    _buffer = nullptr;
    _size = 0;
  }
  _rows = 0;
  _cols = 0;
}


//
template< class T >
void Buffer<T>::setSize(const size_t rows, const size_t cols) {
  _rows = rows;
  _cols = cols;
  if ( rows * cols > _size ) {
    _size = rows*cols;
    delete[] _buffer;
    _buffer = nullptr;
    _buffer = new T[_size];
    std::clog << "Buffer needs to be reallocated in " << __func__ << std::endl;
    if ( _buffer == nullptr ) {
      _size = 0;
      throw EXCEPTION("Failed to allocate buffer.",ERRMEM);
    }
  }
  this->clear();
}


// 
/*
   template< class T >
   std::ostream& operator << (std::ostream& out, const Buffer<T>& buffer) {
   for ( size_t i=0;i<buffer._rows;i++ ){
   for (size_t j=0;j<buffer._cols;j++)
   out << buffer._buffer[buffer.elt(i,j)];
//out << (buffer._buffer[buffer.elt(i,j)] == 0 ? " ": buffer._buffer[buffer.elt(i,j)] > 128 ? "*" : "+");
out << std::endl;
}
return out;
}
//*/


//
template< class T >
Buffer<T>& Buffer<T>::operator = (const Buffer<T>& buffer) {
  _rows = buffer._rows;
  _cols = buffer._cols;
  _type = buffer._type;
  //std::clog << "Before : size=" << _size << std::endl;
  //std::clog << "Need   : size=" << _rows*_cols << std::endl;
  if ( (_rows*_cols) > _size ) {
    std::clog << "Buffer needs to be reallocated in " << __func__ << std::endl;
    _size = _rows*_cols;
    delete[] _buffer;
    _buffer = nullptr;
    _buffer = new T[_size];
    if ( _buffer == nullptr ) {
      _size = 0;
      throw EXCEPTION("Failed to allocate buffer.",ERRMEM);
    }
  }
  if ( _rows*_cols > 0 ) {
    std::copy(buffer._buffer, buffer._buffer+(_rows*_cols), _buffer);
    if ( _buffer == nullptr ) {
      _size = 0;
      throw EXCEPTION("Failed to copy buffer.",ERRMEM);
    }
  }
  //std::clog << "Buffer copied (assignement)" << std::endl;
  return *this;
}


//
template< class T >
Buffer<T>& Buffer<T>::operator = (Buffer<T>&& buffer) {
  _rows = buffer._rows;
  _cols = buffer._cols;
  _size = buffer._size;
  _buffer = buffer._buffer;
  _type = buffer._type;
  buffer._size = 0;
  buffer._rows = 0;
  buffer._cols = 0;
  buffer._type = 0;
  buffer._buffer = nullptr;
  std::clog << "Buffer moved (assignement)" << std::endl;
  return *this;
}


//
template< class T >
Buffer<T>& Buffer<T>::operator += (const Buffer<T>& buffer) {
  if ( buffer._rows != _rows || buffer._cols != _cols )
    throw EXCEPTION("Dimensions mismastch.",ERRDIV);

  for ( size_t i = 0 ; i < _rows*_cols ; ++i )
    _buffer[i] =  ( buffer._buffer[i] == 0 ) ? _buffer[i] : buffer._buffer[i];

  return *this;
}


//
template< class T >
Buffer<T>& Buffer<T>::operator -= (const Buffer<T>& buffer) {
  if ( buffer._rows != _rows || buffer._cols != _cols ) {
    std::ostringstream str_tmp;
    str_tmp << "Dimensions mismastch : (this) " << _cols << "x" << _rows
      << " and (other)" << buffer._cols << "x" << buffer._rows << ".";
    throw EXCEPTION(str_tmp.str(),ERRDIV);
  }

  for ( size_t i = 0 ; i < _rows*_cols ; ++i )
    _buffer[i] =  ( buffer._buffer[i] == _buffer[i] ) ? 0 : _buffer[i];

  return *this;
}


//
template< class T >
void Buffer<T>::copy(const Buffer<T>& buffer, const ssize_t bStartY, const ssize_t bStartX) {

  size_t me_start_x = MIN((MAX(bStartX,0)),(ssize_t)_cols);
  ssize_t start_x = MAX(0,-bStartX);
  size_t me_start_y = MIN(MAX(bStartY,0),(ssize_t)_rows);
  ssize_t start_y = MAX(0,-bStartY);

  for( size_t i = start_y, me_i = me_start_y ; (i<buffer._rows) && (me_i<_rows) ; ++i, ++me_i) {
    for( size_t j = start_x, me_j = me_start_x ; (j<buffer._cols) && (me_j<_cols) ; ++j, ++me_j) {
      const unsigned char pix = buffer._buffer[i*buffer._cols+j];
      if ( pix != 0 )
        _buffer[me_i*_cols+me_j] = pix;
    }
  }
}

/**
 * @file exception.cpp
 *
 * @brief Implementation of the Exception class.
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

#include "base/exception.hpp"
#include <sstream>
#include <iostream>
#ifdef HAVE_CPPTHREAD
#include <thread>
#endif

// Declare verbosity mode
int Exception::_verbose=VDEBUG;


//
Exception::Exception(const std::string file, const std::string func, int line, const std::string message, int value) _NOEXCEPT :
  _file(1,file), 
  _line(1), 
  _func(1,func), 
  _text(1,message), 
  _activeStack(0),
  _returnValue(value)
{
  // Convert int to string
  std::ostringstream oss;
  oss << line;
  _line[0] = oss.str();
}


//
Exception::Exception(const std::string func, const std::string message, int value) _NOEXCEPT :
  _file(1,"Unknown"), 
  _line(1,"-1"), 
  _func(1,func), 
  _text(1,message), 
  _activeStack(0),
  _returnValue(value)
{
  switch (Exception::_verbose) {
    case 2 :
      _line[0] = func;
      break;
    case 3 :
      _func[0] = func;
      break;
  }
}


//
Exception::Exception(const std::string message, int value) _NOEXCEPT :
  _file(1,"Unknown"), 
  _line(1,"-1"), 
  _func(1,"Unknown"), 
  _text(1,message), 
  _activeStack(0),
  _returnValue(value)
{
}


//
Exception::Exception() _NOEXCEPT :
  _file(), 
  _line(), 
  _func(), 
  _text(), 
  _activeStack(-1),
  _returnValue(0)
{
}


//
Exception::Exception(const Exception& e) _NOEXCEPT :
  _file(e._file),
  _line(e._line),
  _func(e._func),
  _text(e._text),
  _activeStack(0),
  _returnValue(e._returnValue)
{
}


Exception::~Exception(){
}


//
void Exception::add(const std::string file, const std::string func, int line, const std::string message, int value) _NOEXCEPT {
  // convert int to string
  std::ostringstream oss;
  oss << line;

  _file.push_back(file);
  _line.push_back(oss.str());
  _func.push_back(func);
  _text.push_back(message);
  _activeStack = _text.size()-1;
  _returnValue = value;
}


//
void Exception::add(const std::string func, const std::string message, int value) _NOEXCEPT {
  _file.push_back("Unknown");
  _line.push_back("-1");
  _func.push_back("Unknown");
  switch (Exception::_verbose) {
    case 2 :
      _line.back() = func;
      break;
    case 3 :
      _func.back() = func;
      break;
  }
  _text.push_back(message);
  _activeStack = _text.size()-1;
  _returnValue = value;
}


//
void Exception::add(const std::string message, int value) _NOEXCEPT {
  _file.push_back("Unknown");
  _line.push_back("-1");
  _func.push_back("Unknown");
  _text.push_back(message);
  _activeStack = _text.size()-1;
  _returnValue = value;
}


//
std::string Exception::what(std::string prefix, bool textonly) const _NOEXCEPT {
  std::string buffer;
  std::ostringstream oss;

  if ( textonly ) return _text[_text.size()-1];

#if defined(HAVE_CPPTHREAD) && defined(DEBUG)
  std::thread::id my_id = std::this_thread::get_id();
  if ( my_id != std::thread::id() )
    oss << "== " << my_id << " == " ;
#endif

  // convert int to string
  oss << "[" << _activeStack+1 << "] ";
  //buffer = "[" + oss.str() + "] ";
  buffer = oss.str();


  switch (Exception::_verbose) {
    case 0 :
      buffer += "In file " + _file[_activeStack] + " : function " + _func[_activeStack] + " l. " + _line[_activeStack] + ": ";
      break;
    case 1 :
      buffer += "In function " + _func[_activeStack] + " l. " + _line[_activeStack] + ": ";
      break;
    case 2 :
      buffer += "Line " + _line[_activeStack] + ": ";
      break;
    case 3 :
      buffer += "In function " + _func[_activeStack] + ": ";
      break;
  }

  buffer += _text[_activeStack];
  if ( *buffer.rbegin() != '.' ) buffer += ".";

  std::string rbuffer;
  size_t begin = 0;
  size_t end = std::string::npos;
  try {
    do {
      end = buffer.find_first_of('\n',begin);
      if ( end == std::string::npos ) end = buffer.size();
      size_t end2 = end;
      do {
        end2 = (end-begin)<79-prefix.size()?end:begin+(79-prefix.size());
        size_t goodcut = buffer.substr(begin,end2-begin).find_last_of(" ");
        goodcut = ( goodcut == 0 || goodcut == std::string::npos ) ? end2-begin : goodcut;
        size_t end3 = begin+goodcut;
        end2 = ( end3 == std::string::npos || end2 == end ? end2 : end3+1 );
        rbuffer += (prefix+buffer.substr(begin,end2-begin)+"\n");
        begin = end2;
      } while (begin < end );
      begin = end+1;
    } while ( begin < buffer.size() && end != std::string::npos );
    rbuffer.erase(rbuffer.end()-1);
  }
  catch ( ... ) {
    std::cerr << "bad" << std::endl;
  }

  /*
     size_t buf_size = buffer.size();
     size_t newline = _text[_activeStack].find_first_of("\n");
     if ( newline == std::string::npos ) newline = _text[_activeStack].size();
     buffer += _text[_activeStack].substr(0,newline);
     size_t begin = ++newline;

     try {
     while ( (newline = _text[_activeStack].substr(begin).find_first_of("\n")) != std::string::npos ) {
     buffer += "\n|";
     buffer.insert(buffer.end(),buf_size-1,' ');
     buffer += _text[_activeStack].substr(begin,newline);
     begin += ++newline;
     }
     }
     catch ( ... ) {
     }

     if ( begin < _text[_activeStack].size() ) {
     buffer += "\n|";
     buffer.insert(buffer.end(),buf_size-1,' ');
     buffer += _text[_activeStack].substr(begin,_text[_activeStack].size()-begin);
     }
     */

  return rbuffer;
}


//
std::string Exception::fullWhat() _NOEXCEPT {
  std::string buffer;
  std::string prefix;

  if ( _returnValue == ERRWAR ) {
    buffer = "\n+--| Warning |\n";
    prefix = "| ";
  }
  else if ( _returnValue == ERRDIV || _returnValue == ERRABT ) {
    buffer = "\n+--| Error |\n";
    prefix = "| ";
  }
  else if ( _returnValue == ERRCOM ) {
    buffer = "\n+--| Comment |\n";
    prefix = "| ";
  }

  for ( unsigned it = 0 ; it < _text.size() ; ++it ) {
    _activeStack = it;
    buffer += this->what(prefix) + "\n";
  }

  if ( _returnValue == ERRWAR || _returnValue == ERRABT || _returnValue == ERRDIV  || _returnValue == ERRCOM ) {
    buffer += "+--| End |\n";
  }
  return buffer;
}


//
int Exception::getReturnValue() const _NOEXCEPT {
  return _returnValue;
}


//
void Exception::setVerbosity(int verbose) _NOEXCEPT {
  _verbose = verbose;
}


//
Exception& Exception::operator += (Exception& e) {
  _file.insert(_file.end(),e._file.begin(),e._file.end());
  _line.insert(_line.end(),e._line.begin(),e._line.end());
  _func.insert(_func.end(),e._func.begin(),e._func.end());
  _text.insert(_text.end(),e._text.begin(),e._text.end());
  _activeStack = _text.size()-1;
  _returnValue = e._returnValue;
  return *this;
}

/**
 * @file configparser.hxx
 *
 * @brief Implementation of the template methods for ConfigParser
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


#include "io/configparser.hpp"
#include "base/exception.hpp"
#include "base/typename.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <utility>
#include <type_traits>
#include "base/utils.hpp"
#include "base/phys.hpp"
#include <regex>

template<typename T>
T ConfigParser::parseNumber(std::string& str) {
  T numerator;
  T denominator;
  auto convertNumber = [](std::string snum) {
    using namespace std::regex_constants;
    std::smatch m;
    if ( std::regex_search ( snum, m, std::regex("sqrt\\((.*)\\)") ) ) {
      std::istringstream inside(m[1]);
      T value;
      inside >> value;
      if ( inside.fail() ) throw EXCEPTION("",1);
      return static_cast<T>(std::sqrt(value));
    }
    else {
      std::istringstream basic(snum);
      T value;
      basic >> value;
      if ( basic.fail() ) throw EXCEPTION("",1);
      return  value;
    }
  };

  if ( std::is_arithmetic<T>::value ) {
    std::vector<std::string> divide = utils::explode(str,'/');
    if ( divide.size() == 0 ) {
      return static_cast<T>(0);
    }
    else if ( divide.size() == 1 ) { // No / found
      numerator = convertNumber(divide[0]);
      denominator = static_cast<T>(1);
    }
    else if ( divide.size() == 2 ) { // We have on / found
      numerator = convertNumber(divide[0]);
      denominator = convertNumber(divide[1]);
    }
    return numerator/denominator;
  }
  else {
    return convertNumber(str);
  }
}

//
template<typename T>
std::vector<T> ConfigParser::getToken(const std::string& token, const unsigned size, Characteristic dim) const {
  const std::string &content = ( _caseSensitive ? _contentOrig : _content );
  size_t pos = _content.find(" "+token+" "); // Add space for safety (ntypat != typat)
  if ( pos == std::string::npos ) {
    std::string str_tmp = "Input token \"" + token + "\" could not be found";
    throw EXCEPTION(str_tmp,ConfigParser::ERFOUND);
  }

  std::istringstream str_data;
  str_data.str(std::move(content.substr(pos)));
  std::string readToken;
  str_data >> readToken;
  std::vector<T> rvector(size);
  for ( unsigned i = 0 ; i < size ; ++i ) {
    T value;
    std::string str;
    str_data >> str;
    // Try to convert D exp format to E format
    {
      size_t pos(str.find_first_of("Dd"));
      if (pos != std::string::npos) str[pos] = 'e';
    }
    try {
      std::vector<std::string> multiple = utils::explode(str,'*');
      if ( multiple.size() == 0) { // No multiple
        throw EXCEPTION("",1);
      }
      else if ( multiple.size() == 1) { // No multiple
        value = ConfigParser::parseNumber<T>(multiple[0]);
      }
      else if ( multiple.size() == 2 ) { // We have a multiplication
        unsigned factor;
        try{
          factor = ConfigParser::parseNumber<unsigned>(multiple[0]);
          if ( factor == 0 ) factor = size;
        }
        catch ( Exception &e ) {
          if ( e.getReturnValue() == 1 )
            factor = size;
          else throw e;
        }
        value = ConfigParser::parseNumber<T>(multiple[1]);
        for ( unsigned time = 1 ; time < factor && i < (size-1); ++time ) {
          // factor - 1 insertion since the last on is done at the end.
          rvector[i++]=value;
        }
      }
      else {
        throw EXCEPTION("",2);
      }
    }
    catch ( Exception& e ) {
      std::string str_err("Failed to read ");
      int err = ConfigParser::ERTYPE;
      if ( e.getReturnValue() == 1 ) {
        str_err +=  TypeName<T>::get() + " value for token \"" + token + "\"."; 
        if ( i > 0 ) {
          str_err += "\nCould only read " + utils::to_string(i) + " data instead of "
            + utils::to_string(size) + ".";  // long long type for icc to_string
          err |= ConfigParser::ERDIM;
        }
      }
      else if ( e.getReturnValue() == 2 ) {
        str_err +=  "expression for token \"" + token + "\"."; 
      }
      throw EXCEPTION(str_err,err);
    }
    rvector[i] = value;
    /*
       if ( str_data.eof() && i != (size-1)) {
       std::string str_err = "Could only read ";
       str_err += utils::to_string(i);
       str_err += "data for token " + token + " instead of " + utils::to_string(size) + ".";
       throw EXCEPTION(str_err,ConfigParser::ERDIM);
       }
       */
  }
  /* Try to read characteristic */
  if ( dim != NONE && std::is_arithmetic<T>::value ) {
    str_data >> readToken;
    if ( !str_data.fail() ) {
      double conversion = 1.;
      switch( dim ) {
        case LENGTH :
          if ( readToken == "angstrom" ) 
            conversion = 1./phys::b2A;
          break;
        case ENERGY :
          if ( readToken == "ev" ) 
            conversion = 1./phys::Ha2eV;
          break;
        case NONE :
          conversion = 1.;
          break;
      }
      for ( auto &value : rvector) value *= conversion;
    }
  }
  return rvector;
}


//
template<typename T>
T ConfigParser::getToken(const std::string& token, Characteristic dim) const {
  const std::string &content = ( _caseSensitive ? _contentOrig : _content );
  size_t pos = _content.find(" "+token+" ");
  if ( pos == std::string::npos ) {
    std::string str_tmp = "Input token \"" + token + "\" could not be found";
    throw EXCEPTION(str_tmp,ConfigParser::ERFOUND);
  }

  std::istringstream str_data;
  str_data.str(std::move(content.substr(pos)));
  std::string readToken;
  str_data >> readToken;
  str_data >> readToken;
  if ( str_data.fail() ) {
    std::string str_err("Failed to read ");
    str_err +=  TypeName<T>::get() + " value for token \"" + token + "\"."; 
    throw EXCEPTION(str_err,ConfigParser::ERTYPE);
  }
  T rvector = ConfigParser::parseNumber<T>(readToken);
  /* Try to read characteristic */
  if ( dim != NONE && std::is_arithmetic<T>::value ) {
    str_data >> readToken;
    if ( !str_data.fail() && !std::is_same<T,bool>::value ) {
      double conversion = 1.;
      switch( dim ) {
        case LENGTH :
          if ( readToken == "angstrom" ) 
            conversion = 1./phys::b2A;
          break;
        case ENERGY :
          if ( readToken == "ev" ) 
            conversion = 1./phys::Ha2eV;
          break;
        case NONE :
          conversion = 1.;
          break;
      }
      rvector *= (T) conversion;
    }
  }
  /*
     if ( str_data.eof() ) {
     std::string str_err = "Could not read value for token \"";
     str_err += token + "\".";
     throw EXCEPTION(str_err,ConfigParser::ERDIM);
     }
     */
  return rvector;
}


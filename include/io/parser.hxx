/**
 * @file parser.hxx
 *
 * @brief Template function to get an option
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

#include "io/parser.hpp"
#include <type_traits>
#include "base/typename.hpp"

//
template<typename T>
T Parser::getOption(std::string option) {
  static_assert((std::is_arithmetic<T>::value || std::is_base_of<std::string,T>::value),"Typename should arithmetic or a string only");
  for ( auto& testOpt : _options ) {
    if ( testOpt._name == option ) {
      if ( testOpt._hasArg == 1 ) {
        std::stringstream str_tmp;
        str_tmp << testOpt._value;
        T rval;
        str_tmp >> rval;
        if ( str_tmp.fail() ) {
          std::stringstream str_err;
          str_err << "Fail to read " << TypeName<T>::get() <<" value for option --" << option; 
          str_err << "/-" << testOpt._letter;
          throw EXCEPTION(str_err.str(),ERRDIV);
        }
        return rval;
      }
      else {
        std::ostringstream str_tmp;
        str_tmp << "Option " << option << " has no argument.";
        throw EXCEPTION(str_tmp.str(),ERRDIV);
      }
    }
  }
  std::ostringstream str_tmp;
  str_tmp << "Unknown option ";
  str_tmp << option;
  throw EXCEPTION(str_tmp.str(),ERRDIV);
}


/**
 * @file include/utils.hpp
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


#ifndef UTILS_HPP
#define UTILS_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <vector>
#include <list>
#include <string>
#ifndef HAVE_STOD
#include <cstdlib>
#endif
#include <stdexcept>
#include <numeric>
#include <cmath>
#include <regex>
#include <vector>
#include <string>
#include <sstream>
//#include <iomanip>
#include <utility>
#include <type_traits>
#include "base/exception.hpp"

//#define DEBUG_LOG(a) std::cerr << a << std::endl;

namespace utils {
  /**
   * Dump configuration and compilation information for debugging purpose
   * @param out stream to write to
   */
  void dumpConfig(std::ostream &out);

  /**
   * Display version number
   */
  void Version();

  /**
   * Get Agate version string like x.y.z
   */
  std::string agateVersion();

  /**
   * get spglib version is available
   * @return the string major.minor.micro
   */
  std::string spglibVersion();

  /**
   * Init FFTW3 if needed
   */
  void fftw3Init();

  /**
   * Free FFTW3 if needed
   */
  void fftw3Free();

  /**
   * Overload std::to_string in case it does not exist
   * and write the function otherwise.
   * @param num Integer value only.
   * @return A string with num
   */
  template<typename T>
  std::string to_string(T num);

  /**
   * Overload std::stoi in case it does not exist
   * and write the function otherwise.
   * @param str string containing a double
   * @return the converted integer number
   */
  inline int stoi(const std::string& str) {
#ifdef HAVE_STOI
    try {
      return std::stoi(str);
    }
    catch(const std::invalid_argument& ex) {
      throw EXCEPTION(ex.what(),ERRDIV);
    }
    catch(const std::out_of_range& ex) {
      throw EXCEPTION(ex.what(),ERRDIV);
    }
    catch(...) {
      throw EXCEPTION("Bad cast string -> int",ERRDIV);
    }
#else
    long int val = strtol(str.c_str(), nullptr, 10);
    if ( errno == ERANGE 
        || val > ((1<<(sizeof(int)*8)/2)-1) 
        || val < -((1<<(sizeof(int)*8)/2)-1) ) 
      throw std::out_of_range("Bad string number");
    return (int) val;
#endif
  }

  /**
   * Overload std::stod in case it does not exist
   * and write the function otherwise.
   * @param str string containing a double
   * @return the converted double number.
   */
  inline double stod(const std::string& str) {
    //First change D exp notation in E exp notation (Fortran to C++)
    std::string s(str);
    size_t pos(s.find_first_of("Dd"));
    if (pos != std::string::npos) s[pos] = 'e';
#ifdef HAVE_STOD
    return std::stod(s);
#else
    double val = strtod(s.c_str(), nullptr);
    if ( errno == ERANGE ) throw std::out_of_range("Bad string number");
    return val;
#endif
  }

  /**
   * The string is transformed to be in lower case only
   * @param str The string to modify.
   */
  void tolower(std::string& str);

  /**
   * The string is transformed to be in upper case only
   * @param str The string to modify.
   */
  void toupper(std::string& str);

  /**
   * The string is transformed to be in lower case only
   * @param str The string to modify.
   */
  std::string tolower(const std::string& str);

  /**
   * The string is transformed to be in upper case only
   * @param str The string to modify.
   */
  std::string toupper(const std::string& str);

  /**
   * Trim right a string by copy
   * @param str The string to trim
   * @param delimiters Character to ignore
   * @return The trimmed string
   */
  inline std::string rtrim(const std::string& str, const std::string& delimiters = " \f\n\r\t\v" ) {
    return str.substr( 0, str.find_last_not_of( delimiters ) + 1 );
  }

  /**
   * Trim left a string by copy
   * @param str The string to trim
   * @param delimiters Character to ignore
   * @return The trimmed string
   */
  inline std::string ltrim(const std::string& str, const std::string& delimiters = " \f\n\r\t\v" ) {
    return str.substr( str.find_first_not_of( delimiters ) );
  }

  /**
   * Trim left and right a string by copy
   * @param str The string to trim
   * @param delimiters Character to ignore
   * @return The trimmed string
   */
  inline std::string trim(const std::string& str, const std::string& delimiters = " \f\n\r\t\v" ) {
    return rtrim( ltrim( str, delimiters ), delimiters );
  }

  /**
   * Trim right a string by erasing
   * @param str The string to trim
   * @param delimiters Character to ignore
   */
  inline void rtrim(std::string& str, const std::string& delimiters = " \f\n\r\t\v" ) {
    str.erase( str.find_last_not_of( delimiters ) + 1 );
  }

  /**
   * Trim left a string by erasing
   * @param str The string to trim
   * @param delimiters Character to ignore
   */
  inline void ltrim(std::string& str, const std::string& delimiters = " \f\n\r\t\v" ) {
    str.erase( 0, str.find_first_not_of( delimiters ) );
  }

  /**
   * Trim left and right a string by erasing
   * @param str The string to trim
   * @param delimiters Character to ignore
   */
  inline void trim(std::string& str, const std::string& delimiters = " \f\n\r\t\v" ) {
    ltrim( str, delimiters );
    rtrim( str, delimiters );
  }

  /**
   * Split a string into a vector of string.
   * The split is done with the delim character.
   * @param str Input string to split.
   * @param delim Delimiter to perform the split.
   * @return A vector a string from the input string splitted.
   */
  std::vector<std::string> explode(const std::string& str, const char delim);

  //template<typename T>
  double mean(std::vector<double>::const_iterator first, std::vector<double>::const_iterator last);

  //template<typename T>
  double deviation(std::vector<double>::const_iterator first, std::vector<double>::const_iterator last, const double meanV);

  /**
   * Small function to print mean value of plot function
   */
  void sumUp(const std::list<std::vector<double>>& y, const std::list<std::string>& labels,std::ostream& sum, bool ordered=false);

  /**
   * Remove suffix of a file name
   * @param filenmae the file to treat
   * @return The filename without its extension if it has one.
   */
  std::string noSuffix(std::string filename);

  /**
   * Get the directory path of current filename
   * Equivalent to dirname in bash
   * @param filenmae the file to treat
   * @return The absolute path of the file 
   */
  std::string dirname(std::string filename);

  /**
   * Get the filename without the path
   * Equivalent to basename in bash
   * @param filenmae the file to treat
   * @return The name of the file without the path
   */
  std::string basename(std::string filename);

  /**
   * List all the files (directories included) in the current path.
   * The first element of the pair is the timestamp of the file (last modificiation)
   * The second is the filename
   * Files ares sorted. First is oldest, last is newest.
   * @pattern Regex expression to find files.
   * @return A vector containing a pair of timestamp (modification time) and file name
   */
  std::vector<std::pair<long int, std::string>> ls(std::string dir, std::string pattern=".*");

  template<typename T>
    T parseNumber(std::string& str) {
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
        else {
          throw EXCEPTION("Unable to parse number "+str,ERRDIV);
          numerator = 0;
          denominator = 0;
        }
        return numerator/denominator;
      }
      else {
        return convertNumber(str);
      }
    }

  std::string base64_encode(const std::string &in);
  std::string base64_decode(const std::string &in);

  std::istream& getline(std::istream& cin,std::string& line, unsigned int& counter, std::string comment="#");

  /**
   * Read a string that might be divided with space represented as '\ '
   * @param stream
   * @return the full string
   */
  std::string readString(std::istream& stream);
}

#endif  // UTILS_HPP

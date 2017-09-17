/**
 * @file src/utils.cpp
 *
 * @brief 
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


#include "base/utils.hpp"
#include <sstream>
#include <utility>
#include <locale>
#include <iomanip>
#include "base/exception.hpp"

namespace utils {

  //
  void dumpConfig(std::ostream &out) {
    out << "-----------------------" << std::endl;
    out << "   Build information   " << std::endl;
    out << "-----------------------" << std::endl;
    out <<  "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    out << std::endl;
    out << " - Architectures -" << std::endl;
    out << " ~~~~~~~~~~~~~~~~~~~~" << std::endl;
    out << " * Host....: " << COMPILE_HOST << std::endl;
    out << " * Build...: " << COMPILE_BUILD << std::endl;
    out << std::endl;

    out << " - Compiler options -" << std::endl;
    out << " ~~~~~~~~~~~~~~~~~~~~" << std::endl;
    out << " * CPPFLAGS.:  " << COMPILE_CPPFLAGS << std::endl;
    out << " * CXXFLAGS.:  " << COMPILE_CXXFLAGS << std::endl;
    out << " * LIBS.....:  " << COMPILE_LIBS << std::endl;
    out << " * LDFLAGS..:  " << COMPILE_LDFLAGS << std::endl;
#ifdef HAVE_CPPTHREAD
    out << " * pthread..:  " << "yes" << std::endl;
#else
    out << " * pthread..:  " << "no" << std::endl;
#endif

#ifdef HAVE_OMP
    out << " * OpenMP...:  " << "yes" << std::endl;
#else
    out << " * OpenMP...:  " << "no" << std::endl;
#endif
    out << std::endl;

    out << " - Libraries (Tools) -" << std::endl;
    out << " ~~~~~~~~~~~~~~~~~~~~~" << std::endl;
#ifdef HAVE_CURL
    out << " * cURL.....: yes" << std::endl;
#else
    out << " * cURL.....: no" << std::endl;
#endif
#ifdef HAVE_GNUPLOT
    out << " * gnuplot..: " << GNUPLOT_BIN << std::endl;
#else
    out << " * gnuplot..: no" << std::endl;
#endif

#ifdef HAVE_NETCDF
    out << " * NetCDF...: yes" << std::endl;
#  ifdef HAVE_NETCDFCXX4
    out << "              (C++4)" << std::endl;
#  elif defined(HAVE_NETCDFCPP)
    out << "              (C++)" << std::endl;
#  endif
#else
    out << " * NetCDF...: no" << std::endl;
#endif

#ifdef HAVE_LIBXML2
    out << " * XML2.....: yes" << std::endl;
#else
    out << " * XML2.....: no" << std::endl;
#endif

#ifdef HAVE_FREETYPE
    out << " * FreeType.: yes" << std::endl;
    out << "              (Font to use: "<< DEFAULT_FONT << ")" << std::endl;
#else
    out << " * FreeType.: no" << std::endl;
#endif
#ifdef HAVE_EIGEN
    out << " * Eigen....: yes" << std::endl;
#else
    out << " * Eigen....: no" << std::endl;
#endif
#ifdef HAVE_SPGLIB
    out << " * Spglib...: yes" << std::endl;
#else
    out << " * Spglib...: no" << std::endl;
#endif
#ifdef HAVE_YAMLCPP
    out << " * Yaml-cpp.: yes" << std::endl;
#else
    out << " * Yaml-cpp.: no" << std::endl;
#endif
#ifdef HAVE_YAMLCPP
    out << " * FFTW3....: yes" << std::endl;
#else
    out << " * FFTW3....: no" << std::endl;
#endif
    out << std::endl;

    out << " - Libraries (Graphics) -" << std::endl;
    out << " ~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
#ifdef HAVE_GL
    out << " * OpenGL...: yes" << std::endl;
#else
    out << " * OpenGL...: no" << std::endl;
#endif

#ifdef HAVE_GLEXT
    out << " * GLEXT....: yes" << std::endl;
#else
    out << " * GLEXT....: no" << std::endl;
#endif

#ifdef HAVE_GLU
    out << " * GLU......: yes" << std::endl;
#else
    out << " * GLU......: no" << std::endl;
#endif

#if defined(HAVE_GLFW2) || defined(HAVE_GLFW3)
    out << " * GLFW.....: yes" << std::endl;
#  ifdef HAVE_GLFW3
    out << "              (version 3)" << std::endl;
#  else
      out << "              (version 2)" << std::endl;
#  endif
#else
      out << " * GLFW.....: no" << std::endl;
#endif

#if defined(HAVE_QT)
    out << " * Qt.......: yes" << std::endl;
    //out << "              (" << QT_VERSION_STR << ")" << std::endl;
#else
    out << " * Qt.......: no" << std::endl;
#endif

#ifdef HAVE_LIBJPEG
    out << " * LibJPEG..: yes" << std::endl;
#else
    out << " * LibJPEG..: no" << std::endl;
#endif

#ifdef HAVE_LIBPNG
    out << " * LibPNG...: yes" << std::endl;
#else
    out << " * LibPNG...: no" << std::endl;
#endif
    out << "-----------------------\n" << std::endl;
  }


  // 
  template<typename T>
    std::string to_string(T num) {
#ifdef HAVE_TO_STRING_DONT_USE
      return std::to_string(num);
#else
      std::stringstream convert;
      // Max precision if floating point.
      convert.precision(14);
      convert << num;
      return convert.str();
#endif
    }

  /**
   * Explicit definition for int
   */
  template std::string to_string(int num);
  /**
   * Explicit definition for long
   */
  template std::string to_string(long num);
#ifdef HAVE_LONG_LONG
  /**
   * Explicit definition for long long
   */
  template std::string to_string(long long num);
#endif
  /**
   * Explicit definition for unsigned
   */
  template std::string to_string(unsigned num);
  /**
   * Explicit definition for unsigned long
   */
  template std::string to_string(unsigned long num);
#ifdef HAVE_LONG_LONG
  /**
   * Explicit definition for unsigned long long
   */
  template std::string to_string(unsigned long long num);
#endif
  /**
   * Explicit definition for float
   */
  template std::string to_string(float num);
  /**
   * Explicit definition for double
   */
  template std::string to_string(double num);
#ifdef HAVE_LONG_DOUBLE
  /**
   * Explicit definition for long double
   */
  template std::string to_string(long double num);
#endif


  //
  void tolower(std::string& str) {
    std::locale loc;
    for ( auto& letter : str ) {
      letter = std::tolower(letter,loc);
    }
  }


  void toupper(std::string& str) {
    std::locale loc;
    for ( auto& letter : str ) {
      letter = std::toupper(letter,loc);
    }
  }


  //
  std::string tolower(const std::string& str) {
    std::locale loc;
    std::string rstr(str);
    for ( auto& letter : rstr ) {
      letter = std::tolower(letter,loc);
    }
    return rstr;
  }


  std::string toupper(const std::string& str) {
    std::locale loc;
    std::string rstr(str);
    for ( auto& letter : rstr ) {
      letter = std::toupper(letter,loc);
    }
    return rstr;
  }


  //
  std::vector<std::string> explode(const std::string& str, const char delim) {
    std::vector<std::string> result;
    std::istringstream iss(str);

    for (std::string token; std::getline(iss, token, delim); ) {
      result.push_back(std::move(token));
    }

    return result;
  }

  double mean(std::vector<double>::const_iterator first, const std::vector<double>::const_iterator last) {
#ifdef HAVE_ACCUMULATE
    return std::accumulate(first,last,(double)0.)/(double)std::distance(first,last);
#else
    double acc = 0;
    for ( auto it = first ; it != last ; ++it ) acc += *it;
    return acc/(double)std::distance(first,last);
#endif
  }

  double deviation(std::vector<double>::const_iterator first, std::vector<double>::const_iterator last, const double meanV) {
#ifdef HAVE_ACCUMULATE
    return std::sqrt(
        std::accumulate(
          first,last,(double)0.,
          [meanV](double x, double y){ return x+(y-meanV)*(y-meanV);}
          )
        /(double)std::distance(first,last)
        );
#else
    double acc = 0;
    for ( auto it = first ; it != last ; ++it ) acc += (*it-meanV)*(*it-meanV);
    return std::sqrt(acc/(double)std::distance(first,last));
#endif
  }

  //
  void sumUp(const std::list<std::vector<double>>& y, const std::list<std::string>& labels, std::ostream& sum) {
    sum.setf(std::ios::scientific,std::ios::floatfield);
    sum.precision(10);
    if ( labels.size() < 2 ) {
      sum << " Mean value: " ;
      sum.setf(std::ios::right,std::ios::adjustfield); 
      const double meanV = utils::mean(y.begin()->begin(),y.begin()->end());
      const double deviationV = utils::deviation(y.begin()->begin(),y.begin()->end(),meanV);
      sum << std::setw(19) << meanV << " +/- " << std::setw(17) << deviationV << std::endl;
    }
    else {
      sum << " Mean values: " ;
      auto label = labels.begin();
      auto vec = y.begin();
      sum << std::endl;
      sum.precision(10);
      for ( ; vec != y.end() ; ++label, ++vec ) {
        using namespace std;
        sum.setf(ios::left,ios::adjustfield); 
        sum << setw(24) << *label << ":"; 
        sum.setf(ios::right,ios::adjustfield); 
        const double meanV = utils::mean(vec->begin(),vec->end());
        const double deviationV = utils::deviation(vec->begin(),vec->end(),meanV);
        sum << setw(19) << meanV << " +/- " << setw(17) << deviationV << endl;
      }
    }
  }
}


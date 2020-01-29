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
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <cstring>
#include <regex>
#include <ctime>
#include <vector>
#include "base/exception.hpp"
#include <tuple>
#include <algorithm>

#if defined(HAVE_SPGLIB) && defined(HAVE_SPGLIB_VERSION)
#  ifdef __cplusplus
extern "C"{
#  endif
#  include "spglib/spglib.h"
#  ifdef __cplusplus
}
#  endif
#endif

#ifdef HAVE_FFTW3_THREADS
#include "fftw3.h"
#endif

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
#ifdef HAVE_FFTW3
    out << " * FFTW3....: yes" << std::endl;
#else
    out << " * FFTW3....: no" << std::endl;
#endif
#ifdef HAVE_SSH
    out << " * SSH(SFTP): yes" << std::endl;
#else
    out << " * SSH(SFTP): no" << std::endl;
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

//#if defined(HAVE_QT)
//    out << " * Qt.......: yes" << std::endl;
//    //out << "              (" << QT_VERSION_STR << ")" << std::endl;
//#else
//    out << " * Qt.......: no" << std::endl;
//#endif

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
  void sumUp(const std::list<std::vector<double>>& y, const std::list<std::string>& labels, std::ostream& sum, bool ordered) {
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
      std::vector<std::tuple<std::string,double,double>> toSort;
      sum << std::endl;
      sum.precision(10);
      for ( ; vec != y.end() ; ++label, ++vec ) {
        const double meanV = utils::mean(vec->begin(),vec->end());
        const double deviationV = utils::deviation(vec->begin(),vec->end(),meanV);
        toSort.push_back(std::make_tuple(*label,meanV,deviationV));
      }
      if ( ordered ) {
        std::sort(toSort.begin(),toSort.end(),[](const std::tuple<std::string,double,double> &t1, const std::tuple<std::string, double,double> &t2){
          return std::abs(std::get<1>(t1))<std::abs(std::get<1>(t2));
        });
      }
      for ( auto t = toSort.begin(); t != toSort.end() ; ++t ) {
        using namespace std;
        sum.setf(ios::left,ios::adjustfield);
        sum << setw(24) << std::get<0>(*t) << ":";
        sum.setf(ios::right,ios::adjustfield);
        sum << setw(19) << std::get<1>(*t) << " +/- " << setw(17) << std::get<2>(*t) << endl;
      }
    }
  }

  std::string noSuffix(std::string filename) {
    auto pos = filename.find_last_of(".");
    return ( pos != std::string::npos ) ? filename.substr(0,pos) : filename;
  }

#ifndef _WIN32
  std::vector<std::pair<long int, std::string>> ls(std::string dir, std::string pattern) {

    DIR *dp;
    dirent *d;

    std::regex search(pattern);
    std::vector<std::pair<long int, std::string>> files;

    if((dp = opendir(dir.c_str())) == NULL)
      throw EXCEPTION("Unable to open current directory", ERRABT);

    struct stat fordate;
    while((d = readdir(dp)) != NULL)
    {
      if(!strcmp(d->d_name,".") || !strcmp(d->d_name,".."))
        continue;
      if ( regex_match(std::string(d->d_name),search) ) {
        stat(d->d_name,&fordate);
        files.push_back(std::pair<long int,std::string>(fordate.st_mtime,d->d_name));
      }
    }
    std::sort(files.begin(),files.end(),[](std::pair<long int, std::string> it1,std::pair<long int,std::string> it2){return it1.first < it2.first;});
    return files;
  }
#endif

std::string base64_encode(const std::string &in) {

  std::string out;

  int val=0, valb=-6;
  for (unsigned char c : in) {
    val = (val<<8) + c;
    valb += 8;
    while (valb>=0) {
      out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val>>valb)&0x3F]);
      valb-=6;
    }
  }
  if (valb>-6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val<<8)>>(valb+8))&0x3F]);
  while (out.size()%4) out.push_back('=');
  return out;
}

std::string base64_decode(const std::string &in) {

  std::string out;

  std::vector<int> T(256,-1);
  for (int i=0; i<64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

  int val=0, valb=-8;
  for (unsigned char c : in) {
    if (T[c] == -1) break;
    val = (val<<6) + T[c];
    valb += 6;
    if (valb>=0) {
      out.push_back(char((val>>valb)&0xFF));
      valb-=8;
    }
  }
  return out;
}

void Version(){
  std::cout << PACKAGE_NAME << " version " << PACKAGE_VERSION << std::endl;
  utils::dumpConfig(std::clog);
#if defined(HAVE_SPGLIB) && defined(HAVE_SPGLIB_VERSION)
  std::clog << "Using spglib version " << spglibVersion() << std::endl;
#endif
}

std::string spglibVersion(){
  std::ostringstream out;
#if defined(HAVE_SPGLIB) && defined(HAVE_SPGLIB_VERSION)
  out << spg_get_major_version() << "." 
    << spg_get_minor_version() << "." 
    << spg_get_micro_version();
#endif
  return out.str();
}

void fftw3Init() {
#ifdef HAVE_FFTW3_THREADS
  fftw_init_threads();
#endif
}

void fftw3Free() {
#ifdef HAVE_FFTW3_THREADS
      fftw_cleanup_threads();
#endif

}

std::istream& getline(std::istream& cin,std::string& line, unsigned int& counter, std::string comment) {
  do {
    std::getline(cin,line);
    size_t pos_com = line.find_first_of(comment);
    if ( pos_com != std::string::npos ) {
      line.resize(pos_com);
    }
    utils::trim(line);
    counter++;
  } while (cin && line.empty());
  return cin;
}

std::string readString(std::istream& stream) {
  std::string fullString;
  stream >> fullString;
  while ( fullString.back() == '\\' && stream.good()) {
    std::string partialString;
    stream >> partialString;
    fullString.back() = ' ';
    fullString += partialString;
  }
  return fullString;
}

}


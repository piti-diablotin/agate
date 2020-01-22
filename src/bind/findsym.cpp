/**
 * @file findsym.cpp
 *
 * @brief Interface to the webpage to get symetry analysis
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


#include "bind/findsym.hpp"
#include "base/exception.hpp"
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#if defined(HAVE_UNISTD_H ) && !defined(HAVE_MINGW)
#include <unistd.h>  // getuid getgid
#endif
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include "base/utils.hpp"
#include "base/mendeleev.hpp"

using Agate::Mendeleev;

Findsym::Findsym() :
  _findsym(""),
  _isset(false),
  _mendeleev(false),
  _title("AbiOut2Findsym"),
  _tolerance(0.0005),
  _rprim(),
  _natom(0),
  _typat(),
  _xred(),
  _result(),
  _cif()
{
#ifdef HAVE_LINUX
  std::string datadir = DATADIR 
    + std::string("/")
    + std::string(PACKAGE_NAME) 
    + std::string("/FINDSYM/");

  char *pISODATA = nullptr;
  pISODATA = getenv("ISODATA");
  if ( pISODATA != nullptr ) datadir = pISODATA;
  std::clog << "Looking for findsym in " << datadir << std::endl;

  if ( datadir.at(datadir.size()-1) != '/' ) datadir.push_back('/');
  datadir += "findsym";

  try {
    struct stat buf;
    if ( stat(datadir.c_str(), &buf) != 0 ) 
      throw EXCEPTION("findsym executable can not be found",ERRWAR);
#ifdef HAVE_UNISTD_H
    unsigned uid = getuid();
    unsigned gid = getgid();
    if ( buf.st_uid == uid ) {
      if ( !(buf.st_mode & S_IRUSR && buf.st_mode & S_IXUSR) )
        throw EXCEPTION("Bad user permission for findsym",ERRWAR);
    }
    else if (  buf.st_gid == gid ){
      if ( !(buf.st_mode & S_IRGRP && buf.st_mode & S_IXGRP) )
        throw EXCEPTION("Bad group permission for findsym",ERRWAR);
    }
    else {
      if  ( !(buf.st_mode & S_IROTH && buf.st_mode & S_IXOTH) )
        throw EXCEPTION("Bad permission for findsym",ERRWAR);
    }
#endif

  }
  catch (Exception& e) {
    if ( e.getReturnValue() == ERRWAR ) {
      e.ADD("Executable tried: "+datadir,ERRWAR);
#ifdef HAVE_CURL
      std::cerr << e.fullWhat() << std::endl;
      datadir = "";
#else 
      throw e;
#endif
    }
    else {
      e.ADD("Error while constructing findsym", ERRDIV);
    }
  }
  _findsym = datadir;
#else
  std::clog << "Your are not running on Linux system.\n" 
    << "The executable of findsym only works on Linux systems." << std::endl;
#endif // LINUX
}


Findsym::~Findsym(){}


//
std::string Findsym::title() const {
  return _title;
}


//
double Findsym::tolerance() const {
  return _tolerance;
}


//
geometry::mat3d Findsym::rprim() const {
  return _rprim;
}


//
unsigned Findsym::natom() const {
  return _natom;
}


//
std::vector<int> Findsym::typat() const {
  return _typat;
}


//
std::vector<geometry::vec3d> Findsym::xred() const {
  return _xred;
}


//
const std::string& Findsym::cif() const {
  if ( !_isset )
    throw EXCEPTION("FINDSYM not executed before",ERRDIV);
  return _cif;
}


//
void Findsym::title(const std::string& title) {
  _title = title;
}


//
void Findsym::tolerance(double tol) {
  _tolerance = tol;
}


//
void Findsym::rprim(const geometry::mat3d& rprim) {
  if ( geometry::det(rprim) <= 0) {
    throw EXCEPTION("Determinant is negative whereas it should  be positive.\nChange the basis vector order.", ERRDIV);
  }
  _rprim = rprim;
}


//
void Findsym::natom(unsigned natom) {
  _natom = natom;
}


//
void Findsym::typat(const std::vector<int>& typat, bool meaningfull) {
  if ( typat.size() != _natom ) {
    std::string err_str = "Number of provided typat (" + utils::to_string(typat.size()) +
      ") is not the number of atoms (" + utils::to_string(_natom) + ")";
    throw EXCEPTION(err_str, ERRDIV);
  }
  _typat = typat;
  _mendeleev = meaningfull;
}


//
void Findsym::xred(const std::vector<geometry::vec3d>& xred) {
  if ( xred.size() != _natom ) {
    std::string err_str = "Number of provided xred (" + utils::to_string(xred.size()) +
      ") is not the number of atoms (" + utils::to_string(_natom) + ")";
    throw EXCEPTION(err_str, ERRDIV);
  }
  _xred = xred;
}


//
void Findsym::findsym() {
  bool have_findsym = (!_findsym.empty());
  bool try_website = !have_findsym;
#if HAVE_LINUX
  if ( have_findsym ) {
    setenv("ISODATA",_findsym.substr(0,_findsym.size()-7).c_str(),1);

    std::stringstream fsym_input;
    fsym_input << _title << std::endl;
    fsym_input.precision(14);
    fsym_input.setf(std::ios::scientific,std::ios::floatfield);
    fsym_input << _tolerance << std::endl;
    fsym_input << 1 << /*"\t\t We give the vectors"<<*/ std::endl;
    fsym_input << std::setw(23) << _rprim[geometry::mat3dind(1,1)] << " " ;
    fsym_input << std::setw(23) << _rprim[geometry::mat3dind(1,2)] << " ";
    fsym_input << std::setw(23) << _rprim[geometry::mat3dind(1,3)] << std::endl;
    fsym_input << std::setw(23) << _rprim[geometry::mat3dind(2,1)] << " ";
    fsym_input << std::setw(23) << _rprim[geometry::mat3dind(2,2)] << " ";
    fsym_input << std::setw(23) << _rprim[geometry::mat3dind(2,3)] << std::endl;
    fsym_input << std::setw(23) << _rprim[geometry::mat3dind(3,1)] << " ";
    fsym_input << std::setw(23) << _rprim[geometry::mat3dind(3,2)] << " ";
    fsym_input << std::setw(23) << _rprim[geometry::mat3dind(3,3)] << std::endl;
    fsym_input << 2 << /*"\t\t Not sure the meaning but it is the form of the vectors" <<*/ std::endl;
    fsym_input << "P" << std::endl;
    fsym_input << utils::to_string(_natom) << std::endl;
    for ( auto typat : _typat ) 
      fsym_input << std::setw(4) << (_mendeleev ? std::string(Mendeleev::name[typat]) : utils::to_string(typat));
    fsym_input << std::endl;
    for ( auto& coord : _xred )
      fsym_input << std::setw(23) << coord[0] << std::setw(23) << coord[1] << std::setw(23) << coord[2] << std::endl;
    fsym_input << "EOF" <<std::endl;

    std::string processcall = _findsym + " << EOF \n" + fsym_input.str();

    //std::clog << processcall << std::endl;

    FILE *fsym;
    fsym = popen(processcall.c_str(),"r");
    if ( fsym == nullptr ) 
      throw EXCEPTION("Unable to open pipe for fsym",ERRDIV);

    char buff[256];
    while ( fgets(buff,256,fsym) != nullptr ) {
      for ( unsigned i=0 ; i < strlen(buff) ;++i)
        _result << buff[i];
    }
    pclose(fsym);
    try_website = false;
    if ( _result.str().find("# CIF file") == std::string::npos ) {
      std::clog << "Execution of findsym failed." << std::endl;
      try_website = true;
    }
    _result.str(_result.str().substr(0,_result.str().size()-2)); 

  }
#endif
  if ( try_website ) {
#ifdef HAVE_CURL
    std::clog << "Trying the website of findsym...";
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    if ( curl ) {
      _result.str("");
      CURLcode res;
      char curl_error[CURL_ERROR_SIZE];
      curl_easy_setopt(curl,CURLOPT_URL, Findsym::findsym_url.c_str());
      curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,Findsym::writeback);
      curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void *) &_result);
      curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,curl_error);
      curl_easy_setopt(curl,CURLOPT_POST,1);
      std::string post="input=findsym&title="+_title
        + "&acclat="+ std::to_string((long double) _tolerance)
        + "&accpos="+ std::to_string((long double) _tolerance/10)
        + "&accmag="+ std::to_string((long double) _tolerance/10)
        + "&a=&b=&c="
        + "&alpha=&beta=&gamma="
        + "&vectors=" 
        + std::to_string((long double)_rprim[geometry::mat3dind(1,1)]) + " "
        + std::to_string((long double)_rprim[geometry::mat3dind(1,2)]) + " "
        + std::to_string((long double)_rprim[geometry::mat3dind(1,3)]) + "\r\n"
        + std::to_string((long double)_rprim[geometry::mat3dind(2,1)]) + " "
        + std::to_string((long double)_rprim[geometry::mat3dind(2,2)]) + " "
        + std::to_string((long double)_rprim[geometry::mat3dind(2,3)]) + "\r\n"
        + std::to_string((long double)_rprim[geometry::mat3dind(3,1)]) + " "
        + std::to_string((long double)_rprim[geometry::mat3dind(3,2)]) + " "
        + std::to_string((long double)_rprim[geometry::mat3dind(3,3)])
        + "&centering=P"
        //+ "&axesm=a(b)c"
        //+ "&cell=1"
        //+ "&axeso=abc"
        //+ "&origin=2"
        //+ "&axesh=h"
        + "&atoms=" + utils::to_string(_natom)
        + "&types=";
      for ( auto typat : _typat ) {
        post += _mendeleev ? std::string(Mendeleev::name[typat]) + " " :
          utils::to_string(typat) + " ";
      } 
      post += "&positions=";
      for ( auto& coord : _xred ) {
        post += std::to_string((long double) coord[0]) + " " + std::to_string((long double) coord[1]) + " "
          + std::to_string((long double) coord[2]) + "\r\n" ;
      }

      //std::clog << post << std::endl;
      curl_easy_setopt(curl,CURLOPT_POSTFIELDS,post.c_str());
      res = curl_easy_perform(curl);
      if ( res != CURLE_OK ) {
        std::string err_str = "An error occured during curl performance:\n" +
          std::string(curl_error);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        throw EXCEPTION(err_str,ERRDIV);
      }
      curl_easy_cleanup(curl);
      curl_global_cleanup();
      _result.str(_result.str().substr(_result.str().find("<pre>")+6));
      _result.str(_result.str().substr(0,_result.str().find("</pre>")-2));
      std::clog << "ok" << std::endl;
      //std::clog << _result.str() << std::endl;
    }
    else
      throw EXCEPTION("Failed to init curl",ERRDIV);
#else
    throw EXCEPTION("FINDSYM executable is not available.\nConsider compiling with libcurl support to use the website version",ERRDIV);
#endif
  }
  try {
    //std::clog << _result.str().substr(0,_result.str().find("# CIF file"));
    _cif = _result.str().substr(_result.str().find("# CIF file"));
    //_cif = _result.str();
  }
  catch (std::out_of_range& oor) {
    throw EXCEPTION(std::string("FINDSYM failed: ")+oor.what()+std::string("\nCheck your input file"),ERRDIV);
  }
  _isset = true;
}


//
size_t Findsym::writeback(char *ptr, size_t size, size_t nmemb, void *strstream){
  static_cast<std::stringstream*>(strstream)->write(ptr,size*nmemb);
  return size*nmemb;
}


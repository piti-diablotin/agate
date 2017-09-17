/**
 * @file src/histdataxyz.cpp
 *
 * @brief  Read a XYZ file and build the HistData structure.
 * No force are available
 * No stress
 * No primitive cell
 * xred and xcart are the same.
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


#include "hist/histdataxyz.hpp"
#include "base/mendeleev.hpp"
#include "base/phys.hpp"
#include "base/utils.hpp"
#include <fstream>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iostream>
#include <csignal>
#include <iomanip>

//
HistDataXYZ::HistDataXYZ() {
  ;
}

//
HistDataXYZ::~HistDataXYZ() {
#ifdef HAVE_CPPTHREAD
  if ( _thread.joinable() ) {
    _endThread = true;
    _thread.join();
  }
#endif
}

//
void HistDataXYZ::readFromFile(const std::string& filename) {
  std::ifstream file;
  try {
    //std::chrono::system_clock::time_point ts = std::chrono::system_clock::now();
    file.open(filename.c_str(),std::ios::in);
    if ( !file )
      throw EXCEPTION("Unable to open file "+filename,ERRDIV);

    file >> _natom;
    if ( file.fail() )
      throw EXCEPTION("Unable to read number of atoms", ERRDIV);
    file.ignore(1024,'\n');

    std::string line;
    std::getline(file,line); // This is a comment.
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      std::string specie;
      file >> specie;
      file.ignore(1024,'\n');
      unsigned z = mendeleev::znucl(specie);
      auto it = _znucl.end();
      if ( (it = std::find(_znucl.begin(),_znucl.end(),z)) != _znucl.end() ) {
        _typat.push_back(it-_znucl.begin()+1);
      }
      else {
        _znucl.push_back(z);
        _typat.push_back(_znucl.size());
      }
    }

    file.seekg(0,file.beg);
    int nline=0;
    while ( !file.eof() ) {
      file.ignore(1024,'\n');
      ++nline;
    }
    _ntime = nline/(_natom+2);
    file.close();
    _xyz = 3;

    // Allocate arrays
    _xcart  .resize(_ntime*_natom*_xyz);
    _xred   .resize(_ntime*_natom*_xyz);
    //_fcart  .resize(_ntime*_natom*_xyz);
    _acell  .resize(_ntime*_xyz);
    _rprimd .resize(_ntime*_xyz*_xyz);
    _etotal .resize(_ntime);
    _time   .resize(_ntime);
    _stress .resize(_ntime*6);

    _ntimeAvail = 0;
    _filename = filename;
#ifdef HAVE_CPPTHREAD
    _endThread = false;
    _thread = std::thread([=](){
#endif
        try {
          std::ifstream file(filename.c_str(),std::ios::in);
          for ( unsigned itime = 0 ; itime < _ntime ; ++itime ) {
#ifdef HAVE_CPPTHREAD
            if ( _endThread == true ) break;
#endif

            unsigned natom = 0;

            file >> natom;
            if ( natom != _natom ) {
              _ntime = _ntimeAvail;
              throw EXCEPTION("Only able to read XYZ file with same number of atoms for each snapshot",ERRABT);
            }
            file.ignore(1024,'\n'); // Terminate natom line
            file.ignore(1024,'\n'); // Comment line
            _acell[itime*3  ] = 1.0;
            _acell[itime*3+1] = 1.0;
            _acell[itime*3+2] = 1.0;
            _rprimd[itime*9  ] = 0.0;
            _rprimd[itime*9+1] = 0.0;
            _rprimd[itime*9+2] = 0.0;
            _rprimd[itime*9+3] = 0.0;
            _rprimd[itime*9+4] = 0.0;
            _rprimd[itime*9+5] = 0.0;
            _rprimd[itime*9+6] = 0.0;
            _rprimd[itime*9+7] = 0.0;
            _rprimd[itime*9+8] = 0.0;
            _time[itime] = static_cast<double>(itime)/phys::atu2fs;
            for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
              std::string specie;
              file >> specie 
                >> _xcart[itime*_natom*_xyz+iatom*_xyz  ]
                >> _xcart[itime*_natom*_xyz+iatom*_xyz+1]
                >> _xcart[itime*_natom*_xyz+iatom*_xyz+2];
              file.ignore(1024,'\n'); // Terminate natom line
              if ( specie != utils::trim(std::string(mendeleev::name[_znucl[_typat[iatom]-1]])) ) {
                _ntime = _ntimeAvail;
                throw EXCEPTION("Only able to read XYZ file with same type of atoms for each snapshot",ERRABT);
              }
              _xred[itime*_natom*_xyz+iatom*_xyz  ] = (_xcart[itime*_natom*_xyz+iatom*_xyz  ] /= phys::b2A);
              _xred[itime*_natom*_xyz+iatom*_xyz+1] = (_xcart[itime*_natom*_xyz+iatom*_xyz+1] /= phys::b2A);
              _xred[itime*_natom*_xyz+iatom*_xyz+2] = (_xcart[itime*_natom*_xyz+iatom*_xyz+2] /= phys::b2A);
              //_fcart[itime*_natom*_xyz+iatom*_xyz  ] = 0.0;
              //_fcart[itime*_natom*_xyz+iatom*_xyz+1] = 0.0;
              //_fcart[itime*_natom*_xyz+iatom*_xyz+2] = 0.0;
            }
            _ntimeAvail++;
          }
          //std::chrono::system_clock::time_point tf = std::chrono::system_clock::now();
          //std::clog << std::chrono::duration_cast< std::chrono::duration<double> >(tf-ts).count() << std::endl;

          file.close();
        } 
        catch (Exception &e) {
          std::cerr << e.fullWhat() << std::endl;
          if ( _ntimeAvail == 0 ) {
            _ntimeAvail = 1;
            _ntime = 0;
          }
        }
#ifdef HAVE_CPPTHREAD
    });
#endif
  }
  catch (Exception &e){
    e.ADD("Unable to build XYZ HistData",e.getReturnValue());
    throw e;
  }
}


//
void HistDataXYZ::dump(const std::string& filename, unsigned tbegin, unsigned tend) const {
  std::ofstream file;
  using namespace std;
  try {
    HistData::checkTimes(tbegin,tend);
    file.open(filename.c_str(),std::ios::out);
    if ( !file ) 
      throw EXCEPTION("Unable to creat file"+filename,ERRABT);

    file.precision(14);
    file.setf(std::ios::right,std::ios::adjustfield);
    file.setf(std::ios::scientific,std::ios::floatfield);
    for ( unsigned itime = tbegin ; itime < tend ; ++itime ) { 
      file << _natom << endl;
      file << itime << "/" << tend << " created by " << PACKAGE_STRING << " without any warranty" <<endl;

      for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
        file << mendeleev::name[_znucl[_typat[iatom]-1]];
        file 
          << std::setw(23) << _xcart[itime*_natom*_xyz+iatom*_xyz  ] * phys::b2A
          << std::setw(23) << _xcart[itime*_natom*_xyz+iatom*_xyz+1] * phys::b2A
          << std::setw(23) << _xcart[itime*_natom*_xyz+iatom*_xyz+2] * phys::b2A
          << std::endl;
      }
    }
  }
  catch ( Exception &e ) {
    e.ADD("Dumping failed",ERRABT);
    throw e;
  }
}

//
void HistDataXYZ::dump(const HistData &hist, const std::string& filename, unsigned tbegin, unsigned tend) {
  std::ofstream file;
  using namespace std;
  try {
    hist.checkTimes(tbegin,tend);
    unsigned natom = 0;
    auto _xcart = hist.getXcart(0,&natom);
    auto _znucl = hist.znucl();
    auto _typat = hist.typat();
    file.open(filename.c_str(),std::ios::out);
    if ( !file ) 
      throw EXCEPTION("Unable to creat file"+filename,ERRABT);

    file.precision(14);
    file.setf(std::ios::right,std::ios::adjustfield);
    file.setf(std::ios::scientific,std::ios::floatfield);
    for ( unsigned itime = tbegin ; itime < tend ; ++itime ) { 
      file << natom << endl;
      file << itime << "/" << tend << " created by " << PACKAGE_STRING << " without any warranty" <<endl;

      for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
        file << mendeleev::name[_znucl[_typat[iatom]-1]];
        file 
          << std::setw(23) << _xcart[itime*natom*3+iatom*3  ] * phys::b2A
          << std::setw(23) << _xcart[itime*natom*3+iatom*3+1] * phys::b2A
          << std::setw(23) << _xcart[itime*natom*3+iatom*3+2] * phys::b2A
          << std::endl;
      }
    }
  }
  catch ( Exception &e ) {
    e.ADD("Dumping failed",ERRABT);
    throw e;
  }
}

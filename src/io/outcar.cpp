/**
 * @file outcar.cpp
 *
 * @brief Implementation of the outcar class
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2020 Jordan Bieder
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


#include "io/outcar.hpp"
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <locale>
#include <cmath>
#include "base/phys.hpp"
#include "base/mendeleev.hpp"
#include "base/utils.hpp"
#include "base/geometry.hpp"

using Agate::Mendeleev;

//
Outcar::Outcar() : Dtset()
{
}

//
Outcar::~Outcar(){
}

//
void Outcar::readFromFile(const std::string& filename) {
  using geometry::mat3dind;
  std::ifstream outcar(filename,std::ios::in);  
  try {
    if ( !outcar)
      throw EXCEPTION("Error while opening file " + filename, ERRDIV);

    std::string line;
    unsigned int iline = 0;
    _znucl.clear();
    while ( utils::getline(outcar,line,iline) ) {
      size_t pos;
      if ( (pos=line.find("NION")) != std::string::npos ) {
        std::istringstream str(line.substr(pos+7));
        str >> _natom;
        _xred.resize(_natom);
      }
      else if ( (pos=line.find("ions per type")) != std::string::npos ) {
        std::istringstream str(line.substr(pos+15));
        _typat.clear();
        unsigned itypat = 0;
        while ( str ) {
          unsigned ntypat;
          str >> ntypat;
          if ( !str ) break;
          ++itypat;
          for ( unsigned i = 0 ; i < ntypat ; ++ i ) _typat.push_back(itypat);
        }
        _ntypat = itypat;
      }
      else if ( (pos=line.find("direct lattice vectors")) != std::string::npos ) {
        for ( unsigned i = 1 ; i < 4 ; ++i ) {
          utils::getline(outcar,line,iline);
          std::istringstream str(line);
          str >> _rprim[mat3dind(i,1)] >> _rprim[mat3dind(i,2)] >> _rprim[mat3dind(i,3)];
        }
        for ( auto& r : _rprim ) r *= phys::A2b;
      }
      else if ( (pos=line.find("position of ions in fractional coordinates")) != std::string::npos ) {
        for ( unsigned i = 0 ; i < _natom ; ++i ) {
          utils::getline(outcar,line,iline);
          std::istringstream str(line);
          str >> _xred[i][0] >> _xred[i][1] >> _xred[i][2];
        }
      }
      else if ( (pos=line.find("POMASS")) != std::string::npos ) {
        std::istringstream str(line.substr(pos+8));
        double mass;
        str >> mass;
        _znucl.push_back(Agate::Mendeleev::znucl(mass));
      }
    }
    outcar.close();
    _gprim = geometry::invertTranspose(_rprim);
    geometry::changeBasis(_rprim, _xcart, _xred, false);
    _znucl.pop_back(); // Remove last POMASS which is in OUTCAR POMASS = M1 M2 M3 ....
    if ( _znucl.size() != _ntypat ) {
      std::ostringstream str;
      str << "Found " << _ntypat << " types of atom but znucl size is " << _znucl.size();
      throw EXCEPTION(str.str(),ERRABT);
    }
  }
  catch( Exception& e ) {
    outcar.close();
    std::string err_str = "Failed to build Outcar from file "+filename;
    e.ADD(err_str,ERRDIV);
    throw e;
  }
}


//
void Outcar::dump(std::ostream& out) const {
  (void) out;
  throw EXCEPTION("It does not make sens to dump an OUTCAR !", ERRDIV);
}


/**
 * @file src/ddbabinit.cpp
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


#include "io/ddbabinit.hpp"
#include <fstream>
#include <string>
#include "base/utils.hpp"
#include "io/configparser.hpp"
#include "base/mendeleev.hpp"

//
DdbAbinit::DdbAbinit() : Ddb() {
  _haveMasses = false;
}

//
DdbAbinit::~DdbAbinit() {
  ;
}

void DdbAbinit::readFromFile(const std::string& filename) {
  {
    // Start building structure information
    ConfigParser parser(filename);
    try {
      this->readConfig(parser);
      // znucl is given as double so readConfig fails to read znucl as unsigned.
      std::vector<double> znucl(parser.getToken<double>("znucl",_ntypat));
      _znucl.resize(_ntypat);
      for( unsigned z = 0 ; z < _ntypat ; ++z ) {
        _znucl[z] = static_cast<unsigned>(znucl[z]);
        if ( _znucl[z] > 118.0 || _znucl[z] == 0 ) { //max znucl allowed
          std::string err_str = "Found value " + utils::to_string(_znucl[z]) 
            + " for token znucl\nwhich should be smaller than 118 ";
          throw EXCEPTION(err_str,ERRDIV);
        }
      }
    }
    catch( Exception& e ) {
      e.ADD(std::string("Failed to read header from DDB file ")+filename,ERRDIV);
      throw e;
    }
   // zion is given as double so readConfig fails to read zion as unsigned.
    std::vector<double> zion(parser.getToken<double>("zion",_ntypat));
    _zion.resize(_typat.size());                                 // resize with typat.size() to account for cases with several atoms of same type	
    for( unsigned x = 0 ; x < _typat.size() ; ++x ) {
        _zion[x] = static_cast<unsigned>(zion[_typat[x]-1]);     // write values into _zion
    } 	
 	
    try{
      std::vector<double> amu(parser.getToken<double>("amu",_ntypat));
      for ( unsigned ityp = 0 ; ityp < _ntypat ; ++ityp ) {
        mendeleev::mass[_znucl[ityp]] = amu[ityp];
      }
    }
    catch ( Exception& e ) {
      e.ADD("This is not a DDB file",ERRABT);
      std::clog << e.fullWhat() << std::endl;
    }
  }

  std::ifstream idd(filename,std::ios::in);
  if ( !idd ) 
    throw EXCEPTION(std::string("Failed to open DDB file ")+filename,ERRDIV);

  // Look for beginning of DDB information : Number of blocks
  unsigned nblock;
  {
    std::string tmp_line;
    do {
      std::getline(idd,tmp_line);
      if ( idd.fail() || idd.eof() )
        throw EXCEPTION(std::string("Bad DDB format in file ")+filename+". Could not find \"**** Database of total energy derivatives ****\"",ERRDIV);
   } while ( tmp_line.compare(" **** Database of total energy derivatives ****") != 0 );

    // Next line is the number of block
    std::getline(idd,tmp_line,'=');
    if ( idd.fail() || idd.eof() )
      throw EXCEPTION(std::string("Bad DDB format in file ")+filename,ERRDIV);
    idd >> nblock;
    std::getline(idd,tmp_line); // Empty line
    if ( idd.fail() || idd.eof() )
      throw EXCEPTION(std::string("Bad DDB format in file ")+filename,ERRDIV);
  }

  std::clog << "About to read a maximum of " << nblock << " data blocks." << std::endl;

  try {
    _nqpt = 0;
    for ( unsigned iqpt = 0 ; iqpt < nblock ; ++iqpt ) {
      if ( this->readBlock(idd) ) ++_nqpt;
    }
  }
  catch (Exception& e) {
    e.ADD(std::string("Unable to correctly read the DDB file")+filename,ERRDIV);
    throw e;
  }
}


//
bool DdbAbinit::readBlock(std::ifstream& idd) {
  // We are just at the beginning of a block
  // First read the number of elements
  std::string line;
  unsigned nelts = 0;
  std::getline(idd, line, ':');

  idd >> nelts;
  if ( line.find("2nd derivative") == std::string::npos ) {
    for ( unsigned ign = 0 ; ign <= nelts ; ++ign ) {
      std::getline(idd, line);
    }
    return false;
  }

  // read qpt
  double qx, qy, qz, weight;
  idd >> line >> qx >> qy >> qz >> weight; 
  if ( idd.fail() || idd.eof() || idd.bad() )
    throw EXCEPTION(std::string("Cannot identify the Qpt"),ERRDIV);

  std::vector<d2der> block;
  for ( unsigned ielt = 0 ; ielt < nelts && !idd.eof() ; ++ielt ) {
    // Need to read string since cpp does not understand x.xxDyy
    unsigned idir1, ipert1, idir2, ipert2;
    std::string real, img;
    idd >> idir1 >> ipert1 >> idir2 >> ipert2 >> real >> img;
    block.push_back(
       std::make_pair(
         std::array<unsigned,4>{{ idir1-1, ipert1-1, idir2-1, ipert2-1 }} ,
         complex(utils::stod(real),utils::stod(img))
         )
       );
  }

  // Vector size is twice the real size ... I don't know why
  if ( block.size() != nelts ) 
    throw EXCEPTION("Not enought derivates to construct block",ERRDIV);
  if ( qx*qx+qy*qy+qz*qz < 1e-7 ) {
    std::clog << "Reading Gamma point. Setting imaginary part to 0." << std::endl;
    for ( unsigned ielt = 0 ; ielt < nelts && !idd.eof() ; ++ielt ) {
      block[ielt].second.imag(0.e0);
    }
  }
  _blocks.insert(std::make_pair(
          geometry::vec3d({{ qx, qy, qz }}),
          block));

  std::getline(idd, line);
  return true;
}

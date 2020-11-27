/**
 * @file src/./ddboutcar.cpp
 *
 * @brief 
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


#include "io/ddboutcar.hpp"
#include "io/outcar.hpp"
#include "base/unitconverter.hpp"

//
DdbOutcar::DdbOutcar() {
  ;
}

//
DdbOutcar::~DdbOutcar() {
  ;
}

void DdbOutcar::readFromFile(const std::string& filename) {
  try {
    Outcar::readFromFile(filename);
  }
  catch( Exception &e ) {
    e.ADD("File does not seem to be OUTCAR",ERRDIV);
    throw e;
  }

  std::ifstream outcar(filename,std::ios::in);  
  try {
    if ( !outcar)
      throw EXCEPTION("Error while opening file " + filename, ERRDIV);

    std::string line;
    unsigned int iline = 0;
    UnitConverter eunit(UnitConverter::eV);
    UnitConverter dunit(UnitConverter::A);
    eunit = UnitConverter::Ha;
    dunit = UnitConverter::bohr;
    const double factor = (-1*eunit)/(1*dunit*dunit);
    while ( utils::getline(outcar,line,iline) ) {
      size_t pos;
      if ( (pos=line.find("SECOND DERIVATIVES")) != std::string::npos ) {
        utils::getline(outcar,line,iline); // line with ---------------
        utils::getline(outcar,line,iline); // line with 1X 2X 3X ...
        std::istringstream str(line);
        std::string coord;
        unsigned npert = 0;
        while (str) {
          str >> coord;
          ++npert;
        }
        --npert; //remove last pass which is "EOF"
        if ( _natom != (npert/3) )
          throw EXCEPTION("Missmatch between natom and number of perturbation read", ERRDIV);
        double val;
        std::string dummy;
        std::vector<d2der> block;
        geometry::vec3d qpt = {0,0,0}; // Gamma only
        for ( unsigned ipert1 = 0 ; ipert1 < _natom ; ++ipert1 ) {
          for ( unsigned idir1 = 0 ; idir1 < 3  ; ++idir1 ) {
            utils::getline(outcar,line,iline); 
            std::istringstream derive(line);
            derive >> dummy;
            for ( unsigned ipert2 = 0 ; ipert2 < _natom ; ++ipert2 ) {
              for ( unsigned idir2 = 0 ; idir2 < 3  ; ++idir2 ) {
                derive >> val;
                block.push_back(
                    std::make_pair(
                      std::array<unsigned,4>{{ idir1, ipert1, idir2, ipert2 }} , 
                      complex(val*factor,0)
                      )
                    );
              }
            }
          }
        }
        _blocks.insert(std::make_pair( qpt, block));
        break;
      }
    }
    if ( _blocks.size() < 1 ) 
      throw EXCEPTION("Could not find the second derivatives",ERRDIV);
    this->blocks2Reduced();
  }
  catch ( Exception &e ) {
    e.ADD("Failed to read DdbOutcar",ERRDIV);
    throw e;
  }
}

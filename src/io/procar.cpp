/**
 * @file src/io/procar.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2019 Jordan Bieder
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


#include "io/procar.hpp"
#include <fstream>
#include "base/exception.hpp"
#include "base/geometry.hpp"
#include "base/utils.hpp"
#include "io/poscar.hpp"
#include "io/outcar.hpp"

//
Procar::Procar() : EigParserElectrons() {
  ;
}

//
Procar::~Procar() {
  ;
}

void Procar::readFromFile(const std::string& filename) {
  unsigned kpts;
  unsigned nband;
  unsigned nblock = 0;
  unsigned nspin = 0;
  std::string tmp;
  double kx=0.0, ky=0.0, kz=0.0;
  double value=0.0;
  int natom = 0;
  _lmax = 3; // Seems that only up to d-composition is provided in PROCAR format
  _lmMask.resize(_lmax*_lmax,1);

  std::ifstream procar(filename.c_str(), std::ios::in);
  if ( !procar ) 
    throw EXCEPTION(std::string("Unable to read the file ")+filename,ERRABT);

  // First line is a comment -> ignore it
  std::getline(procar,tmp);

  for ( unsigned ispin = 0 ; ispin < 2 ; ++ispin ) {
    // Second line is number of k-points bands and ions;
    std::getline(procar,tmp,':');
    procar >> kpts;
    std::getline(procar,tmp,':');
    procar >> nband;
    std::getline(procar,tmp,':');
    procar >> natom;

    if ( procar.eof() ) break;
    if ( !procar )
      throw EXCEPTION("Cannot read properly the parameters",ERRDIV);

    if ( ispin == 1 && _nband != nband )
      throw EXCEPTION("Spins do not have the same number of band, weird ?!",ERRABT);
    _nband = nband;
    nspin++;
    geometry::vec3d prev_kpt = {{0.,0.,0.}};
    double length = 0.0;
    for ( unsigned ikpt = 0 ; ikpt < kpts ; ++ ikpt ) {
      // Empty line
      std::getline(procar,tmp);
      // Line with kpt and weight
      std::getline(procar,tmp,':');
      procar >> kx >> ky >> kz;
      _kpts.push_back({{kx,ky,kz}});
      //geometry::print(_kpts.back(),std::cerr);
      if ( procar.fail() )  {
        throw EXCEPTION(std::string("Cannot read coordinates for kpt ")
            +utils::to_string(ikpt+1),ERRDIV);
      }
      std::getline(procar,tmp);
      // Empty line
      std::getline(procar,tmp);

      // Testing for spinor detection
      auto pos = procar.tellg();
      std::getline(procar,tmp); // band line
      std::getline(procar,tmp); // empty
      nblock = 0;
      procar >> tmp; // First word
      if ( tmp == "ion" )
        std::getline(procar,tmp); // end line
      else throw EXCEPTION("Line should start with ion",ERRABT);
      for ( int s = 0 ; s < 4 ; ++s ) {
        for ( int iatom = 0 ; iatom < natom ; ++iatom )
          std::getline(procar,tmp); // Get projection
        procar >> tmp; // Should read tot
        if ( tmp != "tot" ) break;
        std::getline(procar,tmp); // Finish line
        nblock++;
      }
      procar.clear();
      procar.seekg(pos);


      std::vector<double> values;
      size_t nfraction = _lmax*_lmax*natom;
      std::vector<double> fraction(nblock*_nband*nfraction,0);
      try {
        for ( unsigned iband = 0 ; iband < nband ; ++iband ) {
          // Line with band index energy and occupations;
          procar >> tmp >> tmp >> tmp >> tmp;
          procar >>  value;
          //std::cerr << value << std::endl;
          values.push_back(value);
          std::getline(procar,tmp);
          if ( procar.fail() ) {
            throw EXCEPTION(std::string("Cannot read enough bands: ")
                +utils::to_string(iband-1)+std::string(" instead of ")
                +utils::to_string(nband),ERRDIV);
          }
          // Empty line
          std::getline(procar,tmp);
          // Ignore projections for the moment
          /*
          do {
            std::getline(procar,tmp);
            //std::cerr << " Ignoring " << tmp.size() << " :"<< tmp << std::endl;
          } while ( tmp.size() > 2 );
          */
          std::getline(procar,tmp); // first line with description
          for ( unsigned iblock = 0 ; iblock < nblock ; ++iblock ) {
            for ( int iatom = 0 ; iatom < natom ; ++iatom ) {
              int itest;
              procar >> itest;
              if ( itest-1 != iatom )
                throw EXCEPTION("Bad number of atom",ERRABT);
              for ( int l = 0 ; l < _lmax*_lmax ; ++l ) {
                procar >> fraction[(iblock*nfraction+iatom*_lmax*_lmax+l)*_nband+iband];
              }
              double tot;
              procar >> tot;
            }
            procar >> tmp;
            if ( tmp != "tot" ) throw EXCEPTION("Expecting tot",ERRABT);
            std::getline(procar,tmp); // Last line of the block with tot
          }
        }
      }
      catch(Exception& e) {
        e.ADD(std::string("Something went wrong reading kpt ")
            +utils::to_string(ikpt+1),ERRDIV);
        throw e;
      }
      _eigens.push_back(std::move(values));
      _fractions.push_back(std::move(fraction));
      if ( ikpt == 0 ) prev_kpt = {{kx,ky,kz}};
      length += geometry::norm(geometry::operator-({{kx,ky,kz}},prev_kpt));
      _lengths.push_back(length);
      prev_kpt = {{kx,ky,kz}};
      if ( procar.eof() ) break;
    }
    _hasSpin = (nspin == 2);
    _eunit.rebase(UnitConverter::eV);
  }

  Dtset *structure = nullptr;
  std::string structFile = utils::dirname(filename)+"/POSCAR";
  std::ifstream test(structFile);
  if ( ! test ) {
    structFile = utils::dirname(filename)+"/OUTCAR";
    std::ifstream test2(structFile);
    if ( !test2 ) {
      Exception e = EXCEPTION("Structure file is missing.\nProjection won't be available",ERRWAR);
      std::clog << e.fullWhat() << std::endl;
    }
    test2.close();
    structure = new Outcar;
  }
  else {
    test.close();
    structure = new Poscar;
  }

  try {
    structure->readFromFile(structFile);
    _dtset.reset(structure);
  }
  catch ( Exception &e ) {
    e.ADD("Projection won't be available",ERRWAR);
    std::clog << e.fullWhat() << std::endl;
  }
}

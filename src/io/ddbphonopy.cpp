/**
 * @file src/ddbphonopy.cpp
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


#include "io/ddbphonopy.hpp"
#ifdef HAVE_YAMLCPP
#include "io/yaml.hpp"
#endif
#include "base/exception.hpp"
#include "base/geometry.hpp"
#include "base/phys.hpp"
#include "base/mendeleev.hpp"

//
DdbPhonopy::DdbPhonopy() : Ddb() {
  _haveMasses = true;
}

//
DdbPhonopy::~DdbPhonopy() {
  ;
}

//
void DdbPhonopy::readFromFile(const std::string& filename) {
  using namespace geometry;
#ifndef HAVE_YAMLCPP
  (void) (filename);
  throw EXCEPTION("You need the yaml-cpp library to read a yaml file",ERRDIV);
#else
  try {
    YAML::Node fulldoc = YAML::LoadFile(filename);
    _natom = fulldoc["natom"].as<unsigned>();
    _ntypat = _natom;
    _typat.resize(_natom);
    _xred.resize(_natom);
    _xcart.resize(_natom);
    _znucl.resize(_natom);
    for ( unsigned i = 0 ; i < _natom ; ++i ) {
      _typat[i]=i+1;
      _xred[i]={0,0,0};
      _xcart[i]={0,0,0};
      _znucl[i]=0;
    }

    YAML::Node rlattice;
    if ( fulldoc["reciprocal_lattice"] ) {
      rlattice = fulldoc["reciprocal_lattice"];
    }
    else if ( fulldoc["reciprocal-basis"] ) {
      rlattice = fulldoc["reciprocal-basis"];
    }
    else {
      throw EXCEPTION("Need reciprocal lattice",ERRABT);
    }
    if ( rlattice.size() != 3 ) 
      throw EXCEPTION("Bad number of vectors for reciprocal lattice",ERRABT);

    for( unsigned i = 0 ; i < 3 ; ++i ) {
      vec3d vec = rlattice[i].as<vec3d>();
      _gprim[i  ] = vec[0]*phys::b2A; // vec/a2b = vec/(1/b2a) = vec*b2a
      _gprim[i+3] = vec[1]*phys::b2A;
      _gprim[i+6] = vec[2]*phys::b2A;
    }
    _rprim = invertTranspose(_gprim);

    auto qpoints = fulldoc["phonon"];
    std::clog << "Will read " << qpoints.size() << " qpoints" << std::endl;

    const double factor = phys::b2A*phys::b2A/phys::Ha2eV;
    for ( unsigned iqpt = 0 ; iqpt < qpoints.size() ; ++iqpt ) {
      vec3d qpt = qpoints[iqpt]["q-position"].as<vec3d>();
      auto dynmat = qpoints[iqpt]["dynamical_matrix"];
      if ( !dynmat.IsSequence() || dynmat.size() != 3*_natom ) throw EXCEPTION("Bad formatted file",ERRDIV);
      auto& block = this->getD2der(qpt);
      unsigned row = 0;
      for ( unsigned ipert1 = 0 ; ipert1 < _natom ; ++ipert1 ) {
        for ( unsigned idir1 = 0 ; idir1 < 3 ; ++idir1 ) {
          if ( !dynmat[row].IsSequence() && dynmat[row].size() != 3*_natom ) throw EXCEPTION("Bad formatted dynamical_matrix",ERRDIV);
          for ( unsigned ipert2 = 0, col = 0 ; ipert2 < _natom ; ++ipert2 ) {
            for ( unsigned idir2 = 0 ; idir2 < 3 ; ++idir2 ) {
              block.push_back(
                  std::make_pair(
                    std::array<unsigned,4>{{ idir1, ipert1, idir2, ipert2 }} , 
                    complex(dynmat[row][col].as<double>()*factor,dynmat[row][col+1].as<double>()*factor)
                    )
                  );
              col+=2;
            }
          }
          ++row;
        }
      }
    }
    this->blocks2Reduced();
    std::string bornfile = utils::dirname(filename)+"/BORN";
    std::ifstream born(bornfile);
    if ( born ) {
      _zion.resize(_typat.size()); 
      unsigned iline = 0;
      std::string line;
      utils::getline(born,line,iline); // First line is an conversion factor eventually.
      utils::getline(born,line,iline); 
      std::istringstream eps(line);
      mat3d epsinf;
      for ( unsigned i = 0 ; i < 9 ; ++i ) eps >> epsinf[i];
      if ( eps ) this->setEpsInf(epsinf);
      unsigned iatom = 0;
      while ( utils::getline(born,line,iline) && iatom < _natom) {
        std::istringstream zi(line);
        mat3d zeff;
        for ( unsigned i = 0 ; i < 9 ; ++i ) zi >> zeff[i];
        this->setZeff(iatom++,zeff);
      }
    }
  }
  catch (YAML::BadSubscript &e) {
    throw EXCEPTION("Bad subscript",ERRABT);
  }
  catch (YAML::Exception &e){
    throw EXCEPTION(e.what(), ERRDIV);
  }
#endif
}

//
void DdbPhonopy::buildFrom(const Dtset& dtset) {
  if ( _natom != dtset.natom() ) 
    throw EXCEPTION("Not the same number of atoms !!",ERRABT);
  Ddb::buildFrom(dtset);
  _zion.resize(_ntypat);
  std::fill_n(_zion.begin(),_ntypat,0); // all should be 0
  std::vector<geometry::mat3d> currentZ;
  for ( unsigned izeff = 0 ; izeff < _natom ; ++izeff ) {
    try {
      currentZ.push_back(this->getZeff(izeff));
    }
    catch (...) {
      break;
    }
  }
  if ( currentZ.size() != _natom ) {
    throw EXCEPTION("Number of Zeff is smaller than natom is not supported yet",ERRDIV);
#ifdef HAVE_SPGLIB
    SpglibDataset* spgDtset = this->getSpgDtset(0.001*phys::A2b);
    if ( spgDtset != nullptr ) {
      unsigned prevAtom = 0;
      for ( unsigned iatom = 0, izeff=0; iatom < _natom; ++iatom ) {
        unsigned eqatom = static_cast<unsigned>(spgDtset->equivalent_atoms[iatom]);
        this->setZeff(iatom,eqatom == prevAtom ? currentZ[izeff] : currentZ[++izeff]);
      }
    }
    spg_free_dataset(spgDtset);
#endif
  }
  if ( _typat.size() == _natom && _blocks.size() > 0) {
    for( auto& block : _blocks ){
      for ( auto& elt : block.second ) {
        const unsigned idir1 = elt.first[0];
        const unsigned ipert1 = elt.first[1];
        const unsigned idir2 = elt.first[2];
        const unsigned ipert2 = elt.first[3];
        if ( !(idir1 < 3 && idir2 < 3 && ipert1 < _natom && ipert2 < _natom) ) continue;
        elt.second *= std::sqrt(MendeTable.mass[_znucl[_typat[ipert1]-1]]*MendeTable.mass[_znucl[_typat[ipert2]-1]]);
      }
    }
  }
  _haveMasses = false;
}

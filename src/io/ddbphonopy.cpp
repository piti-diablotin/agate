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

using namespace Agate;

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
    complex *matrix = new complex[3*_natom*3*_natom];
    for ( unsigned iqpt = 0 ; iqpt < qpoints.size() ; ++iqpt ) {
      vec3d qpt = qpoints[iqpt]["q-position"].as<vec3d>();
      auto dynmat = qpoints[iqpt]["dynamical_matrix"];
      if ( !dynmat.IsSequence() || dynmat.size() != 3*_natom ) throw EXCEPTION("Bad formatted file",ERRDIV);
      std::vector<d2der> block;
      unsigned row = 0;
      for ( unsigned ipert1 = 0 ; ipert1 < _natom ; ++ipert1 ) {
        for ( unsigned idir1 = 0 ; idir1 < 3 ; ++idir1 ) {
          if ( !dynmat[row].IsSequence() && dynmat[row].size() != 3*_natom ) throw EXCEPTION("Bad formatted dynamical_matrix",ERRDIV);
          for ( unsigned ipert2 = 0, col = 0 ; ipert2 < _natom ; ++ipert2 ) {
            for ( unsigned idir2 = 0 ; idir2 < 3 ; ++idir2 ) {
              matrix[(ipert1*3+idir1)*3*_natom + ipert2*3+idir2] = complex(dynmat[row][col].as<double>()*factor,dynmat[row][col+1].as<double>()*factor);
              col+=2;
            }
          }
          ++row;
        }
      }

      // Go to reduce coordinates
      for ( unsigned ipert1 = 0 ; ipert1 < _natom ; ++ipert1 ) {
        for ( unsigned idir1 = 0 ; idir1 < 3 ; ++idir1 ) {
          for ( unsigned ipert2 = 0 ; ipert2 < _natom ; ++ipert2 ) {
            vec3d d2cartR;
            vec3d d2cartI;
            d2cartR[0] = matrix[(ipert1*3+idir1)*3*_natom + ipert2*3  ].real();
            d2cartI[0] = matrix[(ipert1*3+idir1)*3*_natom + ipert2*3  ].imag();
            d2cartR[1] = matrix[(ipert1*3+idir1)*3*_natom + ipert2*3+1].real();
            d2cartI[1] = matrix[(ipert1*3+idir1)*3*_natom + ipert2*3+1].imag();
            d2cartR[2] = matrix[(ipert1*3+idir1)*3*_natom + ipert2*3+2].real();
            d2cartI[2] = matrix[(ipert1*3+idir1)*3*_natom + ipert2*3+2].imag();
            vec3d d2redRowR = _rprim * d2cartR;
            vec3d d2redRowI = _rprim * d2cartI;
            matrix[(ipert1*3+idir1)*3*_natom + ipert2*3  ].real(d2redRowR[0]);
            matrix[(ipert1*3+idir1)*3*_natom + ipert2*3  ].imag(d2redRowI[0]);
            matrix[(ipert1*3+idir1)*3*_natom + ipert2*3+1].real(d2redRowR[1]);
            matrix[(ipert1*3+idir1)*3*_natom + ipert2*3+1].imag(d2redRowI[1]);
            matrix[(ipert1*3+idir1)*3*_natom + ipert2*3+2].real(d2redRowR[2]);
            matrix[(ipert1*3+idir1)*3*_natom + ipert2*3+2].imag(d2redRowI[2]);
          }
        }
      }
      //Second loop : change basis from reduced to cartesian (columns)
      for ( unsigned ipert1 = 0 ; ipert1 < _natom ; ++ipert1 ) {
        for ( unsigned ipert2 = 0 ; ipert2 < _natom ; ++ipert2 ) {
          for ( unsigned idir2 = 0 ; idir2 < 3 ; ++idir2 ) {
            vec3d d2redRowR;
            vec3d d2redRowI;
            d2redRowR[0] = matrix[(ipert1*3  )*3*_natom + ipert2*3+idir2].real();
            d2redRowI[0] = matrix[(ipert1*3  )*3*_natom + ipert2*3+idir2].imag();
            d2redRowR[1] = matrix[(ipert1*3+1)*3*_natom + ipert2*3+idir2].real(); 
            d2redRowI[1] = matrix[(ipert1*3+1)*3*_natom + ipert2*3+idir2].imag(); 
            d2redRowR[2] = matrix[(ipert1*3+2)*3*_natom + ipert2*3+idir2].real();
            d2redRowI[2] = matrix[(ipert1*3+2)*3*_natom + ipert2*3+idir2].imag();
            vec3d d2redR = _rprim * d2redRowR;
            vec3d d2redI = _rprim * d2redRowI;

            for ( unsigned idir1 = 0 ; idir1 < 3 ; ++idir1 ) {
              block.push_back(
                  std::make_pair(
                    std::array<unsigned,4>{{ idir1, ipert1, idir2, ipert2 }} , 
                    complex(d2redR[idir1],d2redI[idir1])
                    )
                  );
            }
          }
        }
      }



      _blocks.insert(std::make_pair( qpt, block));
    }

  }
  catch (YAML::BadSubscript &e) {
    throw EXCEPTION("Bad subscript",ERRABT);
  }
  catch (...) {
    throw EXCEPTION("Something unexpected happened", ERRDIV);
  }
#endif
}

//
void DdbPhonopy::buildFrom(const Dtset& dtset) {
  if ( _natom != dtset.natom() ) 
    throw EXCEPTION("Not the same number of atoms !!",ERRABT);
  Ddb::buildFrom(dtset);
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

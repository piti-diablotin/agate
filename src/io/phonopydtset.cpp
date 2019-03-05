/**
 * @file src/./phonopydtset.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2018 Jordan Bieder
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


#include "io/phonopydtset.hpp"
#ifdef HAVE_YAMLCPP
#include "io/yaml.hpp"
#endif
#include "base/mendeleev.hpp"
#include "base/phys.hpp"

using Agate::mendeleev;

//
PhonopyDtset::PhonopyDtset() : Dtset() {
  ;
}

//
PhonopyDtset::~PhonopyDtset() {
  ;
}

void PhonopyDtset::readFromFile(const std::string& filename) {
#ifndef HAVE_YAMLCPP
  (void) (filename);
  throw EXCEPTION("You need the yaml-cpp library to read a yaml file",ERRDIV);
#else

  YAML::Node fulldoc;
  try { 
    fulldoc = YAML::LoadFile(filename);
  }
  catch(...) {
    throw EXCEPTION("Cannot read file",ERRDIV);
  }
  this->readFromYAML(fulldoc);
#endif
}

#ifdef HAVE_YAMLCPP
void PhonopyDtset::readFromYAML(const YAML::Node &doc) {
  try {
    _natom = doc["natom"].as<unsigned>();
    _spinat.clear();

    double scaling = phys::A2b;
    if ( doc["calculator"] && doc["calculator"].as<std::string>() == std::string("abinit") ) scaling = 1e0;

    auto lattice = doc["lattice"];
    _rprim[0] = lattice[0][0].as<double>()*scaling;
    _rprim[3] = lattice[0][1].as<double>()*scaling;
    _rprim[6] = lattice[0][2].as<double>()*scaling;
    _rprim[1] = lattice[1][0].as<double>()*scaling;
    _rprim[4] = lattice[1][1].as<double>()*scaling;
    _rprim[7] = lattice[1][2].as<double>()*scaling;
    _rprim[2] = lattice[2][0].as<double>()*scaling;
    _rprim[5] = lattice[2][1].as<double>()*scaling;
    _rprim[8] = lattice[2][2].as<double>()*scaling;

    YAML::Node points;
    std::string coordinates = "coordinates";
    try {
      points = doc["points"];
    }
    catch (...) {
      points = doc["atoms"];
      coordinates = "position";
    }
    _xred.resize(_natom);
    _xcart.resize(_natom);
    _typat.resize(_natom);
    if ( points.size() != _natom ) 
      throw EXCEPTION("Band number of atoms",ERRABT);
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      auto data = points[iatom];
      _xred[iatom] = data[coordinates].as<geometry::vec3d>();
      std::string name = data["symbol"].as<std::string>();
      unsigned z = mendeleev::znucl(name);
      auto it = std::find(_znucl.begin(),_znucl.end(),z);
      if ( it != _znucl.end() ) {
        int typat = std::distance(_znucl.begin(),it);
        _typat[iatom] = typat+1;
      }
      else {
        _znucl.push_back(z);
        _typat[iatom] = _znucl.size();
      }
    }
    _ntypat = _znucl.size();
    _gprim = geometry::invertTranspose(_rprim);
    geometry::changeBasis(_rprim, _xcart, _xred, false);
  }
  catch ( Exception &e ) {
    e.ADD("Aborting yaml parsing",ERRABT);
  }
  catch ( ... ) {
    throw EXCEPTION("Yaml error happend",ERRABT);
  }
}
#endif 

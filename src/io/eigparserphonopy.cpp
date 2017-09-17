/**
 * @file src/eigparserphonopy.cpp
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


#include "io/eigparserphonopy.hpp"
#ifdef HAVE_YAMLCPP
#include "io/yaml.hpp"
#endif
#include "base/exception.hpp"
#include "base/geometry.hpp"

#ifdef HAVE_YAMLCPP
namespace YAML {
  using geometry::vec3d;
  template<>
    struct convert<vec3d> {
      static Node encode(const vec3d& vec) {
        Node node;
        node.push_back(vec[0]);
        node.push_back(vec[1]);
        node.push_back(vec[2]);
        return node;
      }

      static bool decode(const Node& node, vec3d& vec) {
        if(!node.IsSequence() || node.size() != 3) {
          return false;
        }

        vec[0] = node[0].as<double>();
        vec[1] = node[1].as<double>();
        vec[2] = node[2].as<double>();
        return true;
      }
    };
}
#endif

//
EigParserPhonopy::EigParserPhonopy() : EigParser() {
}

//
EigParserPhonopy::~EigParserPhonopy() {
  ;
}

//
void EigParserPhonopy::readFromFile(const std::string& filename) {
#ifndef HAVE_YAMLCPP
  (void) (filename);
  throw EXCEPTION("You need the yaml-cpp library to read a yaml file",ERRDIV);
#else

  YAML::Node fulldoc;
  try { 
    fulldoc = YAML::LoadFile(filename);
  }
  catch(...) {
    throw EXCEPTION("Cannot read file",ERRABT);
  }
  try {
    unsigned nband = 3*fulldoc["natom"].as<unsigned>();
    unsigned npath = fulldoc["npath"].as<unsigned>();
    auto qpoints = fulldoc["phonon"];
    //unsigned nqpoints = fulldoc["nqpoint"].as<unsigned>();
    unsigned nqpoints = qpoints.size();
    std::clog << "Will read " << qpoints.size() << " qpoints" << std::endl;
    if ( fulldoc["segment_nqpoint"] && fulldoc["segment_nqpoint"].IsSequence() ) {
      _ndiv.resize(fulldoc["segment_nqpoint"].size());
      for ( unsigned i = 0 ; i < fulldoc["segment_nqpoint"].size() ; ++i )
        _ndiv[i] = fulldoc["segment_nqpoint"][i].as<unsigned>();
    }
    else {
      _ndiv.resize(npath,(unsigned)(nqpoints/npath));
    }
    for ( unsigned iqpt = 0 ; iqpt < qpoints.size() ; ++iqpt ) {
      geometry::vec3d qpt = qpoints[iqpt]["q-position"].as<geometry::vec3d>();
      if ( qpoints[iqpt]["label"] ) {
        if ( _labels.size() == 0 )
          _labels.push_back(qpoints[iqpt]["label"].as<std::string>());
        else if ( _labels.back() != qpoints[iqpt]["label"].as<std::string>()  )
          _labels.push_back(qpoints[iqpt]["label"].as<std::string>());
      }
      double length = qpoints[iqpt]["distance"].as<double>();
      auto nodeBand = qpoints[iqpt]["band"];
      if ( !nodeBand.IsSequence() || nodeBand.size() != nband ) throw EXCEPTION("Bad formatted file",ERRDIV);
      std::vector<double> values(nband,0);
      for ( unsigned i = 0 ; i < nodeBand.size() ; ++i )
        values[i] = nodeBand[i]["frequency"].as<double>();
      _eigens.push_back(std::move(values));
      _kpts.push_back(qpt);
      _lengths.push_back(length);
    }
    _filename = filename;
    _nband = nband;
    _eunit = THz;
    _hasSpin = false;
  }
  catch (YAML::BadSubscript &e) {
    throw EXCEPTION("Bad subscript",ERRABT);
  }
  catch (...) {
    throw EXCEPTION("Something unexpected happened", ERRDIV);
  }
#endif
}

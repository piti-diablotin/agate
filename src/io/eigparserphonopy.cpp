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
#include "io/phonopydtset.hpp"

//
EigParserPhonopy::EigParserPhonopy() : EigParserPhonons() {
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
    throw EXCEPTION("Cannot read file",ERRDIV);
  }
  try {
    try {
      PhonopyDtset *tmp = new PhonopyDtset;
      tmp->readFromYAML(fulldoc);
      _dtset.reset(tmp);
    }
    catch (Exception& e) {
      _dtset.reset(nullptr); // safety
    }
    unsigned natom = fulldoc["natom"].as<unsigned>();
    unsigned nband = 3*natom;
    _nband = nband;
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
        utils::trim(_labels.back());
      }
      double length = qpoints[iqpt]["distance"].as<double>();
      auto nodeBand = qpoints[iqpt]["band"];
      if ( !nodeBand.IsSequence() || nodeBand.size() != nband ) throw EXCEPTION("Bad formatted file",ERRDIV);
      std::vector<double> values(nband,0);
      std::vector<double> disp(2*nband*nband,0);
      bool hasVector = false;
      for ( unsigned i = 0 ; i < nodeBand.size() ; ++i ) {
        values[i] = nodeBand[i]["frequency"].as<double>();
        if ( nodeBand[i]["eigenvector"] ) {
          hasVector = true;
          auto vectors = nodeBand[i]["eigenvector"];
          for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
            for ( unsigned idir = 0 ; idir < 3 ; ++idir ) {
              disp[2*nband*i+3*2*iatom+idir*2] = vectors[iatom][idir][0].as<double>();
              disp[2*nband*i+3*2*iatom+idir*2+1] = vectors[iatom][idir][1].as<double>();
            }
          }
        }
      }
      _eigens.push_back(std::move(values));
      if ( hasVector ) _eigenDisp.push_back(std::move(disp));
      _kpts.push_back(qpt);
      _lengths.push_back(length);
    }
    _filename = filename;
    _eunit = UnitConverter(UnitConverter::THz);
    _hasSpin = false;
    if ( !_eigenDisp.empty() && _dtset != nullptr )
      this->renormalizeEigenDisp();
    else if ( !_eigenDisp.empty() && _dtset == nullptr )
      throw EXCEPTION("Eigen displacements read but no dtset found -> fatband unavailable",ERRWAR);
  }
  catch (YAML::BadSubscript &e) {
    throw EXCEPTION("Bad subscript",ERRABT);
  }
  catch (Exception& e) {
    throw e;
  }
  catch (...) {
    throw EXCEPTION("Something unexpected happened", ERRDIV);
  }
#endif
}

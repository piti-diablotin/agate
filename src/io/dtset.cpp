/**
 * @file dtset.cpp
 *
 * @brief Implementation of the Dtset class
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


#include "io/dtset.hpp"
#include "io/configparser.hpp"
#include "bind/findsym.hpp"
#include <iomanip>
#include <string>
#include <fstream>
#include <algorithm>
#include <utility>
#include <cmath>
#include "base/phys.hpp"
#include "base/mendeleev.hpp"
#include "base/utils.hpp"
#include "io/cifparser.hpp"
#ifdef HAVE_SPGLIB
#  ifdef __cplusplus
extern "C" {
#  endif
#  include "spglib/spglib.h"
#  ifdef __cplusplus
}
#  endif
#endif
#include "phonons/supercell.hpp"
#include "hist/histdatamd.hpp"

using namespace Agate;

//
Dtset::Dtset() : 
  _natom(0),
  _ntypat(0),
  _typat(),
  _znucl(),
  _acell({{1.0,1.0,1.0}}),                           ///< Initialize with 1 scaling factor for each vector.
  _rprim({{1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0}}),   ///< Initialize with the identity matrix.
  _gprim({{1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0}}),   ///< Initialize with the identity matrix
  _xcart(),   ///< No atom first so no position
  _xred(),    ///< No atom first so no position
  _spinat(),  ///< No atom first so no position
  _velocities(), 
  _fcart(),
  _findsym()  ///< Return string by findsym so nothing first.
{
}


//
Dtset::Dtset(const HistData &hist, const unsigned itime) : 
  _natom(0),
  _ntypat(0),
  _typat(),
  _znucl(),
  _acell({{1.0,1.0,1.0}}),                           ///< Initialize with 1 scaling factor for each vector.
  _rprim({{1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0}}),   ///< Initialize with the identity matrix.
  _gprim({{1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0}}),   ///< Initialize with the identity matrix
  _xcart(),   ///< No atom first so no position
  _xred(),    ///< No atom first so no position
  _spinat(),
  _velocities(),
  _fcart(),
  _findsym()  ///< Return string by findsym so nothing first.
{
  const unsigned ntime = hist.ntime();
  if ( itime >= ntime )
    throw EXCEPTION("Bad itime value",ERRABT);
  const double *xred = hist.getXred(itime);
  const double *xcart = hist.getXcart(itime);
  const double *rprimd = hist.getRprimd(itime);
  const double *spinat = hist.getSpinat(itime);

  _natom = hist.natom();
  _ntypat = hist.znucl().size();
  _typat = hist.typat();
  _znucl = hist.znucl();

  _rprim[0] = rprimd[0];
  _rprim[1] = rprimd[1];
  _rprim[2] = rprimd[2];
  _rprim[3] = rprimd[3];
  _rprim[4] = rprimd[4];
  _rprim[5] = rprimd[5];
  _rprim[6] = rprimd[6];
  _rprim[7] = rprimd[7];
  _rprim[8] = rprimd[8];

  _gprim = geometry::invertTranspose(_rprim);

  for ( unsigned iatom = 0; iatom < _natom; ++iatom ) {
    _xred.push_back({{xred[iatom*3], xred[iatom*3+1], xred[iatom*3+2]}});
    _xcart.push_back({{xcart[iatom*3], xcart[iatom*3+1], xcart[iatom*3+2]}});
    if ( spinat != nullptr ) 
      _spinat.push_back({{spinat[iatom*3], spinat[iatom*3+1], spinat[iatom*3+2]}});
  }

  const double* fcart = hist.getFcart(itime);
  if ( fcart != nullptr ){
    for ( unsigned iatom = 0; iatom < _natom; ++iatom ) {
      _fcart.push_back({{fcart[iatom*3], fcart[iatom*3+1], fcart[iatom*3+2]}});
    }
  }

  try {
    const HistDataMD& md = dynamic_cast<const HistDataMD&>(hist);
    const double* vel = md.getVel(itime);
    if ( vel != nullptr ){
      for ( unsigned iatom = 0; iatom < _natom; ++iatom ) {
        _velocities.push_back({{vel[iatom*3], vel[iatom*3+1], vel[iatom*3+2]}});
      }
    }
  }
  catch(...) {
  }

  _acell[0] = hist.getAcell(itime)[0];//sqrt(_rprim[0]*_rprim[0]+_rprim[3]*_rprim[3]+_rprim[6]*_rprim[6]);
  _acell[1] = hist.getAcell(itime)[1];//sqrt(_rprim[1]*_rprim[1]+_rprim[4]*_rprim[4]+_rprim[7]*_rprim[7]);
  _acell[2] = hist.getAcell(itime)[2];//sqrt(_rprim[2]*_rprim[2]+_rprim[5]*_rprim[5]+_rprim[8]*_rprim[8]);

}


//
Dtset::~Dtset(){
}


//
void Dtset::readFromFile(const std::string& filename) {
  ConfigParser parser(filename);
  try {
    this->readConfig(parser);
  }
  catch( Exception& e ) {
    std::string err_str = "Failed to build Dtset from file "+filename;
    e.ADD(err_str,e.getReturnValue());
    throw e;
  }
}


//
void Dtset::readConfig(ConfigParser& parser, unsigned img, unsigned jdtset) {
  std::string token;
  int tokenValueCheck; 
  unsigned step = 0;

  try {
    parser.parse();
  }
  catch( Exception& e ) {
    e.ADD("ConfigParser failed to parse.",ERRDIV);
    throw e;
  }

  std::string suffix_img = ( img == 0 ? "" : "_"+utils::to_string(img)+"img" );
  std::string suffix_jdtset = ( jdtset == 0 ? "" : utils::to_string(jdtset) );

  try {
    token = "natom";
    tokenValueCheck = parser.getToken<int>(token);
    if ( tokenValueCheck <= 0 ) {
      std::string err_str = "Negative value ("+ utils::to_string(tokenValueCheck)
        +") for token " + token + " is not allowed";
      throw EXCEPTION(err_str,ERRABT);
    }
    _natom = (unsigned) tokenValueCheck;
    ++step;

    try {
      token = "ntypat";
      tokenValueCheck = parser.getToken<int>(token);
      if ( tokenValueCheck <= 0 ) {
        std::string err_str = "Negative value ("+ utils::to_string(tokenValueCheck)
          +") for token " + token + " is not allowed";
        throw EXCEPTION(err_str,ERRABT);
      }
    }
    catch (Exception& e){
      if ( e.getReturnValue() == ConfigParser::ERFOUND )
        tokenValueCheck = 0;
      else
        throw e;
    }
    _ntypat = (unsigned) tokenValueCheck;

    token = "typat";
    _typat = parser.getToken<int>(token,_natom);
    int compVal = (_ntypat!=0) ? _ntypat : _natom;
    for( auto& typat : _typat ) {
      if ( typat > compVal ) {
        std::string err_str = "Found value " + utils::to_string(typat) 
          + " for token " + token + "\nwhich should have only value smaller or equal to " 
          + utils::to_string(compVal);
        throw EXCEPTION(err_str,ERRABT);
      }
    }
    if ( _ntypat == 0 ) _ntypat = *std::max_element(_typat.begin(), _typat.end());
    ++step;

    try {
      token = "znucl";
      auto znucl = parser.getToken<double>(token,_ntypat);
      for( auto& z : znucl ) {
        if ( z > 118 || z == 0 ) { //max znucl allowed
          std::string err_str = "Found value " + utils::to_string(z) 
            + " for token " + token + "\nwhich should be smaller than 118 ";
          throw EXCEPTION(err_str,ERRWAR);
        }
        _znucl.push_back((int)z);
      }
    }
    catch (Exception& e){
      if ( e.getReturnValue() == ERRWAR || e.getReturnValue() == ConfigParser::ERFOUND ) {
        std::clog << e.fullWhat() << std::endl;
        _znucl.clear();
      }
      else
        throw e;
    }
    ++step;

    std::vector<double> tokenVectorCheck;
    ConfigParser::Characteristic length = ConfigParser::Characteristic::LENGTH;
    try {
      token = "acell"+suffix_jdtset/*+suffix*/;
      if ( !parser.hasToken(token) ) token = "acell";
      tokenVectorCheck = parser.getToken<double>(token,3,length);
      // Need to convert vector to array by hand.
      _acell[0] = tokenVectorCheck[0];
      _acell[1] = tokenVectorCheck[1];
      _acell[2] = tokenVectorCheck[2];
    }
    catch (Exception& e){
      if ( e.getReturnValue() == ERRWAR || e.getReturnValue() == ConfigParser::ERFOUND ) {
        //std::clog << e.fullWhat() << std::endl;
        _acell[0] = 1.0;
        _acell[1] = 1.0;
        _acell[2] = 1.0;
      }
      else
        throw e;
    }
    ++step;

    double scalecart[3] = {0.0};
    try {
      token = "scalecart"+suffix_jdtset;
      if ( !parser.hasToken(token) ) token = "scalcart";
      tokenVectorCheck = parser.getToken<double>(token,3,length);
      // Need to convert vector to array by hand.
      scalecart[0] = tokenVectorCheck[0];
      scalecart[1] = tokenVectorCheck[1];
      scalecart[2] = tokenVectorCheck[2];
    }
    catch (Exception& e){
      if ( e.getReturnValue() == ERRWAR || e.getReturnValue() == ConfigParser::ERFOUND ) {
        //std::clog << e.fullWhat() << std::endl;
        scalecart[0] = 1.0;
        scalecart[1] = 1.0;
        scalecart[2] = 1.0;
      }
      else
        throw e;
    }
    ++step;

    try {
      token = "rprim"+suffix_jdtset/*+suffix*/;
      if ( !parser.hasToken(token) ) token = "rprim";
      tokenVectorCheck = parser.getToken<double>(token,3*3);
      _rprim[geometry::mat3dind(1,1)] = tokenVectorCheck[0];
      _rprim[geometry::mat3dind(2,1)] = tokenVectorCheck[3];
      _rprim[geometry::mat3dind(3,1)] = tokenVectorCheck[6];
      _rprim[geometry::mat3dind(1,2)] = tokenVectorCheck[1];
      _rprim[geometry::mat3dind(2,2)] = tokenVectorCheck[4];
      _rprim[geometry::mat3dind(3,2)] = tokenVectorCheck[7];
      _rprim[geometry::mat3dind(1,3)] = tokenVectorCheck[2];
      _rprim[geometry::mat3dind(2,3)] = tokenVectorCheck[5];
      _rprim[geometry::mat3dind(3,3)] = tokenVectorCheck[8];
    }
    catch (Exception& e){
      if ( e.getReturnValue() != ERRWAR && e.getReturnValue() != ConfigParser::ERFOUND ) {
        e.ADD("Bad rprim construction",ERRABT);
        throw e;
      }
      else {
        try {
          token = "angdeg"+suffix_jdtset;
          if ( !parser.hasToken(token) ) token = "angdeg";
          tokenVectorCheck = parser.getToken<double>(token,3);
          this->buildRprim(tokenVectorCheck.data());
        }
        catch (Exception &e ) {
          if ( e.getReturnValue() != ERRWAR && e.getReturnValue() != ConfigParser::ERFOUND ) {
            e.ADD("Bad rprim construction",ERRABT);
            throw e;
          }
        }
      }
    }
    _rprim[geometry::mat3dind(1,1)] *= _acell[0] * scalecart[0]; // vec1.x
    _rprim[geometry::mat3dind(2,1)] *= _acell[1] * scalecart[0]; // vec2.x
    _rprim[geometry::mat3dind(3,1)] *= _acell[2] * scalecart[0]; // vec3.x
    _rprim[geometry::mat3dind(1,2)] *= _acell[0] * scalecart[1]; // vec1.y
    _rprim[geometry::mat3dind(2,2)] *= _acell[1] * scalecart[1]; // vec2.y
    _rprim[geometry::mat3dind(3,2)] *= _acell[2] * scalecart[1]; // vec3.y
    _rprim[geometry::mat3dind(1,3)] *= _acell[0] * scalecart[2]; // vec1.z
    _rprim[geometry::mat3dind(2,3)] *= _acell[1] * scalecart[2]; // vec2.z
    _rprim[geometry::mat3dind(3,3)] *= _acell[2] * scalecart[2]; // vec3.z

    /*
    _acell[0] = 1.0;
    _acell[1] = 1.0;
    _acell[2] = 1.0;
    */

    if ( geometry::det(_rprim) < 0.0e0 )
      throw EXCEPTION("rprim is not direct.\nChange order",ERRABT);
    ++step;

    _gprim = geometry::invertTranspose(_rprim);

    try { 
      token = "xcart"+suffix_img+suffix_jdtset;
      if ( !parser.hasToken(token) ) token = "xcart"+suffix_img;
      double factor = 1.0;
      try {
        tokenVectorCheck = parser.getToken<double>(token,3*_natom);
      }
      catch (Exception &e) {
        if ( e.getReturnValue() == ConfigParser::ERFOUND ) {
          token = "xangst"+suffix_img+suffix_jdtset;
          if ( !parser.hasToken(token) ) token = "xangst"+suffix_img;
          tokenVectorCheck = parser.getToken<double>(token,3*_natom);
          factor = 1.0/phys::b2A;
        }
        else {
          e.ADD("Bad xcart parameter",ERRDIV);
          throw e;
        }
      }
      _xcart.resize(_natom);
      for( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) 
        _xcart[iatom] = {{tokenVectorCheck[iatom*3]*factor,tokenVectorCheck[iatom*3+1]*factor,tokenVectorCheck[iatom*3+2]*factor}};
#ifdef HAVE_SHRINK_TO_FIT
      _xcart.shrink_to_fit();
#endif

      geometry::changeBasis(_rprim, _xcart, _xred,true);
    }
    catch (Exception& e) {
      if ( e.getReturnValue() == ConfigParser::ERFOUND ) {
        token = "xred"+suffix_img+suffix_jdtset;
        if ( !parser.hasToken(token) ) token = "xred"+suffix_img;
        try {
          tokenVectorCheck = parser.getToken<double>(token,3*_natom);
        }
        catch (Exception& e) {
          if ( e.getReturnValue() == ConfigParser::ERFOUND ) {
            tokenVectorCheck.clear();
            tokenVectorCheck.resize(3*_natom,0);
          }
          else {
            e.ADD("Problem finding the positions of the atoms",ERRABT);
            throw e;
          }
        }
        _xred.resize(_natom);
        for( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) 
          _xred[iatom] = {{tokenVectorCheck[iatom*3],tokenVectorCheck[iatom*3+1],tokenVectorCheck[iatom*3+2]}};
#ifdef HAVE_SHRINK_TO_FIT
        _xred.shrink_to_fit();
#endif

        geometry::changeBasis(_rprim, _xcart, _xred, false);
      }
      else {
        e.ADD("Neither xcart nor xred were found in the input file",ERRABT);
        throw e;
      }
    }
    ++step;

    try { 
      token = "spinat"+suffix_jdtset;
      if ( !parser.hasToken(token) ) token = "spinat";
      tokenVectorCheck = parser.getToken<double>(token,3*_natom);
      _spinat.resize(_natom);
      for( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) 
        _spinat[iatom] = {{tokenVectorCheck[iatom*3],tokenVectorCheck[iatom*3+1],tokenVectorCheck[iatom*3+2]}};
#ifdef HAVE_SHRINK_TO_FIT
      _spinat.shrink_to_fit();
#endif
    }
    catch (Exception& e) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND ) {
        e.ADD("Bad spinat parameter",ERRABT);
        throw e;
      }
    }
    ++step;

    try { 
      token = "vel"+suffix_jdtset;
      if ( !parser.hasToken(token) ) token = "vel";
      tokenVectorCheck = parser.getToken<double>(token,3*_natom);
      _velocities.resize(_natom);
      for( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) 
        _velocities[iatom] = {{tokenVectorCheck[iatom*3],tokenVectorCheck[iatom*3+1],tokenVectorCheck[iatom*3+2]}};
#ifdef HAVE_SHRINK_TO_FIT
      _velocities.shrink_to_fit();
#endif
    }
    catch (Exception& e) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND ) {
        e.ADD("Bad spinat parameter",ERRABT);
        throw e;
      }
    }
    ++step;

    try {
      token = "fcart"+suffix_jdtset;
      if ( !parser.hasToken(token) ) token = "fcart";
      tokenVectorCheck = parser.getToken<double>(token,3*_natom);
      _fcart.resize(_natom);
      for( unsigned iatom = 0 ; iatom < _natom ; ++iatom )
        _fcart[iatom] = {{tokenVectorCheck[iatom*3],tokenVectorCheck[iatom*3+1],tokenVectorCheck[iatom*3+2]}};
#ifdef HAVE_SHRINK_TO_FIT
      _fcart.shrink_to_fit();
#endif
    }
    catch (Exception& e) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND ) {
        e.ADD("Bad spinat parameter",ERRABT);
        throw e;
      }
    }
    ++step;

    try { 
      token = "supercell_latt"+suffix_jdtset;
      if ( !parser.hasToken(token) ) token = "supercell_latt";
      tokenVectorCheck = parser.getToken<double>(token,9);
      if ( tokenVectorCheck[0] < 1. || tokenVectorCheck[4] < 1. || tokenVectorCheck[8] < 1. )
        throw EXCEPTION("Supercell can only be built with diagonal part",ERRABT);
      Supercell supercell(*this,(unsigned)tokenVectorCheck[0],(unsigned)tokenVectorCheck[4],(unsigned)tokenVectorCheck[8]);
      *this = supercell;
    }
    catch (Exception& e) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND ) {
        e.ADD("Bad supercell_latt parameter",ERRABT);
        throw e;
      }
    }
    ++step;

    token = "amu"+suffix_jdtset;
    try { 
      if ( !parser.hasToken(token) ) token = "amu";
      tokenVectorCheck = parser.getToken<double>(token,_znucl.size());
      for ( unsigned z = 0 ; z < _znucl.size() ; ++z )
        MendeTable.mass[_znucl[z]] = tokenVectorCheck[z];
    }
    catch (Exception& e) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND ) {
        e.ADD("Bad amu parameter",ERRABT);
        throw e;
      }
    }
    ++step;
  }
  catch( Exception& e ) {
    e.ADD("ConfigParser failed to get all required parameters.",(step < 2) ? e.getReturnValue() : ERRABT);
    throw e;
  }

}


//
void Dtset::dump(std::ostream& out) const {
  try { 
    out << "# Input file for Abinit" << std::endl;
    out << "# This file was automatically generated by " << PACKAGE_NAME << " version " << PACKAGE_VERSION << std::endl;
    out << "# There is no warranty this file should work." << std::endl << std::endl;

    out.setf(std::ios::left,std::ios::adjustfield);
    out << std::setw(14) << "natom" << _natom << std::endl;
    out << std::endl;

    out << std::setw(14) << "ntypat" << _ntypat << std::endl;
    out << std::endl;

    if ( _znucl.size() != 0 ) {
      out << std::setw(14) << "# Species";
      for ( unsigned itypat = 0 ; itypat < _ntypat ; ++itypat ) {
        if ( itypat % 10 == 0 ) out << std::endl << std::setw(14) << "#";
        out << std::setw(4) << Mendeleev::name[_znucl[itypat]];
      }
      out << std::endl;
      out << std::setw(14) << "znucl" ;
      for ( unsigned itypat = 0 ; itypat < _ntypat ; ++itypat ) {
        if ( itypat % 10 == 0 ) out << std::endl << std::setw(14) << " ";
        out << std::setw(4) << _znucl[itypat];
      }
      out << std::endl;
      out << std::endl;
    }

    out << std::setw(14) << "typat";
    for ( unsigned itypat = 0 ; itypat < _natom ; ++itypat ) {
      if ( itypat % 10 == 0 ) out << std::endl << std::setw(14) << " ";
      out << std::setw(4) << _typat[itypat];
    }
    out << std::endl;
    out << std::endl;

    out << std::setw(13) << "acell";
    out.setf(std::ios::scientific,std::ios::floatfield);
    out.precision(14);
    out.setf(std::ios::right,std::ios::adjustfield);
    out << std::setw(21) << _acell[0] << std::setw(23) << _acell[1] << std::setw(23) << _acell[2] << std::endl;
    out << std::endl;

    out.setf(std::ios::left,std::ios::adjustfield);
    out << std::setw(14) << "rprim" << std::endl;
    out.setf(std::ios::right,std::ios::adjustfield);
    out << std::setw(34) << _rprim[geometry::mat3dind(1,1)]/_acell[0] << std::setw(23) << _rprim[geometry::mat3dind(1,2)]/_acell[0] << std::setw(23) << _rprim[geometry::mat3dind(1,3)]/_acell[0] << std::endl;
    out << std::setw(34) << _rprim[geometry::mat3dind(2,1)]/_acell[1] << std::setw(23) << _rprim[geometry::mat3dind(2,2)]/_acell[1] << std::setw(23) << _rprim[geometry::mat3dind(2,3)]/_acell[1] << std::endl;
    out << std::setw(34) << _rprim[geometry::mat3dind(3,1)]/_acell[2] << std::setw(23) << _rprim[geometry::mat3dind(3,2)]/_acell[2] << std::setw(23) << _rprim[geometry::mat3dind(3,3)]/_acell[2] << std::endl;
    out << std::endl;

    out.setf(std::ios::left,std::ios::adjustfield);
    out << std::setw(14) << "# For information : gprimd" << std::endl;
    out.setf(std::ios::right,std::ios::adjustfield);
    out << '#' << std::setw(33) << _gprim[geometry::mat3dind(1,1)] << std::setw(23) << _gprim[geometry::mat3dind(1,2)] << std::setw(23) << _gprim[geometry::mat3dind(1,3)] << std::endl;
    out << '#' << std::setw(33) << _gprim[geometry::mat3dind(2,1)] << std::setw(23) << _gprim[geometry::mat3dind(2,2)] << std::setw(23) << _gprim[geometry::mat3dind(2,3)] << std::endl;
    out << '#' << std::setw(33) << _gprim[geometry::mat3dind(3,1)] << std::setw(23) << _gprim[geometry::mat3dind(3,2)] << std::setw(23) << _gprim[geometry::mat3dind(3,3)] << std::endl;
    out << std::endl;

    out.setf(std::ios::left,std::ios::adjustfield);
    out << std::setw(13) << "# angdeg";
    geometry::vec3d angle = geometry::angle(_rprim);
    out.setf(std::ios::scientific,std::ios::floatfield);
    out.precision(14);
    out.setf(std::ios::right,std::ios::adjustfield);
    out << std::setw(21) << angle[0] << std::setw(23) << angle[1] << std::setw(23) << angle[2] << std::endl;
    out << std::endl;

    out.setf(std::ios::left,std::ios::adjustfield);
    out << std::setw(14) << "# xcart" << std::endl;
    out.setf(std::ios::right,std::ios::adjustfield);
    for ( auto& coord : _xcart )
      out << '#' <<  std::setw(33) << coord[0] << std::setw(23) << coord[1] << std::setw(23) << coord[2] << std::endl;

    out.setf(std::ios::left,std::ios::adjustfield);
    out << std::endl;
    out << std::setw(14) << "xred" << std::endl;
    out.setf(std::ios::right,std::ios::adjustfield);
    for ( auto& coord : _xred )
      out << std::setw(34) << coord[0] << std::setw(23) << coord[1] << std::setw(23) << coord[2] << std::endl;

    if ( !_spinat.empty()) {
      out.setf(std::ios::left,std::ios::adjustfield);
      out << std::endl;
      out << std::setw(14) << "spinat" << std::endl;
      out.setf(std::ios::right,std::ios::adjustfield);
      for ( auto& spin : _spinat )
        out << std::setw(34) << spin[0] << std::setw(23) << spin[1] << std::setw(23) << spin[2] << std::endl;
    }

    if ( !_velocities.empty()) {
      out.setf(std::ios::left,std::ios::adjustfield);
      out << std::endl;
      out << std::setw(14) << "vel" << std::endl;
      out.setf(std::ios::right,std::ios::adjustfield);
      for ( auto& vel : _velocities )
        out << std::setw(34) << vel[0] << std::setw(23) << vel[1] << std::setw(23) << vel[2] << std::endl;
    }

    if ( !_fcart.empty()) {
      out.setf(std::ios::left,std::ios::adjustfield);
      out << std::endl;
      out << std::setw(14) << "# fcart" << std::endl;
      out.setf(std::ios::right,std::ios::adjustfield);
      for ( auto& fcart : _fcart)
        out << "#" << std::setw(33) << fcart[0] << std::setw(23) << fcart[1] << std::setw(23) << fcart[2] << std::endl;
    }
  }
  catch (...) {
    throw EXCEPTION("Something went wrong during the dumping of Dtset", ERRDIV);
  }

}


//
void Dtset::dump(const std::string& filename) const {
  std::ofstream file(filename,std::ios::out);
  try { 
    if ( !file ) {
      std::string err_str = "Error opening file " +filename;
      throw EXCEPTION(err_str,ERRDIV);
    }
    this->dump(file);

    file.close();
  }
  catch (Exception& e) {
    std::string err_str = "Abording writing\n";
    err_str += "File " + filename + "might be wrong, incomplete, or corrupted.";
    e.ADD(err_str,ERRDIV);
    throw e;
  }
  catch (...) {
    std::string err_str = "Something went wrong with the stream.\n";
    err_str += "File " + filename + "might be wrong, incomplete, or corrupted.";
    throw EXCEPTION(err_str, ERRDIV);
  }
}


//
void Dtset::cif(const std::string& filename, const double tolerance) {
  std::ofstream file(filename,std::ios::out);
  if ( !file ) {
    std::string err_str = "Error opening file " +filename;
    throw EXCEPTION(err_str,ERRDIV);
  }
  try { 
    this->cif(file, tolerance);

    file.close();
  }
  catch (Exception& e) {
    std::string err_str = "Abording conversion to CIF\n";
    err_str += "File " + filename + " might be wrong, incomplete, or corrupted.";
    e.ADD(err_str,ERRDIV);
    throw e;
  }
  catch (...) {
    std::string err_str = "Something went wrong.\n";
    err_str += "File " + filename + " might be wrong, incomplete, or corrupted.";
    throw EXCEPTION(err_str, ERRDIV);
  }
}


//
void Dtset::cif(std::ostream& out, const double tolerance) {
  try {
    Findsym fsym;
    fsym.tolerance(tolerance*phys::b2A);
    {
      using namespace geometry;
      fsym.rprim(_rprim*phys::b2A);
    }
    std::vector<int> znuclat(_typat.size());
    bool meaningfull = false;
    if ( _znucl.size() != 0 ) {
      for ( unsigned iatom = 0 ; iatom < _typat.size() ; ++iatom )
        znuclat[iatom] = _znucl[_typat[iatom]-1]; // _typat starts at 1;
      meaningfull = true;
    }
    else
      znuclat = _typat;
    fsym.natom(_natom);
    fsym.typat(znuclat,meaningfull);
    fsym.xred(_xred);
    fsym.findsym();
    _findsym = fsym.cif();
    out << _findsym;
    out.flush();
  }
  catch (Exception& e) {
    e.ADD("Generation of CIF file failed",ERRDIV);
    throw e;
  }
}


//
void Dtset::setCif(const std::string& cifFile) {
  std::string tmp;
  std::ifstream file;
  file.open(cifFile.c_str(),std::ios::in);
  if ( !file )
    throw EXCEPTION("Unable to open file "+cifFile,ERRDIV);
  while(!file.eof()) {
    std::getline(file,tmp);
    _findsym += tmp+"\n";
  }
  file.close();
}


//
void Dtset::reBuildStructure(const double tolerance, const bool prtcif) {
  CifParser cif;
  std::stringstream cifstream(_findsym);

  //Get the information we need
  try {
    if ( _findsym.empty() ) this->cif(cifstream,tolerance);
    if ( prtcif ) std::clog << cifstream.str();
    cif.parse(cifstream);
  }
  catch (Exception& e) {
    e.ADD("Something went wrong...\nNot able to re-build the structure.",ERRDIV);
    throw e;
  }

  //CifParser::DataBlock block(cif.getDataBlock("findsym-output"));
  unsigned b = 0;
  for ( b = 0 ; b < cif.ndataBlock() ; ++b ) {
    try {
     cif.getDataBlock(b).getDataLoop("atom_site_fract_x");
     break;
    }
    catch (Exception& e) {
    }
  }

  CifParser::DataBlock block(cif.getDataBlock(b));
  if ( block._dataLoops.size() < 2 )
    throw EXCEPTION("DataBlock from CIF file has not the correct number of loop_",ERRABT);

  CifParser::DataLoop  syms;
  unsigned colsyms = 0;
  try {
    syms = block.getDataLoop("space_group_symop_operation_xyz");
    colsyms = syms.getColumn("space_group_symop_operation_xyz");
  }
  catch ( Exception &e ) {
    try {
      syms = block.getDataLoop("symmetry_equiv_pos_as_xyz");
      colsyms = syms.getColumn("symmetry_equiv_pos_as_xyz");
    }
    catch ( Exception &se ) {
	  e += se;
      se.ADD("Not able to find the symmetry operations",ERRABT);
      throw se;
    }
  }

  // Assume we know atoms are in second loop
  CifParser::DataLoop atoms = block.getDataLoop("atom_site_fract_x");
  unsigned typsymbol, multiplicity, xcoord, ycoord, zcoord;
  try {
    if ( atoms._nfield < 4 ) 
      throw EXCEPTION("Wrong data block",ERRABT);
    try {
      typsymbol = atoms.getColumn("atom_site_type_symbol");
    }
    catch ( Exception& e ) {
      typsymbol = atoms.getColumn("atom_site_label");
    }
    xcoord = atoms.getColumn("atom_site_fract_x");
    ycoord = atoms.getColumn("atom_site_fract_y");
    zcoord = atoms.getColumn("atom_site_fract_z");
  }
  catch (Exception &e){
    e.ADD("Could not get all needed information, abording",ERRABT);
    throw e;
  }
  try{
    multiplicity = atoms.getColumn("atom_site_symmetry_multiplicity");
  }
  catch(Exception &e) {
    multiplicity = (unsigned) -1;
    (void)e;
  }

  std::vector<geometry::vec3d> xred;
  std::vector<int> typat;
  std::vector<std::string> symbol;

  // For all Wyckoff position apply symmetries and reduce coordinates.
  for ( unsigned ientry = 0 ; ientry < atoms._nentry ; ++ientry ) {
    using namespace geometry;
    vec3d coord={{ 
      utils::stod(atoms._data[ientry][xcoord]),
        utils::stod(atoms._data[ientry][ycoord]),
        utils::stod(atoms._data[ientry][zcoord])
    }};
    mat3d rot;
    vec3d trans;
    //std::cout << atoms._data[ientry][1] << " " << atoms._data[ientry][2] << std::endl;
    //print(coord);
    xred.push_back(coord);
    // Find back the symbol
    int type = -1;
    for ( unsigned sp = 0 ; sp < symbol.size() ; ++sp ) {
      std::string specie(symbol[sp]);
      if ( atoms._data[ientry][typsymbol].compare(specie) == 0 ) {
        type = sp+1;
        break;
      }
    }

    if ( type == -1 ) {
      type = symbol.size()+1;
      symbol.push_back(atoms._data[ientry][typsymbol]);
    }
    typat.push_back(type);

    // Apply all symetries
    unsigned begin = xred.size()-1;
    for ( auto& sym : syms._data ) {
      getSymmetry(sym[colsyms],rot,trans);
      vec3d newcoord = (rot*coord)+trans; // New coordinates
      // Look for duplicate
      bool found = false;
      for ( unsigned icomp = begin ; icomp < xred.size() ; ++icomp ){
        auto diff = xred[icomp]-newcoord;
        recenter(diff);
        if ( norm(diff) < tolerance ) {
          found = true;
          break;
        }
      }
      // Add if not found
      if ( !found ) {
        recenter(newcoord);
        for ( unsigned axe = 0 ; axe < 3 ; ++axe ) {
          newcoord[axe] += (newcoord[axe]<0.0 ? +1.0 : 0.0);
        }
        //print(newcoord);
        xred.push_back(newcoord);
        typat.push_back(type);
      }
    }
    if ( multiplicity != (unsigned)-1 && atoms._data[ientry][multiplicity] != "?" && (xred.size()-begin) != (size_t) utils::stoi(atoms._data[ientry][multiplicity]) )
      throw EXCEPTION("Bad multiplicity found",ERRABT);
  }
#ifdef HAVE_SHRINK_TO_FIT
  xred.shrink_to_fit();
  typat.shrink_to_fit();
#endif
  _ntypat = symbol.size();
  // Find symbol if possible
  _znucl.clear();
  for( auto& elmt : symbol ) {
    try{
      _znucl.push_back(Mendeleev::znucl(elmt));
    }
    catch( Exception &e ){
      _znucl.clear();
      break;
      (void)e;
    }
  }
  // Store new values
  _xred = xred ;
  _natom = _xred.size();
  _typat = typat;
  // Construct new rprim and xcart
  try { 
    const double angdeg[3] = { utils::stod(block.getTag("cell_angle_alpha")),
      utils::stod(block.getTag("cell_angle_beta")),
      utils::stod(block.getTag("cell_angle_gamma"))};

    this->buildRprim(angdeg);

    _acell[0] = utils::stod(block.getTag("cell_length_a"))/phys::b2A;
    _acell[1] = utils::stod(block.getTag("cell_length_b"))/phys::b2A;
    _acell[2] = utils::stod(block.getTag("cell_length_c"))/phys::b2A;

    _rprim[geometry::mat3dind(1,1)] *= _acell[0]; // vec1.x
    _rprim[geometry::mat3dind(2,1)] *= _acell[1]; // vec2.x
    _rprim[geometry::mat3dind(3,1)] *= _acell[2]; // vec3.x
    _rprim[geometry::mat3dind(1,2)] *= _acell[0]; // vec1.y
    _rprim[geometry::mat3dind(2,2)] *= _acell[1]; // vec2.y
    _rprim[geometry::mat3dind(3,2)] *= _acell[2]; // vec3.y
    _rprim[geometry::mat3dind(1,3)] *= _acell[0]; // vec1.z
    _rprim[geometry::mat3dind(2,3)] *= _acell[1]; // vec2.z
    _rprim[geometry::mat3dind(3,3)] *= _acell[2]; // vec3.z

    _gprim = geometry::invertTranspose(_rprim);
    geometry::changeBasis(_rprim, _xcart, _xred, false);
  }
  catch (Exception& e) {
    e.ADD("Abording reBuildStructure, rprim misbuilt",ERRABT);
    throw e;
  }
  catch (...) {
    throw EXCEPTION("Abording reBuildStructure, rprim misbuilt",ERRABT);
  }
}

//
void Dtset::buildRprim(const double angdeg[3]) {
  const double torad = phys::pi/180.0;
  const double alpha = angdeg[0]*torad;
  const double beta  = angdeg[1]*torad;
  const double gamma = angdeg[2]*torad;
  const double acella = 1.0e0;
  const double acellb = 1.0e0;
  const double acellc = 1.0e0;
  geometry::mat3d rprim;
  
  rprim[geometry::mat3dind(1,1)] = acella;
  rprim[geometry::mat3dind(1,2)] = 0.0e0;
  rprim[geometry::mat3dind(1,3)] = 0.0e0;

  rprim[geometry::mat3dind(2,1)] = acellb*cos(gamma);
  rprim[geometry::mat3dind(2,2)] = acellb*sin(gamma);
  rprim[geometry::mat3dind(2,3)] = 0.0e0;

  rprim[geometry::mat3dind(3,1)] = acellc*cos(beta);
  rprim[geometry::mat3dind(3,2)] = acellc*(cos(alpha)-cos(gamma)*cos(beta))/sin(gamma);
  rprim[geometry::mat3dind(3,3)] = sqrt(acellc*acellc
      -rprim[geometry::mat3dind(3,1)]*rprim[geometry::mat3dind(3,1)]
      -rprim[geometry::mat3dind(3,2)]*rprim[geometry::mat3dind(3,2)]);
  
  // Set to 0 numbers smallers than 1e-12
  for ( unsigned i = 0 ; i < 9 ; ++i )
    if ( std::abs(rprim[i]) < 1e-12 )
      rprim[i] = 0.0e0;

  if ( geometry::det(rprim) <=0 )
    throw EXCEPTION("New rprim is not direct!",ERRDIV);
  _rprim = rprim;
}

//
void Dtset::standardizeCell(const bool primitive, const double tolerance) {
#ifdef HAVE_SPGLIB
  _xred.resize(4*_natom);
  _typat.resize(4*_natom);
  double (*lattice)[3] = (double(*)[3]) &_rprim[0];
  double (*positions)[3] = (double(*)[3]) &_xred[0];

  int natom = spg_standardize_cell(lattice,positions,&_typat[0],_natom,(primitive?1:0),0,tolerance);
  if ( natom != 0 ) {
    _natom = natom;
    _xred.resize(natom);
#ifdef HAVE_SHRINK_TO_FIT
    _xred.shrink_to_fit();
#endif
    for ( auto& xred : _xred ) {
      for ( int c = 0 ; c < 3 ; ++c ) {
        if ( std::abs(1.-xred[c]) <= tolerance ) xred[c] -= 1;
      }
    }
    _typat.resize(natom);
    if (!_spinat.empty()) _spinat.resize(natom);
#ifdef HAVE_SHRINK_TO_FIT
    _typat.shrink_to_fit();
    _spinat.shrink_to_fit();
#endif
    _gprim = geometry::invertTranspose(_rprim);
    geometry::changeBasis(_rprim, _xcart, _xred, false);
  }
#ifdef HAVE_SHRINK_TO_FIT
    _xcart.shrink_to_fit();
#endif
#else
    if ( primitive ) {
      Exception e = EXCEPTION("Primitive cell can only be built with spglib. You get a standard cell instead",ERRWAR);
      std::clog << e.fullWhat() << std::endl;
    }
    this->reBuildStructure(tolerance,false);
#endif
}

void Dtset::clearSpinat() {
  for (auto& spin : _spinat)
    for (int s = 0 ; s < 3 ; ++s ) spin[s] = 0;
}

void Dtset::getSymmetries(std::vector<geometry::mat3d> &rotations, std::vector<geometry::vec3d> &translations, double symprec) const {
#ifdef HAVE_SPGLIB
  double (*lattice)[3] = (double(*)[3]) &_rprim[0];
  double *positions = new double[_natom*3];
  for ( unsigned i = 0 ; i < _natom ; ++i )
    for ( unsigned j = 0 ; j < 3; ++j )
      positions[i*3+j] = _xred[i][j];
  const int max_size = 192;
  int *rot = new int[192*3*3];
  double *tnon = new double[192*3];

  int nsym = spg_get_symmetry(
      (int(*)[3][3])rot,
      (double(*)[3])tnon,
      max_size,
      lattice,
      (double(*)[3])positions,
      &_typat[0],
      _natom,
      symprec);
  rotations.resize(nsym);
  translations.resize(nsym);
  for ( int isym = 0 ; isym < nsym ; ++isym ){
    auto irot = rotations.begin()+isym;
    auto itrans = translations.begin()+isym;
    for ( int ii = 0 ; ii < 3 ; ++ii ) {
      itrans->at(ii) = tnon[isym*3+ii];
      for ( int jj = 0 ; jj < 3 ; ++jj ) {
        irot->at(ii*3+jj) = rot[isym*9+ii*3+jj];
      }
    }
  }
  delete [] positions;
  delete [] rot;
  delete [] tnon;
#else
  throw EXCEPTION("spglib needed",ERRABT);
#endif
}


std::array<double,6> Dtset::getStrain(const Dtset& reference) const {
  using namespace geometry;
  mat3d gprim = invert(reference.rprim());
  mat3d strain = _rprim*gprim;
  return {strain[mat3dind(1,1)]-1,strain[mat3dind(2,2)]-1,strain[mat3dind(3,3)]-1,strain[mat3dind(3,2)],strain[mat3dind(3,1)],strain[mat3dind(2,1)]};
}


bool Dtset::operator==(const Dtset& dtset1) const {

  if ( dtset1._natom != _natom ) return false;

  for ( unsigned r = 0 ; r < 9 ; ++r ) {
    if ( std::abs(dtset1._rprim[r]-_rprim[r]) > 1e-10 ) return false;
  }

  for ( unsigned i = 0 ; i < dtset1._natom ; ++i ) {
    if ( dtset1._znucl[dtset1._typat[i]-1] != _znucl[_typat[i]-1] ) return false;
    if ( !(dtset1._xred[i] == _xred[i]) ) return false;
  }
  return true;
}

/**
 * @file src/eigparser.cpp
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


#include "io/eigparser.hpp"
#include "io/eigparsereig.hpp"
#include "io/eigparserphfrq.hpp"
#include "io/eigparserphonopy.hpp"
#include "io/eigparserphbst.hpp"
#include "io/eigparserfatbands.hpp"
#include "base/exception.hpp"
#include "base/utils.hpp"
#include "base/phys.hpp"
#include "base/mendeleev.hpp"
#include <iostream>
#include <fstream>
#include <utility>
#include <iomanip>
#ifdef HAVE_NETCDF
#include <netcdf.h>
#endif
#include <memory>

//
EigParser::EigParser() :
  _filename(),
  _kpts(),
  _lengths(),
  _eigens(),
  _nband(-1),
  _eunit(Units::Ha),
  _conversion(1.0),
  _hasSpin(false),
  _ndiv(),
  _labels()
{
  ;
}

//
EigParser::~EigParser() {
  ;
}

//
void EigParser::dump(std::ostream& out, unsigned options, std::vector<unsigned> umask) const {
  try { 
    out << this->dump(options,umask);
  }
  catch ( Exception &e ) {
    e.ADD("Error in dumping",ERRDIV);
    throw e;
  }
  catch (...) {
    throw EXCEPTION("Unknow exception caught", ERRDIV);
  }
}

//
void EigParser::dump(const std::string& filename, unsigned options, std::vector<unsigned> umask) const {
  std::ofstream file(filename,std::ios::out);
  if ( !file ) {
    std::string err_str = "Error opening file " +filename;
    throw EXCEPTION(err_str,ERRDIV);
  }
  try { 
    this->dump(file, options,umask);

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

std::vector<double> EigParser::getBand(const unsigned iband, const double fermi, const unsigned ispin) const { 
  unsigned spin = _hasSpin ? 2 : 1 ;
  unsigned nkpt = _kpts.size()/spin;
  std::vector<double> eigen(nkpt,0);
  if ( (ispin != 1 && ispin != 2) || ispin > spin ) throw EXCEPTION("Bad value for ispin",ERRABT);
  if ( iband < _nband ) {
    for ( unsigned ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
      eigen[ikpt] = _eigens[ikpt+(ispin-1)*nkpt][iband]-fermi*_conversion;
    }
  }
  else
    throw EXCEPTION("Out of range",ERRDIV);
  return eigen;
}

//
void EigParser::setUnit(Units::Energy u) {
  if ( u == _eunit ) return;
  double factor = Units::getFactor(_eunit,u);
  for( auto& eigk : _eigens ) 
    for ( auto& band : eigk )
      band *= factor;
  _eunit = u;
  _conversion = factor;
}

//
EigParser* EigParser::getEigParser(const std::string& file){
  EigParser *eigparser = nullptr;
  Exception eloc;
  std::vector<std::pair<std::unique_ptr<EigParser>,std::string> > allFormat;

  allFormat.push_back(std::make_pair(std::unique_ptr<EigParser>(new EigParserEIG),"Abinit _EIG")); //0
  allFormat.push_back(std::make_pair(std::unique_ptr<EigParser>(new EigParserPHFRQ),"Abinit PHFRQ"));   //1
  allFormat.push_back(std::make_pair(std::unique_ptr<EigParser>(new EigParserPhonopy),"Phonopy band YAML"));   //2
  allFormat.push_back(std::make_pair(std::unique_ptr<EigParser>(new EigParserPHBST),"Abinit _PHBST")); //3
  allFormat.push_back(std::make_pair(std::unique_ptr<EigParser>(new EigParserFatbands),"Abinit _FATBANDS")); //4

  if ( file.find(".yaml") != std::string::npos ) allFormat[0].swap(allFormat[2]);
  if ( file.find("PHFRQ") != std::string::npos ) allFormat[0].swap(allFormat[1]);
  if ( file.find("_PHBST") != std::string::npos ) allFormat[0].swap(allFormat[3]);
  if ( file.find("_FATBANDS") != std::string::npos ) allFormat[0].swap(allFormat[4]);

  for ( auto& p : allFormat ) {
    try {
      p.first->readFromFile(file);
      eigparser = p.first.release();
    }
    catch (Exception &e) {
      eigparser = nullptr;
      eloc += e;
      eloc.ADD("Format is not "+p.second,ERRDIV);
      if ( e.getReturnValue() == ERRABT ) {
        break;
      }
    }
    if ( eigparser != nullptr ) {
      std::clog << "Format is "+p.second << std::endl;
      eigparser->_filename = file;
      return eigparser;
    }
  }

  eloc.ADD("Failed to build the EigParser",ERRDIV);
  throw eloc;
  return nullptr;
}

//
//
std::string EigParser::dump(unsigned options, std::vector<unsigned> umask) const {
  std::ostringstream str;
  unsigned nkpt = _kpts.size();
  unsigned nspin = ( _hasSpin ? 2 : 1 );
  const unsigned w=16;

  if ( _hasSpin ) {
    nkpt /= 2;
    if ( nkpt*2 != _kpts.size() )
      throw EXCEPTION("Non-consistent data : number of bands different for spin-up and spin-down",ERRABT);
  }

  std::vector<std::vector<unsigned>> projections;
  if ( options & PRTPROJ ) {
    try {
      for ( unsigned ispin = 0 ; ispin < nspin ; ++ispin ) {
        for ( unsigned iband = 0 ; iband < _nband ; ++iband ) {
          projections.push_back(std::move(this->getBandColor(iband,ispin+1,umask))); // ispin should be 1 or 2
        }
      }
    }
    catch ( Exception &e ) {
      e.ADD("Not able to dump with projections",ERRDIV);
      throw e;
    }
  }


  //Impose right alignment with 5 digit precision as floating point numbers.
  str.setf(std::ios::right,std::ios::adjustfield);
  str.setf(std::ios::fixed,std::ios::floatfield);
  str.precision(8);
  //Write header

  str << "#";
  if ( options & PRTIKPT )
    str << std::setw(w) << "ikpt";
  if ( options & PRTKPT ) {
    str << std::setw(w) << "kx";
    str << std::setw(w) << "ky";
    str << std::setw(w) << "kz";
  }
  str << std::setw(w) << "length";
  for ( unsigned ispin = 0 ; ispin < nspin ; ++ispin ) {
    std::string prefix = ( _hasSpin ? ( ispin == 0 ? "band-up " : "band-down " ) : "band " );
    for ( unsigned i = 1; i <= _nband ; ++i ) 
      str << std::setw(w) << prefix+utils::to_string(i);
  }
  if ( options & PRTPROJ ) {
    for ( unsigned ispin = 0 ; ispin < nspin ; ++ispin ) {
      std::string prefix = ( _hasSpin ? ( ispin == 0 ? "color-up " : "color-down " ) : "color " );
      for ( unsigned i = 1; i <= _nband ; ++i ) 
        str << std::setw(w) << prefix+utils::to_string(i);
    }
  }
  str << std::endl;

  for ( unsigned ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
    str << " ";
    if ( options & PRTIKPT )
      str << std::setw(w) << ikpt+1;
    if ( options & PRTKPT ) {
      str << std::setw(w) << _kpts[ikpt][0];
      str << std::setw(w) << _kpts[ikpt][1];
      str << std::setw(w) << _kpts[ikpt][2];
    }
    str << std::setw(w) << _lengths[ikpt];
    for ( unsigned ispin = 0 ; ispin < nspin ; ++ispin ) {
      for ( unsigned i = 0; i < _nband ; ++i ) 
        str << std::setw(w) << _eigens[ikpt+ispin*nkpt][i];
    }
    if ( options & PRTPROJ ) {
      for ( unsigned ispin = 0 ; ispin < nspin ; ++ispin ) {
        for ( unsigned i = 0; i < _nband ; ++i ) {
          str << std::setw(w) << projections[ispin*_nband+i][ikpt];
        }
      }
    }
    str << std::endl;
  }
  return str.str();
}



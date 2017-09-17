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
#include "base/exception.hpp"
#include "base/utils.hpp"
#include "base/phys.hpp"
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
  _eunit(Ha),
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
void EigParser::dump(std::ostream& out, unsigned options) const {
  try { 
    out << this->dump(options);
  }
  catch (...) {
    throw EXCEPTION("Something went wrong during the dumping of EigenParser", ERRDIV);
  }
}

//
void EigParser::dump(const std::string& filename, unsigned options) const {
  std::ofstream file(filename,std::ios::out);
  if ( !file ) {
    std::string err_str = "Error opening file " +filename;
    throw EXCEPTION(err_str,ERRDIV);
  }
  try { 
    this->dump(file, options);

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
void EigParser::setUnit(Unit u) {
  using namespace phys;
  double factor = 1.e0;
  if ( u == _eunit ) return;
  // Convert to eV
  switch (_eunit) {
    case eV :
       break;
    case Ha :
      factor = Ha2eV;
      break;
    case THz :
      factor = 1e12 * Hz2eV;
      break;
    case pcm :
      factor = 1e-2 * m2eV; // m-> ev = m2eV/input 
      break;
    default :
      throw EXCEPTION("Unit not yet implemented",ERRDIV);
      break;
  }
  switch (u) {
    case eV :
      break;
    case Ha :
      factor /= Ha2eV;
      break;
    case THz :
      factor /= (1e12 * Hz2eV);
      break;
    case pcm :
      factor /= (1e2 * m2eV);
      break;
    default :
      throw EXCEPTION("Unit not yet implemented",ERRDIV);
      break;
  }
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

  if ( file.find(".yaml") != std::string::npos ) allFormat[0].swap(allFormat[2]);
  if ( file.find("PHFRQ") != std::string::npos ) allFormat[0].swap(allFormat[1]);
  if ( file.find("_PHBST") != std::string::npos ) allFormat[0].swap(allFormat[3]);

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
      return eigparser;
    }
  }

  eloc.ADD("Failed to build the EigParser",ERRDIV);
  throw eloc;
  return nullptr;
}

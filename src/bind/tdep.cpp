/**
 * @file tdep.cpp
 *
 * @brief Interface to the tdep utility of Abinit
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


#include "bind/tdep.hpp"
#include "base/exception.hpp"
#include "base/mendeleev.hpp"
#include "hist/histdatanc.hpp"
#include "hist/histdatadtset.hpp"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cmath>

Tdep::Tdep() :
  _mode(Normal),
  _unitcell(),
  _bravais(1),
  _centering(0),
  _supercell(),
  _tbegin(0),
  _tend(-1),
  _step(1),
  _rcut(-1),
  _multiplicity()
{
#ifndef HAVE_TDEP
  //throw EXCEPTION("TDEP support not enabled",ERRABT);
#endif
}


Tdep::~Tdep() {
  _supercell.release();
}

void Tdep::mode(Mode m) {
  _mode = m;
}

void Tdep::unitcell(Dtset uc) {
  _unitcell = uc;
  std::string name;
  HistDataDtset hist(_unitcell);
  unsigned spgnum = hist.getSpgNum(0,0.01,&name);
  if ( spgnum >= 1 && spgnum <= 2 )
    _bravais = 1;
  else if ( spgnum >= 3 && spgnum <= 15 )
    _bravais = 2;
  else if ( spgnum >= 16 && spgnum <= 74 )
    _bravais = 3;
  else if ( spgnum >= 75 && spgnum <= 142 )
    _bravais = 4;
  else if ( spgnum >= 143 && spgnum <= 167 )
    _bravais = 5;
  else if ( spgnum >= 168 && spgnum <= 194 )
    _bravais = 6;
  else if ( spgnum >= 195 && spgnum <= 230 )
    _bravais = 7;
  else
    throw EXCEPTION("Bad space group number",ERRDIV);

  utils::ltrim(name);
  switch ( name[0] ) {
    case 'P' :
      _centering = 0;
      break;
    case 'I' :
      _centering = -1;
      break;
    case 'F' :
      _centering = -3;
      break;
    case 'A' :
      _centering = 1;
      break;
    case 'B' :
      _centering = 2;
      break;
    case 'C' :
      _centering = 3;
      break;
    default :
      throw EXCEPTION("Unable to find centering of bravais lattice with spacegroup "+name,ERRDIV);
  }
}

void Tdep::unitcell(HistData* hist, unsigned time) {
  if ( time < hist->ntimeAvail() ) {
    this->unitcell(Dtset(*hist,time));
  }
  else
    throw EXCEPTION("time value is out of range",ERRDIV);
}

void Tdep::computeMultiplicity() {
  using namespace geometry;
  mat3d rprimdUc = _unitcell.rprim();
  mat3d rprimdSc;
  const double *tmp = _supercell->getRprimd(0);
  for ( unsigned i = 0 ; i < 9 ; ++i )
    rprimdSc[i] = tmp[i];
  mat3d passage = geometry::invert(rprimdUc);
  _multiplicity = rprimdSc * passage;
}

void Tdep::supercell(HistDataMD* hist) {
  try {
    using namespace geometry;
    Dtset first(*hist,0); 
    first.standardizeCell(true,0.01);
    this->unitcell(first);
    _supercell.reset(hist);
  }
  catch ( Exception &e ) {
    e.ADD("Unitcell cannot be automatically found. Make sure to set it manually with keyword unitcell FILENAME",ERRDIV);
    _supercell.reset(hist);
    throw e;
  }
}

void Tdep::tbegin(unsigned t) {
  _tbegin = t;
}

void Tdep::tend(unsigned t) {
  _tend = t;
}

void Tdep::step(unsigned istep) {
  _step = istep;
}

void Tdep::rcut(double r) {
  _rcut = r;
}

void Tdep::multiplicity(geometry::mat3d m) {
  double det = geometry::det(m);
  if ( det <=0 ) 
    throw EXCEPTION("Bad multiplicity matrix",ERRDIV);
  _multiplicity = m;
}

void Tdep::tdep() {
  // Make some checks first
  if ( _supercell == nullptr ) {
    throw EXCEPTION("Need a HistData",ERRDIV);

  }
  if ( _tbegin > _tend ) 
    throw EXCEPTION("tbegin cannot be larger than tend",ERRDIV);
  else if ( _tbegin > _supercell->ntimeAvail() ) 
    throw EXCEPTION("tbegin is too large",ERRDIV);
  else if ( _tend > _supercell->ntimeAvail() ) 
    throw EXCEPTION("tend is too large",ERRDIV);

  auto rprimd = _supercell->getRprimd(0);
  double small = geometry::getWignerSeitzRadius(rprimd);

  if ( _rcut >= 0 &&  _rcut*_rcut < small ) {
    std::ostringstream mess;
    mess << "Rcut seems to be larger than a safe value which is ";
    mess << small << " bohr.\nPlease make a check.";
    Exception e = EXCEPTION(mess.str(),ERRWAR);
    std::clog << e.fullWhat() << std::endl;
  }
  else if ( _rcut < 0. )
    _rcut = small;

  unsigned natomUc = _unitcell.natom();
  //unsigned natomSc = _supercell->natom();

  double temperature;
  std::stringstream thermo;
  _supercell->printThermo(_tbegin,_tend,thermo);
  std::string line;
  std::getline(thermo,line);
  std::getline(thermo,line);
  std::getline(thermo,line);
  std::getline(thermo,line);
  std::getline(thermo,line);
  thermo >> line >> line >> temperature;

  std::ofstream input("input.in",std::ios::out);
  if ( !input )
    throw EXCEPTION("Unable to write input file",ERRDIV);

  switch (_mode) {
    case Normal :
      input << std::setw(16) << "NormalMode" << std::endl;
      break;
    case Debug :
      input << std::setw(16) <<  "DebugMode" << std::endl;
      break;
  }

  input << std::setw(16) << "# Unit cell definition" << std::endl;
  input << std::setw(16) << "brav";
  input << std::setw(4) << _bravais << std::setw(4) << _centering << std::endl;
  if ( _bravais == 1 ) {
    auto angles = geometry::angle(_unitcell.rprim());
    input.precision(9);
    input.setf(std::ios::scientific,std::ios::floatfield);
    input << std::setw(16) << "angle" << std::setw(18) << angles[0] << std::setw(18) << angles[1] << std::setw(18) << angles[2] << std::endl;
  }
  else if ( _bravais == 2 ) {
    auto angles = geometry::angle(_unitcell.rprim());
    input.precision(9);
    input.setf(std::ios::scientific,std::ios::floatfield);
    input << std::setw(16) << "angle" << std::setw(18) << angles[1] << std::endl;
  }

  input << std::setw(16) << "natom_unitcell";
  input << std::setw(14) << natomUc << std::endl;

  input << std::setw(16) << "xred_unitcell";
  input.precision(9);
  input.setf(std::ios::scientific,std::ios::floatfield);
  for ( auto& coord : _unitcell.xred() ) 
    input << std::setw(18) << coord[0] << std::setw(17) << coord[1] << std::setw(17) << coord[2];
  input << std::endl;
  input.setf(std::ios::fixed,std::ios::floatfield);

  input << std::setw(16) << "typat_unitcell";
  for ( auto t : _unitcell.typat() )
    input << std::setw(4) << t;
  input << std::endl;

  /*
     input << std::setw(16) << "ntypat";
     input << std::setw(4) << _unitcell.znucl().size() << std::endl;

     input << std::setw(16) << "amu";
     for ( auto z : _unitcell.znucl() )
     input << std::setw(25) << mendeleev::mass[z];
     input << std::endl;
     */

  input << std::setw(16) << "# Supercell definition" << std::endl;

  /*
     input << std::setw(16) << "rprimd";
     auto rprimdS = _supercell->getRprimd(0);
     input << std::setw(10) << rprimdS[0];
     input << std::setw(10) << rprimdS[3];
     input << std::setw(10) << rprimdS[6];
     input << std::setw(10) << rprimdS[1];
     input << std::setw(10) << rprimdS[4];
     input << std::setw(10) << rprimdS[7];
     input << std::setw(10) << rprimdS[2];
     input << std::setw(10) << rprimdS[5];
     input << std::setw(10) << rprimdS[8];
     input << std::endl;
     */

  this->computeMultiplicity();
  input << std::setw(16) << "multiplicity";
  input.precision(2);
  input.setf(std::ios::fixed,std::ios::floatfield);
  input << std::setw(7) << _multiplicity[0];
  input << std::setw(7) << _multiplicity[3];
  input << std::setw(7) << _multiplicity[6];
  input << std::setw(7) << _multiplicity[1];
  input << std::setw(7) << _multiplicity[4];
  input << std::setw(7) << _multiplicity[7];
  input << std::setw(7) << _multiplicity[2];
  input << std::setw(7) << _multiplicity[5];
  input << std::setw(7) << _multiplicity[8];
  input << std::endl;

  /*
     input << std::setw(16) << "natom";
     input << std::setw(4) << natomSc << std::endl;

     input << std::setw(16) << "typat";
     for ( auto t : _supercell->typat() )
     input << std::setw(4) << t;
     input << std::endl;
     */

  input << std::setw(16) << "temperature";
  input.precision(2);
  input << std::setw(10) << temperature << std::endl;

  input << std::setw(16) << "# Computation details" << std::endl;
  input << std::setw(16) << "nstep_max";
  input << std::setw(10) << (_tend-_tbegin)/_step + ((_tend-_tbegin)%_step != 0 ? 1 : 0 ) << std::endl; // _tend will be included

  input << std::setw(16) << "nstep_min";
  input << std::setw(10) << 1 << std::endl; // Add 1 since it starts at 1

  input << std::setw(16) << "Rcut";
  input.precision(3);
  input << std::setw(10) << std::floor(_rcut*1000.)/1000. << std::endl;

  input << std::setw(16) << "# Optional inputs" << std::endl;
  input << std::setw(16) << "TheEnd" << std::endl;

  input.close();
  HistDataNC::dump(*_supercell.get(),"HIST.nc",_tbegin,_tend,_step);

  int err = system("tdep");
  if ( err != 0 || errno != 0) 
    throw EXCEPTION("Unable to execute tdep",ERRDIV);
}

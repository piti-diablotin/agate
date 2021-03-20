/**
 * @file tdep.cpp
 *
 * @brief Interface to the tdep utility of Abinit
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


#include "bind/tdep.hpp"
#include "base/exception.hpp"
//#include "base/mendeleev.hpp"
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
  _order(1),
  _rcut(-1),
  _rcut3(-1),
  _dosSmearing(4.5e-6),
  _dosQpt{1,1,1},
  _temperature(-1),
  _idealPositions(true),
  _multiplicity(),
  _outputPrefix("")
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

void Tdep::order(unsigned order) {
  if ( order != 2 && order != 3 )
    throw EXCEPTION("Order must be 2 or 3",ERRDIV);
  _order = order;
}

void Tdep::rcut(double r) {
  _rcut = r;
}

void Tdep::rcut3(double r) {
  _rcut3 = r;
}

void Tdep::temperature(double t) {
  _temperature = t;
}

void Tdep::multiplicity(geometry::mat3d m) {
  double det = geometry::det(m);
  if ( det <=0 ) 
    throw EXCEPTION("Bad multiplicity matrix",ERRDIV);
  _multiplicity = m;
}

void Tdep::idealPositions(bool ideal) {
  _idealPositions = ideal;
}

void Tdep::dosParameters(int qpts[3], double smearing) {
  for ( int i = 0 ; i < 3 ; ++i ) {
    if ( qpts[i] < 1 ) 
      throw EXCEPTION("Bad qpts grid",ERRDIV);
    _dosQpt[i] = qpts[i];
  }

  if ( smearing < 1e-6 ) 
    throw EXCEPTION("Smearing value is < 1e-6", ERRDIV);
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
  double small = geometry::getWignerSeitzRadius(rprimd)-1e-5;

  if ( _rcut >= 0 &&  _rcut > small ) {
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

  if ( _temperature < 0 ) {
    std::stringstream thermo;
    _supercell->printThermo(_tbegin,_tend,thermo);
    std::string line;
    std::getline(thermo,line);
    std::getline(thermo,line);
    std::getline(thermo,line);
    std::getline(thermo,line);
    std::getline(thermo,line);
    thermo >> line >> line >> _temperature;
  }

  std::ofstream input("input.in",std::ios::out);
  if ( !input )
    throw EXCEPTION("Unable to write input file",ERRDIV);

  switch (_mode) {
    case Normal :
      input << std::setw(20) << "NormalMode" << std::endl;
      break;
    case Debug :
      input << std::setw(20) <<  "DebugMode" << std::endl;
      break;
  }

  input << std::setw(20) << "# Unit cell definition" << std::endl;
  input << std::setw(20) << "brav";
  input << std::setw(4) << _bravais << std::setw(4) << _centering << std::endl;
  if ( _bravais == 1 ) {
    auto angles = geometry::angle(_unitcell.rprim());
    input.precision(9);
    input.setf(std::ios::scientific,std::ios::floatfield);
    input << std::setw(20) << "angle" << std::setw(18) << angles[0] << std::setw(18) << angles[1] << std::setw(18) << angles[2] << std::endl;
  }
  else if ( _bravais == 2 ) {
    auto angles = geometry::angle(_unitcell.rprim());
    input.precision(9);
    input.setf(std::ios::scientific,std::ios::floatfield);
    input << std::setw(20) << "angle" << std::setw(18) << angles[1] << std::endl;
  }

  input << std::setw(20) << "natom_unitcell";
  input << std::setw(14) << natomUc << std::endl;

  input << std::setw(20) << "xred_unitcell";
  input.precision(9);
  input.setf(std::ios::scientific,std::ios::floatfield);
  for ( auto& coord : _unitcell.xred() ) 
    input << std::setw(18) << coord[0] << std::setw(17) << coord[1] << std::setw(17) << coord[2];
  input << std::endl;
  input.setf(std::ios::fixed,std::ios::floatfield);

  input << std::setw(20) << "typat_unitcell";
  for ( auto t : _unitcell.typat() )
    input << std::setw(4) << t;
  input << std::endl;

  /*
     input << std::setw(20) << "ntypat";
     input << std::setw(4) << _unitcell.znucl().size() << std::endl;

     input << std::setw(20) << "amu";
     for ( auto z : _unitcell.znucl() )
     input << std::setw(25) << MendeTable.mass[z];
     input << std::endl;
     */

  input << std::setw(20) << "# Supercell definition" << std::endl;

  /*
     input << std::setw(20) << "rprimd";
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
  input << std::setw(20) << "multiplicity";
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
     input << std::setw(20) << "natom";
     input << std::setw(4) << natomSc << std::endl;

     input << std::setw(20) << "typat";
     for ( auto t : _supercell->typat() )
     input << std::setw(4) << t;
     input << std::endl;
     */

  input << std::setw(20) << "temperature";
  input.precision(2);
  input << std::setw(10) << _temperature << std::endl;

  input << std::setw(20) << "# Computation details" << std::endl;
  input << std::setw(20) << "nstep_max";
  //input << std::setw(10) << (_tend-_tbegin)/_step + ((_tend-_tbegin)%_step != 0 ? 1 : 0 ) << std::endl; // _tend will be included
  input << std::setw(10) << _tend+1 << std::endl; // _tend will be included

  input << std::setw(20) << "nstep_min";
  //input << std::setw(10) << 1 << std::endl; // Add 1 since it starts at 1
  input << std::setw(10) << _tbegin+1 << std::endl; // Add 1 since it starts at 1

  input << std::setw(20) << "Rcut";
  input.precision(3);
  input << std::setw(10) << std::floor(_rcut*1000.)/1000. << std::endl;

  input << std::setw(20) << "# Optional inputs" << std::endl;
  input << std::setw(20) << "Slice";
  input << std::setw(10) << _step << std::endl; // Add 1 since it starts at 1
  input << std::setw(20) << "Ngqpt2";
  std::stringstream qpttmp;
  qpttmp << _dosQpt[0] << " " << _dosQpt[1] << " " << _dosQpt[2] << std::endl;
  input << std::setw(10) << qpttmp.str();
  input << std::setw(20) << "DosDeltae";
  input.precision(6);
  input << std::setw(10) << _dosSmearing << std::endl;

  input << std::setw(20) << "Use_Ideal_Positions";
  input << std::setw(10) << (_idealPositions ? 1 : 0) << std::endl;
  
  if ( _order == 3 ) {
    input << std::setw(20) << "Order";
    input << std::setw(10) << "3";
    input << std::setw(10) << std::floor((_rcut3 > 0 ? _rcut3 : _rcut)*1000.)/1000. << std::endl;
  }

  input << std::setw(20) << "TheEnd" << std::endl;

  input.close();
  /*
  HistDataNC::dump(*_supercell.get(),"HIST.nc",_tbegin,_tend,_step);
  try {
    if ( _tbegin != 0 ) {
      HistDataNC::dump(*_supercell.get(),"first.nc",0,1,1);
      HistDataNC first;
      HistDataNC traj;
      first.readFromFile("first.nc");
      traj.readFromFile("HIST.nc");
      first += traj;
      first.dump("HIST.nc",0,traj.ntimeAvail()+1);
      remove("first.nc");
    }
  }
  catch( Exception &e ) {
    e.ADD("First time step migth be missing",ERRWAR);
    std::cerr << e.fullWhat() << std::endl;
  }
  */


  //int err = system("tdep <<< \"input.in \nHIST.nc\"");
  FILE *tdep;
  tdep = popen("atdep","w");
  if ( tdep == nullptr ) 
    throw EXCEPTION("Unable to open pipe for tdep",ERRDIV);

  _outputPrefix = utils::to_string(_temperature)+"K";
  std::string files("input.in\n"+_supercell->filename()+"\n"+_outputPrefix+"\n");
  fputs(files.c_str(),tdep);
  int st = pclose(tdep);
#ifndef _WIN32
  if ( WIFEXITED(st) && WEXITSTATUS(st) != 0 ) {
    throw EXCEPTION("Unable to execute tdep",ERRDIV);
  }
#endif
}

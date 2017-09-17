/**
 * @file src/dispdb.cpp
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


#include <fstream>
#include <sstream>
#include <iostream>
#include <complex>
#include <algorithm>
#include "phonons/dispdb.hpp"
#include "base/exception.hpp"
#include "io/configparser.hpp"
#include "base/utils.hpp"
#include "io/ddb.hpp"
#include "phonons/phononmode.hpp"

//
DispDB::DispDB() :
  _natom(-1),
  _nqpt(0),
  _nmode(0),
  _iqpt(),
  _qpts(),
  _modes(),
  _energies()
{
  _iqpt = _qpts.end();
}

//
DispDB::DispDB(unsigned natom) :
  _natom(natom),
  _nqpt(0),
  _nmode(3*_natom),
  _iqpt(),
  _qpts(),
  _modes(),
  _energies()
{
  _iqpt = _qpts.end();
}

//
DispDB::~DispDB() {
  ;
}

//
void DispDB::clear() {
  _qpts.clear();
  _modes.clear();
  _energies.clear();
}

//
void DispDB::readFromFile(std::string filename, unsigned natom) {
  if ( natom == (unsigned) -1 && _natom == (unsigned)-1 )
    throw EXCEPTION("Need to know how many atoms you have",ERRDIV);
  if ( natom != (unsigned) -1 ) _natom = natom;

  std::ifstream anaddb(filename,std::ios::in);
  if ( !anaddb )
    throw EXCEPTION("Unable to open file "+filename,ERRDIV);

  std::string line;
  unsigned bline = 0;
  do {
    std::getline(anaddb,line);
    ++bline;
  } while( anaddb.good() && line.find("outvars_anaddb") == std::string::npos );

  if ( !anaddb )
    throw EXCEPTION("File "+filename+" ended before starting ...",ERRDIV);

  std::getline(anaddb,line);
  unsigned eline = ++bline;
  std::string content;
  while( anaddb.good() && line.find("========================================") == std::string::npos ) {
    std::getline(anaddb,line);
    content += line;
    ++eline;
  }

  if ( !anaddb )
    throw EXCEPTION("File "+filename+" ended before starting ...",ERRDIV);

  content.resize(content.size()-line.size());
  --eline;

  std::clog << "Input variables read from line " << bline << " to line " << eline << " of file " << filename << std::endl;
  ++eline;
  {
    ConfigParser parser;
    parser.setFile(filename);
    parser.setContent(content);
    unsigned eivec;
    if ( (eivec = parser.getToken<unsigned>("eivec")) != 1 && eivec != 2 ) 
      throw EXCEPTION("eivec should be equal to 1 or 2",ERRABT);

    try {
      _nqpt = parser.getToken<unsigned>("nph1l");
      //_qpts.resize(_nqpt); // Don't resize qpts, it will be a check to know if we have read all information or not
      _nmode = 3*_natom;
      _modes.resize(_nqpt*_nmode*3*_natom); // nqpt * nmode * 1vector per atom
      _energies.resize(_nqpt*_nmode);
    }
    catch ( Exception &e ){
      e.ADD("Input variable nph1l is mandatory",e.getReturnValue());
      throw e;
    }
  }

  do {
    std::getline(anaddb,line);
    ++eline;
  } while ( anaddb.good() && line.find("Treat the first list of vectors") == std::string::npos );

  int iqpt = -1;
  bool inqpt = false;
  int imode = -1;
  bool inmode = false;
  int idisp = -1;
  bool vecreal = false;
  int iatom = -1;
  while ( !anaddb.eof() ) {
    std::getline(anaddb,line);
    ++eline;
    if ( line.find("Phonon wavevector") != std::string::npos ) {
      std::istringstream explode(line);
      std::vector<std::string> fields;
      std::string tmp;
      do {
        explode >> tmp;
        fields.push_back(std::move(tmp));
      } while (explode.good());
      if ( fields.size() < 8 )
        throw EXCEPTION("Bad line "+utils::to_string(eline)+line,ERRDIV);
      _qpts.push_back({{ utils::stod(fields[5]),
        utils::stod(fields[6]),
        utils::stod(fields[7]) }});
      ++iqpt;
      inqpt = true;
    }
    else if ( line.find("phonon wavelength") != std::string::npos ) {
      Exception e = EXCEPTION(std::string("It seems you are using a log file instead of an output file of anaddb.")
          + std::string(" This is not possible"),ERRWAR);
      std::clog << e.fullWhat();
    }
    else if ( line.find("Mode number") != std::string::npos ) {
      std::string tmp;
      std::istringstream fornrj(line);
      fornrj >> tmp >> tmp;
      int limode;
      fornrj >> limode;
      if ( !inqpt || limode != ++imode+1 )
        throw EXCEPTION("Something weird is happening line "+utils::to_string(eline)+line,ERRDIV);
      fornrj >> tmp >> _energies[iqpt*_nmode+imode];
      inmode = true;
      idisp = 0;
      vecreal = true;
    }
    else if ( inmode && ( line[0] == '-' || line[0] == ';') ) {
      std::istringstream explode(line);
      std::vector<std::string> fields;
      std::string tmp;
      do {
        explode >> tmp;
        fields.push_back(std::move(tmp));
      } while (explode.good());
      if ( vecreal &&  fields.size() == 5 ) {
        iatom = utils::stoi(fields[1]);
        if ( iatom != idisp+1 )
          throw EXCEPTION("Something weird is happening line "+utils::to_string(eline)+line,ERRDIV);
        _modes[iqpt*_nmode*3*_natom+imode*3*_natom+idisp*3  ].real(utils::stod(fields[2]));
        _modes[iqpt*_nmode*3*_natom+imode*3*_natom+idisp*3+1].real(utils::stod(fields[3]));
        _modes[iqpt*_nmode*3*_natom+imode*3*_natom+idisp*3+2].real(utils::stod(fields[4]));
        vecreal = false;
      }
      else if ( !vecreal && fields.size() == 4 ) {
        // Should be the imaginary part that we ignore so next line should be real
        if ( iatom != idisp+1 )
          throw EXCEPTION("Something weird is happening line "+utils::to_string(eline)+line,ERRDIV);
        _modes[iqpt*_nmode*3*_natom+imode*3*_natom+idisp*3  ].imag(utils::stod(fields[1]));
        _modes[iqpt*_nmode*3*_natom+imode*3*_natom+idisp*3+1].imag(utils::stod(fields[2]));
        _modes[iqpt*_nmode*3*_natom+imode*3*_natom+idisp*3+2].imag(utils::stod(fields[3]));
        vecreal = true;
        if ( (unsigned) ++idisp == _natom ) { 
          idisp = -1;
          inmode = false;
          if ( (unsigned) imode == _nmode-1 ) {
            imode = -1;
            if ( (unsigned) iqpt == _nqpt -1 ) break;
          }
        }
      }
    }
  }
  anaddb.close();
  /*
  for ( iqpt = 0 ; iqpt < _nqpt ; ++iqpt ) {
    using std::cout;
    using std::endl;
    cout << "Qpt-" << iqpt << "\t" << _qpts[iqpt][0] << "\t" << _qpts[iqpt][1] << "\t" << _qpts[iqpt][2] << endl;
    for ( imode = 0 ; imode < _nmode ; ++ imode ) {
      cout << "  Mode-" << imode << "\t" << _energies[iqpt*_nmode+imode] << endl;
      for ( idisp = 0 ; idisp < _natom ; ++idisp ) {
        cout << "    " << idisp << "\t" << _modes[iqpt*_nmode*3*_natom+imode*3*_natom+idisp*3  ] << "\t"
          << "\t" << _modes[iqpt*_nmode*3*_natom+imode*3*_natom+idisp*3+1] << "\t"
          << "\t" << _modes[iqpt*_nmode*3*_natom+imode*3*_natom+idisp*3+2] << endl;
      }
    }
  }
  //*/
  if ( _nqpt != _qpts.size() ) {
    Exception e = EXCEPTION(std::string("It seems some qpt are missing : have ")
        +utils::to_string(_qpts.size())+std::string(" qpt instead of ")+utils::to_string(_nqpt),ERRWAR);
    std::clog << e.fullWhat();
    _nqpt = _qpts.size();
    _modes.resize(_nqpt*_nmode*3*_natom); // nqpt * nmode * 1vector per atom
    _energies.resize(_nqpt*_nmode);

  }
  _iqpt = _qpts.end();
}

void DispDB::computeFromDDB(Ddb &ddb) {
  unsigned natom = ddb.natom();
  if ( _natom != (unsigned) -1 && _natom != natom )
    throw EXCEPTION("Band number of atoms",ERRABT);
  if ( _natom == (unsigned) -1 ) _natom = natom;

  _qpts = ddb.getQpts();
  _nqpt = _qpts.size();
  //_qpts.resize(_nqpt); // Don't resize qpts, it will be a check to know if we have read all information or not
  _nmode = 3*_natom;
  _modes.resize(_nqpt*_nmode*3*_natom); // nqpt * nmode * 1vector per atom
  _energies.resize(_nqpt*_nmode);

  if ( _nqpt > 0 ) {
    PhononMode modes(natom);
    try { 
      modes.computeASR(ddb);
    }
    catch (Exception &e) {
      e.ADD("ASR cannot be computed but continuing",ERRWAR);
      std::clog << e.fullWhat() << std::endl;
    }
    modes.computeAllEigen(ddb,&_energies[0],&_modes[0]);
  }
  else {
    Exception e = EXCEPTION("There is not qpt in this DDB",ERRWAR);
    std::clog << e.fullWhat() << std::endl;
  }
}


bool DispDB::hasQpt(const vec3d qpt) const {
  using geometry::operator-;
  return _qpts.end() != std::find_if(
      _qpts.begin(),
      _qpts.end(),
      [&qpt](vec3d iqpt) {
      return ( norm(qpt-iqpt) < 1e-6 );
      } 
      );
}

//
void DispDB::setQpt(const vec3d qpt) {
  using geometry::operator-;
  _iqpt = std::find_if(
      _qpts.begin(),
      _qpts.end(),
      [&qpt](vec3d iqpt) {
      return ( norm(qpt-iqpt) < 1e-6 );
      } 
      );
  if ( _iqpt == _qpts.end() )
    throw EXCEPTION("Qpt not found",ERRDIV);
}

//
unsigned DispDB::getQpt(const vec3d qpt) {
  using geometry::operator-;
  auto myqpt = std::find_if(
      _qpts.begin(),
      _qpts.end(),
      [&qpt](vec3d iqpt) {
      return ( norm(qpt-iqpt) < 1e-6 );
      } 
      );
  if ( myqpt == _qpts.end() )
    throw EXCEPTION("Qpt not found",ERRDIV);
  return std::distance(_qpts.begin(),myqpt);
}

//
std::vector<DispDB::cplx>::const_iterator DispDB::getMode(unsigned imode) {
  if ( _iqpt == _qpts.end() )
    throw EXCEPTION("Select first a qpt with setQpt",ERRDIV);
  unsigned dq = std::distance(_qpts.begin(),_iqpt);
  if ( imode >=  _nmode )
    throw EXCEPTION("Mode number "+utils::to_string(imode)+" does not exist",ERRDIV);
  //std::vector<cplx>::iterator pos = _modes.begin()+dq*_nmode*3*_natom+imode*3*_natom;
  return _modes.begin()+dq*_nmode*3*_natom+imode*3*_natom;
}

//
double DispDB::getEnergyMode(unsigned imode) {
  if ( _iqpt == _qpts.end() )
    throw EXCEPTION("Select first a qpt with setQpt",ERRDIV);
  unsigned dq = std::distance(_qpts.begin(),_iqpt);
  if ( imode >=  _nmode )
    throw EXCEPTION("Mode number "+utils::to_string(imode)+" does not exist",ERRDIV);
  return _energies[dq*_nmode+imode];
}

//
std::vector<DispDB::cplx>::const_iterator DispDB::getMode(unsigned dq, unsigned imode) {
  if ( dq >= _qpts.size() )
    throw EXCEPTION("dq is too large",ERRDIV);
  if ( imode >=  _nmode )
    throw EXCEPTION("Mode number "+utils::to_string(imode)+" does not exist",ERRDIV);
  //std::vector<cplx>::iterator pos = _modes.begin()+(dq*_nmode*3*_natom+imode*3*_natom);
  return _modes.begin()+dq*_nmode*3*_natom+imode*3*_natom;
}

//
double DispDB::getEnergyMode(unsigned dq, unsigned imode) {
  if ( dq >= _qpts.size() )
    throw EXCEPTION("dq is too large",ERRDIV);
  if ( imode >=  _nmode )
    throw EXCEPTION("Mode number "+utils::to_string(imode)+" does not exist",ERRDIV);
  return _energies[dq*_nmode+imode];
}

//
DispDB& DispDB::operator += ( const DispDB& disp ) {
  if ( _natom != disp._natom )
    throw EXCEPTION("Number of atoms are different",ERRDIV);
  if ( _nmode != disp._nmode )
    throw EXCEPTION("Number of modes are different",ERRDIV);

  bool setToEnd = ( _iqpt == _qpts.end() );
  bool empty = false;
  unsigned nqpt = _nqpt;
  if ( _nqpt == 0 ) {
    _nqpt = disp._nqpt;
    empty = true;
  }
  else {
    _nqpt += disp._nqpt;
  }

  _qpts.resize(_nqpt);
  _modes.resize(_nqpt*_nmode*3*_natom); // nqpt * nmode * 1vector per atom
  _energies.resize(_nqpt*_nmode);

  if ( empty ) {
    _qpts = disp._qpts;
    _modes = disp._modes;
    _energies = disp._energies;
  }
  else {
    std::copy(disp._qpts.begin(), disp._qpts.end(),_qpts.begin()+nqpt);
    std::copy(disp._modes.begin(), disp._modes.end(),_modes.begin()+nqpt*_nmode*3*_natom);
    std::copy(disp._energies.begin(), disp._energies.end(),_energies.begin()+nqpt*_nmode);
  }
   
  if ( setToEnd ) _iqpt = _qpts.end();

  return *this;
}

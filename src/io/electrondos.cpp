/**
 * @file src/./electrondos.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2019 Jordan Bieder
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


#include "io/electrondos.hpp"
#include "base/exception.hpp"
#include <numeric>

//
unsigned ElectronDos::prtdos() const
{
    return _prtdos;
}

unsigned ElectronDos::atom() const
{
    return _iatom;
}

unsigned ElectronDos::nsppol() const
{
    return _nsppol;
}

double ElectronDos::efermi() const
{
  return _efermi;
}

unsigned ElectronDos::nenergy() const
{
  return _nenergy;
}

std::vector<double> ElectronDos::dos(unsigned isppol, int tsmear) const {
  if ( tsmear != 0 && tsmear != 1 && tsmear != -1 )
    throw EXCEPTION("Bad value for tsmear",ERRDIV);
  if ( _prtdos != 1 && tsmear != 0 )
    throw EXCEPTION("tsmear != 0 not allowed for prtdos 1",ERRDIV);
  if ( isppol<1 && isppol >_nsppol )
    throw EXCEPTION("Bad value for isppol",ERRDIV);
  isppol--; // isppol should be 0 or 1 not 1 or 2...
  int ndos = (_prtdos == 1 ? 3 : 1);
  if ( tsmear == 0 ) return _dos[ndos*isppol];
  else if ( tsmear == 1 ) return _dos[ndos*isppol+1];
  else return _dos[ndos*isppol+2];
}

std::vector<double> ElectronDos::dos(unsigned isppol, ElectronDos::Angular angular) const {
  if ( isppol<1 && isppol >_nsppol )
    throw EXCEPTION("Bad value for isppol",ERRDIV);
  if ( !this->isProjected() )
    throw EXCEPTION("Not a projected DOS",ERRDIV);
  isppol--; // isppol should be 0 or 1 not 1 or 2...
  return _dos[5*isppol+(unsigned) angular];


}

std::vector<double> ElectronDos::dos(unsigned isppol, ElectronDos::Angular angular, int magnetic) const {
  if ( isppol<1 && isppol >_nsppol )
    throw EXCEPTION("Bad value for isppol",ERRDIV);
  if ( !this->isProjected() )
    throw EXCEPTION("Not a projected DOS",ERRDIV);
  if ( this->isMResolved() && (magnetic > (int) angular || magnetic < -1*(int)(angular) ) )
    throw EXCEPTION("Bad value for magnetic",ERRDIV);
  isppol--; // isppol should be 0 or 1 not 1 or 2...
  int lbegin = 0;
  for ( int il = 0 ; il < (int)(angular) ; ++il ) {
    lbegin += (2*il+1);
  }
  return _lm[25*isppol+lbegin+(int)(angular)+magnetic];
}

std::vector<double> ElectronDos::dos(unsigned isppol, ElectronDos::Angular angular, ElectronDos::PAWPart part) const {
  if ( isppol<1 && isppol >_nsppol )
    throw EXCEPTION("Bad value for isppol",ERRDIV);
  isppol--; // isppol should be 0 or 1 not 1 or 2...

  return _dos[4*5*isppol+(unsigned)part*5+(unsigned) angular];

}

std::vector<double> ElectronDos::dos(ElectronDos::SOCProj proj) const {
  return _dos[(unsigned) proj];
}

std::vector<double> ElectronDos::energies() const
{
  return _energies;
}

bool ElectronDos::pawDecomposition() const
{
  return _pawDecomposition;
}

ElectronDos::ElectronDos() :
  _prtdos(0),
  _nsppol(1),
  _iatom(0),
  _nenergy(0),
  _prtdosm(false),
  _pawDecomposition(false),
  _efermi(0.e0),
  _energies(),
  _dos(),
  _integrated(),
  _lm()
{
  ;
}

//
ElectronDos::~ElectronDos() {
  ;
}

void ElectronDos::readFromFile(const std::string &filename)
{
  std::ifstream file(filename, std::ios::in);
  if (file)
    this->readFromFile(file);
  else
    throw EXCEPTION("Unable to open file "+filename,ERRDIV);
  file.close();
}

void ElectronDos::readFromFile(std::istream &stream)
{
  std::string tmp;

  // Read 3 first dummy lines
  std::getline(stream,tmp);
  std::getline(stream,tmp);
  std::getline(stream,tmp);

  //Read nsppol
  std::getline(stream,tmp,'=');
  stream >> _nsppol;
  std::getline(stream,tmp);

  // Find fermi energy
  do {
    stream >> tmp;
  } while (tmp != "Fermi");
  stream >> tmp >> tmp; // energy :
  stream >> _efermi;

  std::getline(stream,tmp); // end line
  std::getline(stream,tmp); // empty
  std::getline(stream,tmp); // _prtdos future
  if ( tmp == "# The DOS (in electrons/Hartree/cell) and integrated DOS (in electrons/cell)," ) {
    _prtdos = 1;
    // Read dummy line
    std::getline(stream,tmp);
  }
  else if ( tmp == "# The DOS (in electrons/Hartree/cell) and integrated DOS (in electrons/cell) are computed," ) {
    _prtdos = 2;
  }
  else if ( tmp == "# The local DOS (in electrons/Hartree for one atomic sphere)" ) {
    _prtdos = 3;
    std::getline(stream,tmp);
  }
  else if ( tmp == "# The spin component DOS (in electrons/Hartree/cell)" ) {
    _prtdos = 5;
    // Read 3 dummy lines
    std::getline(stream,tmp);
    std::getline(stream,tmp);
    std::getline(stream,tmp);
  }
  else {
    throw EXCEPTION("Unable to determine the _prtdos",ERRDIV);
  }

  stream >> tmp >> tmp >> _nenergy;
  if (!stream || _nenergy< 1) throw EXCEPTION("Error reading number of energies",ERRDIV);
  std::getline(stream,tmp);
  std::getline(stream,tmp);
  std::getline(stream,tmp);

  // # energy DOS .....
  if ( _prtdos == 1 ) {
    std::getline(stream,tmp);
    std::getline(stream,tmp);
  }

  _energies.resize(_nenergy);
  switch (_prtdos) {
    case 1:
      _dos.resize(3*_nsppol);
      _integrated.resize(1*_nsppol);
      break;
    case 2:
      _dos.resize(1*_nsppol);
      _integrated.resize(1*_nsppol);
      break;
    case 3:
      // Suppose only the total DOS.
      // Projected will be handle later because we don't know
      // if this is for an atom or not
        _dos.resize(1*_nsppol);
        _integrated.resize(1*_nsppol);
      break;
    case 5: // nsppol = 1 since this is non col mag;
        _dos.resize(7);
        _integrated.resize(7);
      break;
    default:
      throw EXCEPTION("This should never happen",ERRDIV);
      break;
  }

  for ( unsigned isppol = 0 ; isppol < _nsppol ; ++isppol ) {
    // READ HEADER EXTRA
    if ( isppol == 1 ) std::getline(stream,tmp); // # empty line for spin down ...
    if ( _nsppol == 2 ) std::getline(stream,tmp); // # Spin
    if ( _prtdos == 2 ) {
      std::getline(stream,tmp); // DOS + integrated DOS
    }
    else if ( _prtdos == 3 ) {
      std::getline(stream,tmp); // DOS + integrated DOS
      if ( tmp.find("Local") == std::string::npos ) {} // DOS + integrated (TOTAL)
      else if ( tmp.find("integrated") != std::string::npos ) { // ATOM decomposition + integrated
        std::getline(stream,tmp,'='); // blabla up to iat=
        std::getline(stream,tmp,'='); // blabla up to iatom=
        stream >> _iatom;
        if ( stream.fail() ) throw EXCEPTION("Expected iatom=",ERRDIV);
        std::getline(stream,tmp); // end line
        std::getline(stream,tmp); // blabla
        std::getline(stream,tmp); // empty line
        std::getline(stream,tmp); // empty line
        if ( tmp.find("PAW") != std::string::npos ) {
          std::getline(stream,tmp); // emtpy line
          std::getline(stream,tmp); // new line with energy header
        }
        if ( tmp.find("lm") != std::string::npos ) _prtdosm = true;
        if ( isppol == 0 && _prtdosm ) _lm.resize(25*_nsppol);
        if ( isppol == 0 && this->isProjected()) {
          _dos.resize(5*_nsppol);
          _integrated.resize(5*_nsppol);
        }
      }
      else {
        _pawDecomposition = true;
        std::getline(stream,tmp); // PW
        std::getline(stream,tmp); // AE
        std::getline(stream,tmp); // -PS
        std::getline(stream,tmp,'='); // blabla up to iat=
        std::getline(stream,tmp,'='); // blabla up to iatom=
        stream >> _iatom;
        if ( stream.fail() ) throw EXCEPTION("Expected iatom=",ERRDIV);
        std::getline(stream,tmp); // end line
        std::getline(stream,tmp); // sphere radius
        std::getline(stream,tmp); // empty line
        std::getline(stream,tmp); // energy l=0 ...
        if ( isppol == 0 ) { _dos.resize(4*5*_nsppol); _integrated.clear();}

      }
    }
    else if ( _prtdos == 5 ) {
      std::getline(stream,tmp); // energy upup up down downup downdown...
    }

    // READ DATA
    if ( _prtdos == 1 ) {
      std::vector<double> &d  = _dos[3*isppol  ];
      std::vector<double> &dp = _dos[3*isppol+1];
      std::vector<double> &dm = _dos[3*isppol+2];
      std::vector<double> &integ = _integrated[isppol];
      d.resize(_nenergy);
      dp.resize(_nenergy);
      dm.resize(_nenergy);
      integ.resize(_nenergy);
      for (unsigned e = 0 ; e < _nenergy ; ++e)
        // energy dos integ dos+tsmear/2 dos-tsmear/2
        stream >> _energies[e] >> d[e] >> integ[e] >> dp[e] >> dm[e];

    }
    else if ( _prtdos == 2 ) {
      std::vector<double> &d  = _dos[isppol  ];
      std::vector<double> &integ = _integrated[isppol];
      d.resize(_nenergy);
      integ.resize(_nenergy);
      for (unsigned e = 0 ; e < _nenergy ; ++e)
        // energy dos integ
        stream >> _energies[e] >> d[e] >> integ[e];
    }
    else if ( _prtdos == 3 || 5 ) {
      for (auto &d : _dos ) d.resize(_nenergy);
      for (auto &i : _integrated ) i.resize(_nenergy);
      for (auto &lm : _lm) lm.resize(_nenergy);

      for (unsigned e = 0 ; e < _nenergy ; ++e) {
        stream >> _energies[e];
        for ( unsigned d = isppol*_dos.size()/_nsppol ; d < (isppol+1)*_dos.size()/_nsppol ; ++d)
          stream >> _dos[d][e];
        for ( unsigned i = isppol*_integrated.size()/_nsppol ; i < (isppol+1)*_integrated.size()/_nsppol ; ++i)
          stream >> _integrated[i][e];
        for ( unsigned lm = isppol*_lm.size()/_nsppol ; lm < (isppol+1)*_lm.size()/_nsppol ; ++lm)
          stream >> _lm[lm][e];

        // energy dos integr
        // energy  0 1 2 3 4 => 0 1 2 3 4
        // energy  0 1 2 3 4 => 0 1 2 3 4 0 ->25
        // energy  0 1 2 3 4  0 1 2 3 4 0  0 1 2 3 4  0 1 2 3 4 0 // PAW
        // energy  UU UD DU DD X Y Z -> UU UD DU DD X Y Z
      }
    }
    std::getline(stream,tmp); // end line
  }
}

bool ElectronDos::isProjected() const {
  return (_prtdos == 3) && (_iatom > 0);
}

bool ElectronDos::isMResolved() const
{
  return _prtdosm;
}


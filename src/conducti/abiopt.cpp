/**
 * @file src/./abiopt.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2019 Jordan Bieder
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


#include "conducti/abiopt.hpp"
#include "base/utils.hpp"
#include <algorithm>

//
AbiOpt::AbiOpt() :
  _header(),
  _nablas(),
  _eigens()
{
  ;
}

//
AbiOpt::~AbiOpt() {
  ;
}

//
void AbiOpt::readFromFile(const std::string &filename) {
  try {
    _header.readFromFile(filename);
  }
  catch( Exception& e ) {
    e.ADD("Header cannot be read properly",e.getReturnValue());
    throw e;
  }
  if ( _header.fform() != 610 )
    throw EXCEPTION("fform should be 610",ERRABT);

  std::ifstream file;
  
  file.open(filename,std::ios::in|std::ios::binary);

  if ( !file )
    throw EXCEPTION(std::string("File ")+filename+" could not be opened",ERRABT);

  file.seekg(_header.endHeader());
  if ( !file )
    throw EXCEPTION(std::string("File ")+filename+" does not contain enough data",ERRABT);

  unsigned int marker;                  // Size of the next variable.

  int nsppol = _header.nsppol();
  int nkpt = _header.nkpt();
  auto &nband = _header.nband();
  int mband = _header.mband();

  _nablas.resize(nkpt*nsppol);
  _eigens.resize(nkpt*nsppol);

  // Read eigens
  file.read((char*)&marker,sizeof(int));
  if ( marker != nkpt*nsppol*mband*sizeof(double) )
    throw EXCEPTION("Bad starting marker for eigens",ERRDIV);

  for ( int ikptpol = 0 ; ikptpol < nsppol*nkpt ; ++ ikptpol ) {
    _eigens[ikptpol].resize(mband);
    file.read((char*)&_eigens[ikptpol][0],mband*sizeof(double));
  }

  file.read((char*)&marker,sizeof(int));
  if ( marker != nkpt*nsppol*mband*sizeof(double) )
    throw EXCEPTION("Bad ending marker for eigen",ERRDIV);

  // Read nabla 
  for ( int ikptpol = 0 ; ikptpol < nsppol*nkpt ; ++ ikptpol ) {
    int nband_k = nband[ikptpol];
    int expected = nband_k*nband_k*2;
    for ( int dir = 0 ; dir < 3 ; ++dir ) { 
      file.read((char*)&marker,sizeof(int));
      if ( marker != expected*sizeof(double) )
        throw EXCEPTION("Bad starting marker for ikptpol "+utils::to_string(ikptpol) + " dir " + utils::to_string(dir),ERRDIV);
      _nablas[ikptpol][dir].resize(nband_k*nband_k);
      file.read((char*)&_nablas[ikptpol][dir][0],expected*sizeof(double));
      file.read((char*)&marker,sizeof(int));
      if ( marker != expected*sizeof(double) )
        throw EXCEPTION("Bad ending marker for ikptpol "+utils::to_string(ikptpol) + " dir " + utils::to_string(dir),ERRDIV);
    }
  }
}

//
int AbiOpt::nband(int isppol, int ikpt) const {
  if ( isppol >= _header.nsppol() )
    throw EXCEPTION("Error isppol",ERRDIV);
  if ( ikpt >= _header.nkpt() )
    throw EXCEPTION("Error nkpt",ERRDIV);
  return _header.nband()[isppol*_header.nkpt()+ikpt];
}

//
std::vector<double>::const_iterator AbiOpt::occ(int isppol, int ikpt) const {
  if ( isppol >= _header.nsppol() )
    throw EXCEPTION("Error isppol",ERRDIV);
  if ( ikpt >= _header.nkpt() )
    throw EXCEPTION("Error nkpt",ERRDIV);
  const int distance = isppol*_header.nkpt()*_header.mband()+ikpt*_header.mband();
  return _header.occ3d().begin()+distance;
}

//
const std::vector<double>& AbiOpt::eigen(int isppol, int ikpt) const {
  if ( isppol >= _header.nsppol() )
    throw EXCEPTION("Error isppol",ERRDIV);
  if ( ikpt >= _header.nkpt() )
    throw EXCEPTION("Error nkpt",ERRDIV);
  return _eigens[isppol*_header.nkpt()+ikpt];
}

//
const std::vector<std::complex<double>>& AbiOpt::nabla(int isppol, int ikpt, int idir) const {
  if ( isppol >= _header.nsppol() )
    throw EXCEPTION("Error isppol",ERRDIV);
  if ( ikpt >= _header.nkpt() )
    throw EXCEPTION("Error nkpt",ERRDIV);
  if ( idir >= 3  )
    throw EXCEPTION("Error idir",ERRDIV);
  return _nablas[isppol*_header.nkpt()+ikpt][idir];
}


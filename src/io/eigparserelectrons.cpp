/**
 * @file src/./eigparserelectrons.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2018 Jordan Bieder
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


#include "io/eigparserelectrons.hpp"
#include "base/mendeleev.hpp"
#include <algorithm>

using namespace Agate;

//
EigParserElectrons::EigParserElectrons() :EigParser(),
  _lmax(-1),
  _fractions(),
  _lmMask(),
  _dtset(nullptr)
{
}

//
EigParserElectrons::~EigParserElectrons() {
  ;
}

//
void EigParserElectrons::selectLM(int l, std::vector<int> &umaskm) {
  if ( l >= _lmax ) 
    throw EXCEPTION("l is too large compared to _lmax",ERRDIV);
  for ( int m : umaskm )
    if ( m < -l || m > l )
      throw EXCEPTION("Found a bad value for m",ERRDIV);

  // Hide all
  std::fill(_lmMask.begin(),_lmMask.end(),0);
  int lbegin = 0;
  //int lend = 0;
  for ( int il = 0 ; il < l ; ++il ) {
    lbegin += (2*il+1);
  }
  //lend = lbegin + (2*l+1);
  //std::cerr << lbegin << "->" << lend << std::endl;
  const int shift = l;
  for ( auto m : umaskm ) {
    _lmMask[lbegin+m+shift] = 1;
  }
}

std::vector<unsigned> EigParserElectrons::getBandColor(const unsigned iband, const unsigned ispin, const std::vector<unsigned> umask) const { 
  unsigned spin = _hasSpin ? 2 : 1 ;
  unsigned nkpt = _kpts.size()/spin;

  if ( _hasSpin ) {
    if ( nkpt*2 != _kpts.size() )
      throw EXCEPTION("Non-consistent data : bad number of k-points",ERRABT);
    if ( ispin != 1 && ispin != 2 )
      throw EXCEPTION("ispin must be 1 or 2",ERRABT);
  }
  else {
    if ( ispin != 1 )
      throw EXCEPTION("ispin must be 1",ERRABT);
  }

  std::vector<unsigned> color(nkpt,0);
  if ( _fractions.empty() ) return color;
  if ( _dtset.get() == nullptr ) throw EXCEPTION("Need a dtset",ERRDIV);
  const unsigned natom = _dtset->natom();

  if ( iband < _nband ) {
    auto typat = _dtset->typat();
    auto znucl = _dtset->znucl();
    unsigned ntypat = znucl.size();

    if ( umask.size() > 0 ) {
      for ( unsigned iatom = 0 ; iatom < _dtset->natom() ; ++iatom ) {
        if ( std::find( umask.begin(), umask.end(), iatom+1 ) == umask.end() ) {
          typat[iatom]=0;
        }
      }
    }

    std::vector<unsigned> colors(ntypat+1,0x666666);
    for ( unsigned itypat = 0 ; itypat < ntypat ; ++itypat ) {
      unsigned r = 255*Mendeleev.color[znucl[itypat]][0];
      unsigned g = 255*Mendeleev.color[znucl[itypat]][1];
      unsigned b = 255*Mendeleev.color[znucl[itypat]][2];
      colors[itypat+1] = (r<<16)|(g<<8)|b;
    }

    for ( unsigned ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
      unsigned r = 0,g = 0,b = 0;
      for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
        for ( int l = 0 ; l < _lmax*_lmax ; ++l ) {
          unsigned cr = (colors[typat[iatom]] & 0xFF0000) >> 16;
          unsigned cg = (colors[typat[iatom]] & 0x00FF00) >> 8;
          unsigned cb = (colors[typat[iatom]] & 0x0000FF);
          const unsigned kpt = (ispin-1)*nkpt+ikpt;
          const unsigned proj = (iatom*_lmax*_lmax+l)*_nband+iband;
          r+= (_fractions[kpt][proj]*_lmMask[l]*cr);
          g+= (_fractions[kpt][proj]*_lmMask[l]*cg);
          b+= (_fractions[kpt][proj]*_lmMask[l]*cb);
        }
      }
      color[ikpt] = ((r<<16)|(g<<8)|b);
    }
  }
  else
    throw EXCEPTION("Out of range",ERRDIV);
  return color;
}

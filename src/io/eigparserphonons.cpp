/**
 * @file src/./eigparserphonons.cpp
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


#include "io/eigparserphonons.hpp"
#include "base/mendeleev.hpp"

using namespace Agate;

//
EigParserPhonons::EigParserPhonons() : EigParser(),
  _dtset(nullptr),
  _eigenDisp()
{
  ;
}

//
EigParserPhonons::~EigParserPhonons() {
  ;
}

std::vector<std::vector<double>> EigParserPhonons::getBandProjection(const unsigned iband, const unsigned ispin) const {
  if ( _eigenDisp.empty() )
    throw EXCEPTION("Eigen displacements are not known",ERRABT);

  if ( _dtset.get() == nullptr )
    throw EXCEPTION("Need a dtset",ERRABT);

  if ( _hasSpin )
    throw EXCEPTION("Not available with phonon files",ERRABT);

  if ( ispin != 1 ) throw EXCEPTION("Bad value for ispin",ERRABT);

  unsigned natom = _dtset->natom();
  unsigned nkpt= _kpts.size();

  if ( _eigenDisp.size() != nkpt )
    throw EXCEPTION("Eigen displacements size for kpts is wrong",ERRABT);

  if ( _eigenDisp[0].size() != _nband*_nband*2 )
    throw EXCEPTION("Eigen displacements size for modes is wrong",ERRABT);

  std::vector<std::vector<double>> projections(nkpt,std::vector<double>(natom,0));

  if ( iband < _nband ) {
    auto znucl = _dtset->znucl();
    auto typat = _dtset->typat();
    for ( unsigned ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
      auto myband = _eigenDisp[ikpt].begin();
      std::advance(myband,2*_nband*iband);
      // Compute weigth for each atom which is the sum over x y and z
      for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
        double mass = Mendeleev.mass[znucl[typat[iatom]-1]]*phys::amu_emass;
        for ( unsigned idir = 0 ; idir < 3 ; ++ idir ) {
          double re = *myband;
          ++myband;
          double im = *myband;
          ++myband;
          projections[ikpt][iatom] += (re*re+im*im)*mass;
        }
      }
    }
    // Renormalize so the sum of projections[ikpt][natom] = 1
    // This is already the case but just in case.
    for ( unsigned ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
      double norm = 0.;
      for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
        norm += projections[ikpt][iatom];
      }
      for ( unsigned iatom = 0 ; iatom < natom ; ++iatom ) {
        projections[ikpt][iatom] /= norm;
      }
    }
  }
  else
    throw EXCEPTION("Out of range",ERRDIV);

  return projections;

}

//
std::vector<unsigned> EigParserPhonons::getBandColor(const unsigned iband, const unsigned ispin, const std::vector<unsigned> umask) const { 
  (void) ispin;
  unsigned nkpt = _kpts.size();
  std::vector<unsigned> color(nkpt,0);
  if ( _dtset.get() == nullptr ) throw EXCEPTION("Need a dtset",ERRDIV);
  if ( _hasSpin ) throw EXCEPTION("Phonons don't have spin !",ERRABT);
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

    try {
      auto projection(this->getBandProjection(iband,1));
      for ( unsigned ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
        unsigned r = 0,g = 0,b = 0;
        for ( unsigned iatom = 0 ; iatom < _dtset->natom() ; ++iatom ) {
          unsigned cr = (colors[typat[iatom]] & 0xFF0000) >> 16;
          unsigned cg = (colors[typat[iatom]] & 0x00FF00) >> 8;
          unsigned cb = (colors[typat[iatom]] & 0x0000FF);
          r+= (projection[ikpt][iatom]*cr);
          g+= (projection[ikpt][iatom]*cg);
          b+= (projection[ikpt][iatom]*cb);
        }
        color[ikpt] = ((r<<16)|(g<<8)|b);
      }
    }
    catch ( Exception &e ) {
      e.ADD("Not able to get projection",ERRDIV);
      throw e;
    }
  }
  else
    throw EXCEPTION("Out of range",ERRDIV);
  return color;
}

void EigParserPhonons::renormalizeEigenDisp() {
  if ( _dtset.get() == nullptr ) throw EXCEPTION("Need a dtset",ERRDIV);
  // Renormalize
  auto znucl = _dtset->znucl();
  auto typat = _dtset->typat();
  for ( auto &disp : _eigenDisp ) {
    for ( unsigned imode = 0 ; imode < _nband ; ++imode ) {
      double norm = 0.;
      for ( unsigned iatom = 0 ; iatom < _dtset->natom() ; ++iatom ) {
        double mass = Mendeleev.mass[znucl[typat[iatom]-1]]*phys::amu_emass;
        for ( unsigned idir = 0 ; idir < 3 ; ++idir ) {
          double re = disp[2*_nband*imode+3*2*iatom+idir*2];
          double im = disp[2*_nband*imode+3*2*iatom+idir*2+1];
          norm += mass*(re*re+im*im);
        }
      }
      norm = std::sqrt(norm);
      for ( unsigned iatom = 0 ; iatom < _dtset->natom() ; ++iatom ) {
        for ( unsigned idir = 0 ; idir < 3 ; ++idir ) {
          disp[2*_nband*imode+3*2*iatom+idir*2] /= norm;
          disp[2*_nband*imode+3*2*iatom+idir*2+1] /= norm;
        }
      }
    }
  }
}

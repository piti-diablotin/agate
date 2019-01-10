/**
 * @file src/./conducti.cpp
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


#include "conducti/conducti.hpp"
#include "base/phys.hpp"
#ifdef HAVE_OMP
#include <omp.h>
#endif

//
Conducti::Conducti() :
  _nomega(4000),
  _omegaMin(0.001),
  _omegaMax(1.837466191e-01),
  _smearing(0.003),
  _bandMin(0),
  _bandMax(-1),
  _eMin(-1000),
  _eMax(1000),
  _histogram()
{
  ;
}

//
Conducti::~Conducti() {
  ;
}


std::array<std::vector<double>,6> Conducti::fullTensor(const AbiOpt &abiopt) {
}

std::array<std::vector<double>,3> Conducti::diagonalTensor(const AbiOpt &abiopt) {
}

std::vector<double> Conducti::traceTensor(const AbiOpt &abiopt) {
  int nsppol = abiopt.nsppol();
  int nkpt = abiopt.nkpt();

  double factor = 2*phys::pi/(geometry::det(abiopt.rprim())*_smearing*2*std::sqrt(phys::pi));

  // On construit les omegas
  std::vector<double> omega(_nomega);
  double domega = (_omegaMax-_omegaMin)/(_nomega-1);
  for ( int iomega = 0 ; iomega < _nomega ; ++iomega )
    omega[iomega] = _omegaMin+(double)iomega*domega;

  std::vector<double> sigma(nsppol*_nomega,0);
  double inv_smearingSquare = 1./(_smearing*_smearing);
  auto wtk = abiopt.wtk();

// Make sigma private and then reduce !

int progress = 0;
#pragma omp declare reduction(+: std::vector<double> : \
                              std::transform(omp_out.begin(), omp_out.end(), omp_in.begin(), omp_out.begin(), std::plus<double>())) \
                    initializer(omp_priv = omp_orig)
#pragma omp parallel for collapse(2), schedule(static), reduction(+:sigma)
  for ( int isppol = 0 ; isppol < nsppol ; ++isppol ) {
    for ( int ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
#pragma omp atomic
      ++progress;
#ifdef HAVE_OMP
      if ( omp_get_thread_num() == 0 )
#endif
        std::clog <<progress/(double)(nkpt*nsppol)*100. << std::endl;
      double weight = wtk[ikpt]/3.;
      int nband = abiopt.nband(isppol,ikpt);
      auto occ = abiopt.occ(isppol,ikpt);
      auto &eigen = abiopt.eigen(isppol,ikpt);
      std::vector<double> coeff(nband*nband,0);
      for ( int idir = 0 ; idir < 3 ; ++idir ) {
        for ( int jband = 0 ; jband < nband ; ++ jband ) {
          for ( int iband = 0 ; iband < nband ; ++iband ) {
            auto &nabla = abiopt.nabla(isppol,ikpt,idir);
            coeff[iband*nband+jband] += weight*std::norm(nabla[jband*nband+iband]);
          }
        }
      }
      for ( int iband = 0 ; iband < nband ; ++iband ) {
        for ( int jband = iband+1 ; jband < nband ; ++jband ) {
          const double docc = occ[iband]-occ[jband];
          if ( std::abs(docc) < 1e-8 ) continue;
          const double preContrib = coeff[iband*nband+jband]*docc;
          for ( int iomega = 0 ; iomega < _nomega ; ++iomega ) {
            const double w = omega[iomega];
            const double dE = eigen[jband]-eigen[iband];
            const double contrib = preContrib/w
              * ( std::exp( -(dE-w)*(dE-w)*inv_smearingSquare ) - std::exp( -(dE+w)*(dE+w)*inv_smearingSquare) );
            sigma[isppol*_nomega+iomega] += contrib*factor ;
          }
        }
      }
    }
  }
  return sigma;
}


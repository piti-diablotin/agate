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
  _nsppol(1),
  _nomega(4000),
  _omegaMin(0.001),
  _omegaMax(1.837466191e-01),
  _smearing(0.003),
  _bandSelection(),
  _energySelection(),
  _histDelta(0),
  _histBorder(),
  _selection(NONE),
  _omega(),
  _sigma(),
  _histogramI(),
  _histogramJ()
{
  _bandSelection[0] = 0;
  _bandSelection[1] = 1000;
  _bandSelection[2] = 0;
  _bandSelection[3] = 1000;
  _energySelection[0] = -1000;
  _energySelection[1] = 1000;
  _energySelection[2] = -1000;
  _energySelection[3] = 1000;
}

//
Conducti::~Conducti() {
  ;
}

void Conducti::setNOmega(int nomega) {
  if ( nomega < 2 )
    throw EXCEPTION("Number of omega should be larger than 2",ERRDIV);
  _nomega = nomega;
}

void Conducti::setSmearing(double smearing) {
  if ( smearing < 1e-6 )
    throw EXCEPTION(utils::to_string(smearing)+": Smearing value must be larger ",ERRDIV);
  _smearing = smearing;
}

void Conducti::setOmegaRange(double omin, double omax) {
  if ( omin > omax )
    throw EXCEPTION("min cannot be larger than max",ERRDIV);
  if ( omin < 0 )
    throw EXCEPTION("min cannot be negative",ERRDIV);
  if ( omax < 0 )
    throw EXCEPTION("min cannot be negative",ERRDIV);
  _omegaMin = omin;
  _omegaMax = omax;
  _selection = NONE;
}

void Conducti::setRange(double eMin1, double eMax1, double eMin2, double eMax2) {
  if ( eMin1 > eMax1 )
    throw EXCEPTION("min1 cannot be larger than max1",ERRDIV);
  if ( eMin2 > eMax2 )
    throw EXCEPTION("min2 cannot be larger than max2",ERRDIV);
  if ( eMin2 < eMax1 )
    throw EXCEPTION("min2 cannot be less than max1",ERRDIV);
  _energySelection[0] = eMin1;
  _energySelection[1] = eMax1;
  _energySelection[2] = eMin2;
  _energySelection[3] = eMax2;
  _selection = ENERGY;
}

void Conducti::setRange(int bandMin1, int bandMax1, int bandMin2, int bandMax2) {
  if ( bandMin1 > bandMax1 )
    throw EXCEPTION("min1 cannot be larger than max1",ERRDIV);
  if ( bandMin2 > bandMax2 )
    throw EXCEPTION("min2 cannot be larger than max2",ERRDIV);
  if ( bandMin2 < bandMax1 )
    throw EXCEPTION("min2 cannot be less than max1",ERRDIV);
  _bandSelection[0] = bandMin1;
  _bandSelection[1] = bandMax1;
  _bandSelection[2] = bandMin2;
  _bandSelection[3] = bandMax2;
  _selection = BAND;
}

void Conducti::buildOmega() {
  _omega.resize(_nomega);
  double domega = (_omegaMax-_omegaMin)/(_nomega-1);
  for ( int iomega = 0 ; iomega < _nomega ; ++iomega )
    _omega[iomega] = _omegaMin+(double)iomega*domega;
}

void Conducti::buildHistogram(double min, double max, int npoints) {
  _histogramI.resize(npoints);
  _histogramJ.resize(npoints);
  _histDelta = (max-min)/(npoints-1);
  _histBorder[0] = min;
  _histBorder[1] = max;
  for ( int i = 0 ; i < npoints ; ++i ) {
    _histogramI[i] = 0;
    _histogramJ[i] = 0;
  }
}


std::array<std::vector<double>,6> Conducti::fullTensor(const AbiOpt &abiopt) {
}

std::array<std::vector<double>,3> Conducti::diagonalTensor(const AbiOpt &abiopt) {
}

void Conducti::traceTensor(const AbiOpt &abiopt) {
  _nsppol = abiopt.nsppol();
  int nkpt = abiopt.nkpt();
  double fermi = abiopt.fermie();
  std::clog << "Fermi level: " << fermi << std::endl;

  double factor = 2*phys::pi/(geometry::det(abiopt.rprim())*_smearing*2*std::sqrt(phys::pi));

  this->buildOmega();

  _sigma.resize(_nsppol*_nomega);
  std::fill(_sigma.begin(),_sigma.end(),0);
  double inv_smearingSquare = 1./(_smearing*_smearing);
  auto wtk = abiopt.wtk();

  std::vector<double> fullMinE;
  std::vector<double> fullMaxE;
  for ( int isppol = 0 ; isppol < _nsppol ; ++isppol ) {
    for ( int ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
      auto &eigen = abiopt.eigen(isppol,ikpt);
      double maxE = *std::max_element(eigen.begin(),eigen.end());
      double minE = *std::min_element(eigen.begin(),eigen.end());
      fullMinE.push_back(minE);
      fullMaxE.push_back(maxE);
    }
  }
  //double fullMin = *std::min_element(fullMinE.begin(),fullMinE.end());
  //double fullMax = *std::max_element(fullMaxE.begin(),fullMaxE.end());
  const int nhist = 1000;
  this->buildHistogram(-_omegaMax*2,_omegaMax*2,nhist);
  //this->buildHistogram(fullMin,fullMax,nhist);

  // Make sigma private and then reduce !

  int progress = 0;
#pragma omp declare reduction(+: std::vector<double> : \
    std::transform(omp_out.begin(), omp_out.end(), omp_in.begin(), omp_out.begin(), std::plus<double>())) \
  initializer(omp_priv = omp_orig)
#pragma omp parallel for collapse(2), schedule(static), reduction(+:_sigma), reduction(+:_histogramI), reduction(+:_histogramJ)
  for ( int isppol = 0 ; isppol < _nsppol ; ++isppol ) {
    for ( int ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
#pragma omp atomic
      ++progress;
#ifdef HAVE_OMP
      if ( omp_get_thread_num() == 0 )
#endif
        std::clog << (int) (progress/(double)(nkpt*_nsppol)*100.) << "% ";
      double weight = wtk[ikpt]/3.;
      int nband = abiopt.nband(isppol,ikpt);
      auto occ = abiopt.occ(isppol,ikpt);
      auto &eigen = abiopt.eigen(isppol,ikpt);
      std::vector<double> coeff(nband*nband,0);

      if ( _selection != BAND ) {
        _bandSelection[0] = 0;
        _bandSelection[1] = nband;
        _bandSelection[2] = 0;
        _bandSelection[3] = nband;
      }
      if ( _selection != ENERGY ) {
        double maxE = *std::max_element(eigen.begin(),eigen.end());
        double minE = *std::min_element(eigen.begin(),eigen.end());
        _energySelection[0] = minE-fermi;
        _energySelection[1] = maxE-fermi;
        _energySelection[2] = minE-fermi;
        _energySelection[3] = maxE-fermi;
      }

      for ( int idir = 0 ; idir < 3 ; ++idir ) {
        for ( int jband = _bandSelection[2] ; jband < _bandSelection[3] ; ++ jband ) {
          for ( int iband = _bandSelection[0] ; iband < _bandSelection[1] ; ++iband ) {
            auto &nabla = abiopt.nabla(isppol,ikpt,idir);
            coeff[iband*nband+jband] += weight*std::norm(nabla[jband*nband+iband]);
          }
        }
      }
      for ( int iband = _bandSelection[0] ; iband < _bandSelection[1] ; ++iband ) {
        const double eigenI = eigen[iband] - fermi;
        if ( eigenI <= _energySelection[0] || eigenI >= _energySelection[1] ) continue;
        for ( int jband = std::max(_bandSelection[2],iband+1) ; jband < _bandSelection[3] ; ++jband ) {
          const double eigenJ = eigen[jband] - fermi;
          if ( eigenJ <= _energySelection[2] || eigenJ >= _energySelection[3] ) continue;
          const double docc = occ[iband]-occ[jband];
          const double dE = eigenJ-eigenI;
          if ( std::abs(docc) < 1e-8 || dE < _omegaMin || dE > (_omegaMax+_smearing*2.0) ) continue;
          const double preContrib = coeff[iband*nband+jband]*docc;
          _histogramI[(eigenI-_histBorder[0])/_histDelta]+=docc;
          _histogramJ[(eigenJ-_histBorder[0])/_histDelta]+=docc;
          for ( int iomega = 0 ; iomega < _nomega ; ++iomega ) {
            const double w = _omega[iomega];
            const double contrib = preContrib/w
              * ( std::exp( -(dE-w)*(dE-w)*inv_smearingSquare ) - std::exp( -(dE+w)*(dE+w)*inv_smearingSquare) );
            _sigma[isppol*_nomega+iomega] += contrib*factor ;
          }
        }
      }
    }
  }
  std::clog << std::endl;
}

void Conducti::printSigma(std::ostream& out) {
  for ( int w = 0 ; w < _nomega ; ++w ) {
    out << _omega[w] << "  ";
    double sum = 0;
    for ( int isppol = 0 ; isppol < _nsppol ; ++isppol ) {
      sum+=_sigma[isppol*_nomega+w];
      out << _sigma[isppol*_nomega+w] << "  ";
    }
    out << sum << std::endl;
  }
}

void Conducti::printHistogram(std::ostream& out) {
  for ( unsigned h = 0 ; h < _histogramI.size() ; ++h ) {
    out << _histBorder[0]+h*_histDelta << "  " << _histogramI[h] << "  " << _histogramJ[h] << std::endl;
  }
}

void Conducti::setParameters(ConfigParser &parser){
  parser.setSensitive(false);
  if ( parser.hasToken("erange") && parser.hasToken("brange") )
    throw EXCEPTION("You cannont ask for energy and band range at the same time",ERRDIV);

  if ( parser.hasToken("smearing") )
    this->setSmearing(parser.getToken<double>("smearing",ConfigParser::Characteristic::ENERGY));

  if ( parser.hasToken("nomega") )
    this->setNOmega(parser.getToken<int>("nomega"));

  if ( parser.hasToken("omegarange") ) {
    auto tmp = parser.getToken<double>("omegarange",2,ConfigParser::Characteristic::ENERGY);
    this->setOmegaRange(tmp[0],tmp[1]);
  }

  if ( parser.hasToken("erange") ) {
    auto tmp = parser.getToken<double>("erange",4,ConfigParser::Characteristic::ENERGY);
    this->setRange(tmp[0],tmp[1],tmp[2],tmp[3]);
  }

  if ( parser.hasToken("brange") ) {
    auto tmp = parser.getToken<int>("erange",4,ConfigParser::Characteristic::ENERGY);
    this->setRange(tmp[0],tmp[1],tmp[2],tmp[3]);
  }


  std::clog << "nomega " << _nomega << std::endl;
  std::clog << "omega range " << _omegaMin << " " << _omegaMax << std::endl;
  std::clog << "smearing " << _smearing << std::endl;
  if ( _selection == ENERGY )
    std::clog << "energy ranges" << _energySelection[0] << "->" << _energySelection[1] << "; " << _energySelection[2] << "->" << _energySelection[3] << std::endl;
  else if ( _selection == BAND )
    std::clog << "band ranges" << _bandSelection[0] << "->" << _bandSelection[1] << "; " << _bandSelection[2] << "->" << _bandSelection[3] << std::endl;
}

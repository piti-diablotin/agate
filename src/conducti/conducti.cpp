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
#include "base/unitconverter.hpp"
#include <iomanip>
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
  _eunit(UnitConverter::Ha),
  _sunit(UnitConverter::au),
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
  if ( omin >= omax )
    throw EXCEPTION("min cannot be larger than max",ERRDIV);
  if ( omin <= 0 )
    throw EXCEPTION("min cannot be negative",ERRDIV);
  if ( omax <= 0 )
    throw EXCEPTION("min cannot be negative",ERRDIV);
  _omegaMin = omin;
  _omegaMax = omax;
  _selection = NONE;
}

void Conducti::setRange(double eMin1, double eMax1, double eMin2, double eMax2) {
  if ( eMin1 >= eMax1 )
    throw EXCEPTION("min1 cannot be larger than max1",ERRDIV);
  if ( eMin2 >= eMax2 )
    throw EXCEPTION("min2 cannot be larger than max2",ERRDIV);
  if ( eMin2 <= eMax1 )
    throw EXCEPTION("min2 cannot be less than max1",ERRDIV);
  _energySelection[0] = eMin1;
  _energySelection[1] = eMax1;
  _energySelection[2] = eMin2;
  _energySelection[3] = eMax2;
  _selection = ENERGY;
}

void Conducti::setRange(int bandMin1, int bandMax1, int bandMin2, int bandMax2) {
  if ( bandMin1 >= bandMax1 )
    throw EXCEPTION("min1 cannot be larger than max1",ERRDIV);
  if ( bandMin2 >= bandMax2 )
    throw EXCEPTION("min2 cannot be larger than max2",ERRDIV);
  if ( bandMin2 <= bandMax1 )
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


void Conducti::fullTensor(const AbiOpt &abiopt) {
  (void) abiopt;
}

void Conducti::diagonalTensor(const AbiOpt &abiopt) {
  (void) abiopt;
}

double Conducti::getOmegaMax(const AbiOpt &abiopt) {
  int nsppol = abiopt.nsppol();
  int nkpt = abiopt.nkpt();
  double maxEnergy = 0;

  for ( int isppol = 0 ; isppol < nsppol ; ++isppol ) {
    for ( int ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
      auto &eigen = abiopt.eigen(isppol,ikpt);
      maxEnergy += (*std::max_element(eigen.begin(),eigen.end()))-abiopt.fermie();
    }
  }
  
  return maxEnergy/(nkpt*nsppol);
}

void Conducti::traceTensor(const AbiOpt &abiopt) {
  using std::min;
  using std::max;
  _nsppol = abiopt.nsppol();
  int nkpt = abiopt.nkpt();
  double fermi = abiopt.fermie();
  double maxEnergy = getOmegaMax(abiopt);
  std::clog << "Fermi level [" << _eunit << "]: " << fermi*_eunit << std::endl;

  double factor = 2*phys::pi/(geometry::det(abiopt.rprim())*_smearing*2*std::sqrt(phys::pi));

  this->buildOmega();

  _sigma.resize(_nsppol*_nomega);
  std::fill(_sigma.begin(),_sigma.end(),0);
  double inv_smearingSquare = 1./(_smearing*_smearing);
  auto wtk = abiopt.wtk();
  
  std::clog << "Emax-Efermi [" << _eunit << "]: " << maxEnergy*_eunit << std::endl;

  const int nhist = 1000;
  this->buildHistogram(-_omegaMax*2,_omegaMax*2,nhist);

  int progress = 0;
  int previous = 0;
  auto printProgress = [&]() {
    const double total = nkpt*_nsppol;
    int actual = (int)(100.*(double)progress/total);
    if ( actual > previous ) {
      std::clog << actual << "% ";
      std::clog.flush();
    }
    previous = actual;
  };
#ifdef HAVE_OMP4
  int nthread = 1;
  auto sigma = _sigma;
  auto histogramI = _histogramI;
  auto histogramJ = _histogramJ;
#pragma omp parallel 
#pragma omp single
  {
  nthread = omp_get_num_threads();
  }

#pragma omp declare reduction(+: std::vector<double> : \
    std::transform(omp_out.begin(), omp_out.end(), omp_in.begin(), omp_out.begin(), std::plus<double>())) \
  initializer(omp_priv = omp_orig)
#pragma omp parallel for collapse(2), schedule(static), reduction(+:sigma), reduction(+:histogramI), reduction(+:histogramJ), if ( _nsppol*nkpt >= nthread )
#else
  auto &sigma = _sigma;
  auto &histogramI = _histogramI;
  auto &histogramJ = _histogramJ;
#endif
  for ( int isppol = 0 ; isppol < _nsppol ; ++isppol ) {
    for ( int ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
#pragma omp atomic
      ++progress;
#ifdef HAVE_OMP
      if ( omp_get_thread_num() == 0 )
#endif
        printProgress();
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
        auto &nabla = abiopt.nabla(isppol,ikpt,idir);
        for ( int jband = max(0,_bandSelection[2]) ; jband < min(nband,_bandSelection[3]) ; ++ jband ) {
          for ( int iband = max(0,_bandSelection[0]) ; iband < min(nband,_bandSelection[1]) ; ++iband ) {
            coeff[iband*nband+jband] += weight*std::norm(nabla[jband*nband+iband]);
          }
        }
      }
//#pragma omp parallel for schedule(static), reduction(+:sigma), reduction(+:histogramI), reduction(+:histogramJ), if (_nsppol*nkpt < nthread )
      for ( int iband = max(0,_bandSelection[0]) ; iband < min(nband,_bandSelection[1]) ; ++iband ) {
        const double eigenI = eigen[iband] - fermi;
        if ( eigenI <= _energySelection[0] || eigenI >= _energySelection[1] ) continue;
        for ( int jband = max(_bandSelection[2],iband+1) ; jband < min(nband,_bandSelection[3]) ; ++jband ) {
          const double eigenJ = eigen[jband] - fermi;
          if ( eigenJ <= _energySelection[2] || eigenJ >= _energySelection[3] ) continue;
          const double docc = occ[iband]-occ[jband];
          const double dE = eigenJ-eigenI;
          if ( std::abs(docc) < 1e-8 || dE < (_omegaMin-_smearing*2.0) || dE > (_omegaMax+_smearing*2.0) ) continue;
          const double preContrib = coeff[iband*nband+jband]*docc;
          histogramI[(eigenI-_histBorder[0])/_histDelta]+=docc;
          histogramJ[(eigenJ-_histBorder[0])/_histDelta]+=docc;
          for ( int iomega = 0 ; iomega < _nomega ; ++iomega ) {
            const double w = _omega[iomega];
            const double contrib = preContrib/w
              * ( std::exp( -(dE-w)*(dE-w)*inv_smearingSquare ) - std::exp( -(dE+w)*(dE+w)*inv_smearingSquare) );
            sigma[isppol*_nomega+iomega] += contrib*factor ;
          }
        }
      }
    }
  }
#ifdef HAVE_OMP4
  _sigma = std::move(sigma);
  _histogramI = std::move(histogramI);
  _histogramJ = std::move(histogramJ);
#endif
  progress = nkpt*_nsppol;
  printProgress();
  std::clog << std::endl;
}

void Conducti::printSigma(std::ostream& out) {
  out.precision(14);
  out.setf(std::ios::scientific,std::ios::floatfield);
  out.setf(std::ios::right,std::ios::adjustfield);
  std::string tmp = std::string("Frequency [") +_eunit.str() + std::string("]");
  out << "# " << std::setw(20) << tmp;
  std::string symbol = _sunit.str();
  if ( _nsppol == 2 ) {
    tmp = std::string("Sigma Up [") + symbol + std::string("]");
    out << std::setw(22) << tmp;
    tmp = std::string("Sigma Dn [") + symbol + std::string("]");
    out << std::setw(22) << tmp;
    tmp = std::string("Sigma Tot [") + symbol + std::string("]");
    out << std::setw(22) << tmp << std::endl;
    for ( int w = 0 ; w < _nomega ; ++w ) {
      out << std::setw(22) << _omega[w]*_eunit << "  ";
      double sum = 0;
      for ( int isppol = 0 ; isppol < _nsppol ; ++isppol ) {
        sum+=_sigma[isppol*_nomega+w];
        out << _sigma[isppol*_nomega+w]*_sunit << "  ";
      }
      out << sum*_sunit << std::endl;
    }
  }
  else {
    tmp = std::string("Sigma [") + symbol + std::string("]");
    out << std::setw(22) << tmp << std::endl;
    for ( int w = 0 ; w < _nomega ; ++w ) {
      out << std::setw(22) << _omega[w]*_eunit << "  " << _sigma[w]*_sunit << std::endl;
    }
  }

}

void Conducti::printHistogram(std::ostream& out) {
  out.precision(14);
  out.setf(std::ios::scientific,std::ios::floatfield);
  out.setf(std::ios::right,std::ios::adjustfield);
  std::string tmp = std::string("Energy [") + _eunit.str() + std::string("]");
  out << "# " << std::setw(20) << tmp;
  out << std::setw(22) << "From [au]";
  out << std::setw(22) << "To [au]";
  out << std::endl;
  for ( unsigned h = 0 ; h < _histogramI.size() ; ++h ) {
    out << std::setw(22) << (_histBorder[0]+h*_histDelta)*_eunit << std::setw(22) << _histogramI[h] << std::setw(22) << _histogramJ[h] << std::endl;
  }
}

void Conducti::setUnits(const std::string &eunit, const std::string &sunit) {
    _eunit = UnitConverter::getFromString(utils::tolower(eunit));
    _eunit.rebase(UnitConverter::Ha);
    _sunit = UnitConverter::getFromString(utils::tolower(sunit));
    _sunit.rebase(UnitConverter::au);
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

  std::string eunit("Ha"), sunit("au");
  if ( parser.hasToken("eunit") ) {
    eunit = parser.getToken<std::string>("eunit");
  }
  if ( parser.hasToken("sunit") ) {
    sunit = parser.getToken<std::string>("sunit");
  }
  Conducti::setUnits(eunit,sunit);


  std::clog << "Number of frequencies: " << _nomega << std::endl;
  std::clog << "Frequency range ["<< _eunit << "]: " << _omegaMin*_eunit << "->" << _omegaMax*_eunit << std::endl;
  std::clog << "Smearing energy [" << _eunit << "]: " << _smearing*_eunit << std::endl;
  if ( _selection == ENERGY )
    std::clog << "Energy ranges selection [" << _eunit << "]: " << _energySelection[0]*_eunit << "->" << _energySelection[1]*_eunit << "; " << _energySelection[2]*_eunit << "->" << _energySelection[3]*_eunit << std::endl;
  else if ( _selection == BAND )
    std::clog << "Band ranges selection: " << _bandSelection[0] << "->" << _bandSelection[1] << "; " << _bandSelection[2] << "->" << _bandSelection[3] << std::endl;
}

void Conducti::getResultSigma(Graph::Config &config, bool spin) {
  config.x = _omega;
  std::list<std::vector<double>> &y = config.y;
  std::list<std::string> &labels = config.labels;
  std::string &filename = config.filename;
  std::string &xlabel = config.xlabel;
  std::string &ylabel = config.ylabel;
  config.title = "Conductivity";
  config.doSumUp = false;

  for ( auto &w : config.x ) w=w*_eunit;
  xlabel = std::string("Frequency [") + _eunit.str() + std::string("]");
  ylabel = std::string("Sigma [") + _sunit.str()+ std::string("]");
  if ( _nsppol == 2 ) {
    if ( spin ) {
      labels.push_back("Up");
      labels.push_back("Down");
      labels.push_back("Total");
    }
    std::vector<double> up(_nomega);
    std::vector<double> down(_nomega);
    std::vector<double> total(_nomega);
    for ( int w = 0 ; w < _nomega ; ++w ) {
      up[w] = _sigma[0*_nomega+w]*_sunit;
      down[w] = _sigma[1*_nomega+w]*_sunit;
      total[w] = up[w]+down[w];
    }
    if ( spin ) {
      y.push_back(std::move(up));
      y.push_back(std::move(down));
    }
    y.push_back(std::move(total));
  }
  else {
    std::vector<double> vec(_nomega);
    for ( int w = 0 ; w < _nomega ; ++w ) {
      vec[w] = _sigma[w]*_sunit;
    }
    y.push_back(std::move(vec));
  }

  if ( config.save == Graph::DATA ) {
    //std::ofstream hist(filename+"_histogram.dat",std::ios::out);
    std::ofstream sigma(filename+"_sigma.dat",std::ios::out);
    config.save = Graph::NONE;
    this->printSigma(sigma);
    //this->printHistogram(hist);
    //hist.close();
    sigma.close();
  }
}

void Conducti::getResultHistogram(Graph::Config &config) {
  auto& x  = config.x;
  std::list<std::vector<double>> &y = config.y;
  std::list<std::string> &labels = config.labels;
  std::string &filename = config.filename;
  std::string &xlabel = config.xlabel;
  std::string &ylabel = config.ylabel;
  config.title = "Historgram";
  config.doSumUp = false;

  x.resize(_histogramI.size());
  for ( unsigned h = 0 ; h < _histogramI.size() ; ++h ) {
    x[h] = (_histBorder[0]+h*_histDelta)*_eunit;
  }

  xlabel = std::string("Frequency [") + _eunit.str() + std::string("]");
  ylabel = std::string("Histogram [au]");
  labels.push_back("From");
  labels.push_back("To");
  y.push_back(_histogramI);
  y.push_back(_histogramJ);

  if ( config.save == Graph::DATA ) {
    std::ofstream hist(filename+"_histogram.dat",std::ios::out);
    //std::ofstream sigma(filename+"_sigma.dat",std::ios::out);
    config.save = Graph::NONE;
    //this->printSigma(sigma);
    this->printHistogram(hist);
    hist.close();
    //sigma.close();
  }
}

/**
 * @file src/./histcustommodes.cpp
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


#include "hist/histcustommodes.hpp"
#include "phonons/supercell.hpp"
#include "base/unitconverter.hpp"
#include "base/phys.hpp"
#include "base/fraction.hpp"
#include <random>
#include <chrono>

void HistCustomModes::buildHist(std::vector<DispDB::qptTree> inputCondensedModes)
{
  std::vector<DispDB::qptTree> &condensedModes = (inputCondensedModes.empty()?_condensedModes:inputCondensedModes);

  // First, find smallest qpt to build the supercell
  geometry::vec3d qpt = {{ 1.0, 1.0, 1.0 }};
  const double tol = 1e-6;

  for ( auto it = condensedModes.begin()->begin() ; it != condensedModes.begin()->end() ; ++it ) {
    if ( std::abs(it->first[0]) > tol && std::abs(it->first[0]) < std::abs(qpt[0]) ) qpt[0] = it->first[0];
    if ( std::abs(it->first[1]) > tol && std::abs(it->first[1]) < std::abs(qpt[1]) ) qpt[1] = it->first[1];
    if ( std::abs(it->first[2]) > tol && std::abs(it->first[2]) < std::abs(qpt[2]) ) qpt[2] = it->first[2];
  }

  if ( condensedModes.empty() ) {
    this->buildFromDtset(_reference);
  }

  else {
    unsigned ntime = condensedModes.size();
    // Now go through all mode of all qpt and make the displacement
    Supercell supercellRef(_reference,qpt);
    this->reserve(ntime,supercellRef);
    this->setTryToMap(false);
#ifdef HAVE_CPPTHREAD
    _endThread = false;
    _thread = std::thread([this,condensedModes,supercellRef,ntime](){
#endif
      try {
        for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
#ifdef     HAVE_CPPTHREAD
          if ( _endThread == true ) break;
#endif
          Supercell supercell(supercellRef);
          for ( auto iqpt = condensedModes[itime].begin() ; iqpt != condensedModes[itime].end() ; ++iqpt ) {
            for ( auto vib : iqpt->second ) {
              supercell.makeDisplacement(iqpt->first,_db,vib.imode,vib.amplitude,0);
            }
          }
          this->push(supercell);
        }
      }
      catch (Exception& e) {
        std::cerr << e.fullWhat() << std::endl;
      }
#ifdef HAVE_CPPTHREAD
    });
#endif
  }
}

void HistCustomModes::addNoiseToHist(const HistData& hist, double temperature, InstableModes instableModes, std::function<void()> callback)
{
  //hist.waitTime(hist._ntime);
  unsigned ntime = hist.ntime();
  Supercell firstTime(hist,0);
  firstTime.findReference(_reference);
  geometry::vec3d qpt = firstTime.getDim();
  this->zachariasAmplitudes(temperature,ntime,qpt,instableModes);
  this->reserve(ntime,firstTime);
  this->setTryToMap(false);
#ifdef HAVE_CPPTHREAD
  _endThread = false;
  _thread = std::thread([this,firstTime,&hist,ntime,callback](){
#endif
    for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
#ifdef     HAVE_CPPTHREAD
      if ( _endThread == true ) break;
#endif
      Supercell currentTime(hist,itime);
      currentTime.setReference(firstTime);
      for ( auto iqpt = _condensedModes[itime].begin() ; iqpt != _condensedModes[itime].end() ; ++iqpt ) {
        for ( auto vib : iqpt->second ) {
          currentTime.makeDisplacement(iqpt->first,_db,vib.imode,vib.amplitude,0);
        }
      }
      this->push(currentTime);
    }
    callback();
#ifdef HAVE_CPPTHREAD
  });
#endif
}

void HistCustomModes::animateModes(DispDB::qptTree& condensedModes, unsigned ntime)
{
  // First, find smallest qpt to build the supercell
  geometry::vec3d qpt = {{ 1.0, 1.0, 1.0 }};
  const double tol = 1e-6;

  for ( auto it = condensedModes.begin() ; it != condensedModes.end() ; ++it ) {
    if ( std::abs(it->first[0]) > tol && std::abs(it->first[0]) < std::abs(qpt[0]) ) qpt[0] = it->first[0];
    if ( std::abs(it->first[1]) > tol && std::abs(it->first[1]) < std::abs(qpt[1]) ) qpt[1] = it->first[1];
    if ( std::abs(it->first[2]) > tol && std::abs(it->first[2]) < std::abs(qpt[2]) ) qpt[2] = it->first[2];
  }

  if ( condensedModes.empty() ) {
    this->buildFromDtset(_reference);
  }

  else {
    Supercell supercellRef(_reference,qpt);
    if ( ntime > 1 ) { // Build an animation
      this->setTryToMap(false);
      this->reserve(ntime,supercellRef);
      const double dtheta = phys::pi/(double)ntime;
#ifdef HAVE_CPPTHREAD
    _endThread = false;
    _thread = std::thread([this,condensedModes,supercellRef,ntime,dtheta](){
#endif
      for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
#ifdef     HAVE_CPPTHREAD
          if ( _endThread == true ) break;
#endif
        const double theta = (double) itime*dtheta;
        Supercell supercell(supercellRef);
        for ( auto qpt = condensedModes.begin() ; qpt != condensedModes.end() ; ++qpt ) {
          for ( auto vib : qpt->second ) {
            supercell.makeDisplacement(qpt->first,_db,vib.imode,vib.amplitude,theta);
          }
        }
        this->push(supercell);
      }
#ifdef HAVE_CPPTHREAD
    });
#endif
    }
    else { //Display vectors
      for ( auto qpt = condensedModes.begin() ; qpt != condensedModes.end() ; ++qpt ) {
        for ( auto vib : qpt->second ) {
          supercellRef.arrowDisplacement(qpt->first,_db,vib.imode,vib.amplitude);
        }
      }
      this->buildFromDtset(supercellRef);
    }
  }
}

void HistCustomModes::zachariasAmplitudes(double temperature, unsigned ntime, geometry::vec3d supercell, InstableModes instable)
{
  std::vector<vec3d> qpts;
  for ( unsigned qx = 0 ; qx <= supercell[0]/2 ; ++qx ) {
    const double ratioX = 1./supercell[0];
    for ( unsigned qy = 0 ; qy <= supercell[1]/2 ; ++qy ) {
      const double ratioY = 1./supercell[1];
      for ( unsigned qz = 0 ; qz <= supercell[2]/2 ; ++qz ) {
        const double ratioZ = 1./supercell[2];
        qpts.push_back({static_cast<double>(qx)*ratioX,
                        static_cast<double>(qy)*ratioY,
                        static_cast<double>(qz)*ratioZ});
      }
    }
  }
  std::clog << "Q-pts list:" << std::endl;
  for (auto q : qpts) geometry::print(q,std::clog);

  UnitConverter tempConverter(UnitConverter::K);
  tempConverter = UnitConverter::Ha;
  temperature = temperature*tempConverter;
  std::random_device rd;
  unsigned seed;
  switch(_seedType){
    case None : seed = 0;
      break;
    case Time : seed = std::chrono::system_clock::now().time_since_epoch().count();
      break;
    case Random :
      seed = rd();
      break;
    case User:
      seed = _seed;
      break;
  }
  std::default_random_engine engine;
  std::uniform_real_distribution<double> randomDistrib(-1,1);
  engine.seed(seed);
  for ( unsigned i = 0 ; i < ntime ; ++i ) {
    DispDB::qptTree conf;
    for (auto qpt : qpts) {
      try{
        int iqpt = _db.getQpt(qpt);
        std::vector<DispDB::qMode> amplitudes;
        for (unsigned imode = 0 ; imode < 3*_db.natom() ; ++imode) {
          double energy = _db.getEnergyMode(iqpt,imode);
          if ( energy < 0 ) {
            switch (instable) {
              case Ignore:
                break;
              case Constant:
                amplitudes.push_back({imode,_instableAmplitude,energy});
                continue;
                break;
              case Absolute:
                energy = -energy;
                break;
            }
          }
          if (energy<1e-6) continue; // avoid gamma acoustic modes
          double sigma = sqrt( (phys::BoseEinstein(energy,temperature)+0.5)/(energy*phys::amu_emass) )
                         *phys::b2A*randomDistrib(engine);
          // convert sigma to correct unit
          amplitudes.push_back({imode,sigma,energy});
        }
        conf[qpt] = amplitudes;
      }
      catch(Exception& e) {
        std::stringstream erreur;
        erreur << "Qpt " << geometry::to_string(qpt);
        erreur << " is not in DispDB -> ignored";
        e.ADD(erreur.str(),ERRDIV);
        std::clog << e.fullWhat() << std::endl;
      }
    }
    _condensedModes.push_back(conf);
  }
}

double HistCustomModes::instableAmplitude() const
{
  return _instableAmplitude;
}

void HistCustomModes::setInstableAmplitude(double instableAmplitude)
{
  _instableAmplitude = instableAmplitude;
}

HistCustomModes::HistCustomModes(Dtset& dtset, DispDB& db) :
  HistDataDtset(),
  _reference(dtset),
  _db(db),
  _condensedModes(),
  _seedType(Random),
  _seed(42),
  _instableAmplitude(1)
{
  if (_db.natom()!=_reference.natom())
    throw EXCEPTION("natoms are different in DB and reference structure",ERRDIV);
}

//
HistCustomModes::SeedType HistCustomModes::seedType() const
{
  return _seedType;
}

void HistCustomModes::setSeedType(const SeedType& seedType)
{
  _seedType = seedType;
}

unsigned HistCustomModes::seed() const
{
  return _seed;
}

void HistCustomModes::setSeed(const unsigned& seed)
{
  _seed = seed;
}

void HistCustomModes::reserve(unsigned ntime, const Dtset& dtset)
{
  _natom = dtset.natom();
  _xyz = 3;
  _ntime = ntime;
  _ntimeAvail = 0;
  _filename = "CustomModes";
  _znucl.clear();
  for ( auto z : dtset.znucl() ) {
    _znucl.push_back((int)z);
  }
  _typat.clear();
  for ( auto t : dtset.typat() )
    _typat.push_back((int)t);

  // Check we have all information
  if ( (_znucl.size() == 0) || (_znucl.size() != dtset.ntypat())  ) {
    _znucl.clear();
    _znucl.insert(_znucl.begin(),1,0);
    _typat.clear();
    _typat.insert(_typat.begin(),_natom,0);
    Exception e = EXCEPTION("znucl can not be read.\nSome data will be missing.",ERRWAR);
    std::clog << e.fullWhat() << std::endl;
  }

  // Allocate arrays
  _xcart  .resize(_ntime*_natom*_xyz);
  _xred   .resize(_ntime*_natom*_xyz);
  //_fcart  .resize(_ntime*_natom*_xyz);
  _acell  .resize(_ntime*_xyz);
  _rprimd .resize(_ntime*_xyz*_xyz);
  _time   .resize(_ntime);
  _etotal .resize(_ntime);
  _stress .resize(_ntime*6);

  if ( !dtset.spinat().empty() ) {
    _spinat.resize(_ntime*_natom*_xyz);
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      _spinat[3*iatom  ] = dtset.spinat()[iatom][0];
      _spinat[3*iatom+1] = dtset.spinat()[iatom][1];
      _spinat[3*iatom+2] = dtset.spinat()[iatom][2];
    }
  }

  _ekin.resize(_ntime,0.);
  _velocities.resize(_xyz*_natom*_ntime,0.);
  _temperature.resize(_ntime,0.);
  _pressure.resize(_ntime,0.);
  _entropy.resize(_ntime,0.);
}

void HistCustomModes::insert(unsigned itime, const Dtset& dtset)
{
  if ( dtset.natom() != _natom )
    throw EXCEPTION("natom in dtset mismatch natom in HistData.",ERRWAR);
  if ( itime >= _ntime )
    throw EXCEPTION("itime is larger than ntime in HistData.",ERRWAR);

  _acell[itime*3+0] = dtset.acell()[0];
  _acell[itime*3+1] = dtset.acell()[1];
  _acell[itime*3+2] = dtset.acell()[2];

  for ( int vgt = 0 ; vgt < 6 ; ++vgt )
    _stress[itime*6+vgt] = 0.;

  _time[itime] = itime;

  {
    geometry::mat3d rprim = dtset.rprim();
    _rprimd [itime*9+0] = rprim[0];
    _rprimd [itime*9+1] = rprim[1];
    _rprimd [itime*9+2] = rprim[2];
    _rprimd [itime*9+3] = rprim[3];
    _rprimd [itime*9+4] = rprim[4];
    _rprimd [itime*9+5] = rprim[5];
    _rprimd [itime*9+6] = rprim[6];
    _rprimd [itime*9+7] = rprim[7];
    _rprimd [itime*9+8] = rprim[8];
  }
  _etotal[itime] = 0.;

  {
    auto xcart = dtset.xcart();
    auto xred = dtset.xred();
    auto spinat = dtset.spinat();

    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      _xcart[itime*_natom*3+iatom*3  ] = xcart[iatom][0];
      _xcart[itime*_natom*3+iatom*3+1] = xcart[iatom][1];
      _xcart[itime*_natom*3+iatom*3+2] = xcart[iatom][2];
      _xred [itime*_natom*3+iatom*3  ] = xred[iatom][0];
      _xred [itime*_natom*3+iatom*3+1] = xred[iatom][1];
      _xred [itime*_natom*3+iatom*3+2] = xred[iatom][2];
      if ( !dtset.spinat().empty() && !_spinat.empty() ) {
        _spinat[itime*_natom*3+3*iatom  ] = spinat[iatom][0];
        _spinat[itime*_natom*3+3*iatom+1] = spinat[iatom][1];
        _spinat[itime*_natom*3+3*iatom+2] = spinat[iatom][2];
      }
    }
  }
}

void HistCustomModes::push(const Dtset& dtset)
{
  if ( _ntimeAvail == _ntime )
    throw EXCEPTION("No space left in HistData.",ERRWAR);
  this->insert(_ntimeAvail++,dtset);
}

HistCustomModes::~HistCustomModes() {
  ;
}


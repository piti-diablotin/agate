/**
 * @file src/histdata.cpp
 *
 * @brief Implements the pure virtual class for HIST file
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


#include "hist/histdata.hpp"
#include "base/utils.hpp"
#include "base/exception.hpp"
#include "base/phys.hpp"
#include "base/mendeleev.hpp"
#include "base/geometry.hpp"
#include "base/uriparser.hpp"
#include "base/unitconverter.hpp"
#include "hist/histdatanc.hpp"
#include "hist/histdataoutnc.hpp"
#include "hist/histdatadtset.hpp"
#include "hist/histdatapolydtset.hpp"
#include "hist/histdataheff.hpp"
#include "hist/vaspxml.hpp"
#include "hist/multibinit.hpp"
#include "hist/histdataxyz.hpp"
#include "hist/histdatagsr.hpp"
#include "plot/gnuplot.hpp"
#include "io/configparser.hpp"
#ifdef HAVE_SPGLIB
#  ifdef __cplusplus
extern "C" {
#  endif
#  include "spglib/spglib.h"
#  ifdef __cplusplus
}
#  endif
#else
#  include "bind/findsym.hpp"
#  include "io/cifparser.hpp"
#endif
#include "phonons/supercell.hpp"
#include <memory>
#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iomanip>
#ifdef HAVE_OMP
#include "omp.h"
#endif
#ifdef HAVE_FFTW3
#include <fftw3.h>
#endif
#include <random>

using Agate::mendeleev;

//
HistData::HistData() :
  _natom(0),
  _xyz(0),
  _ntime(0),
  _nimage(0),
  _isPeriodic(false),
  _tryToMap(true),
  _ntimeAvail(0),
  _xcart(),
  _xred(),
  _fcart(),
  _acell(),
  _rprimd(),
  _etotal(),
  _time(),
  _stress(),
  _spinat(),
  _typat(),
  _znucl(),
  _imgdata(),
#ifdef HAVE_SPGLIB
  _symprec(0.001),
#endif
#ifdef HAVE_CPPTHREAD
  _thread(),
  _endThread(false),
#endif
  _filename()
{}

//
HistData::HistData(const HistData& hist) :
  _natom(hist._natom),
  _xyz(hist._xyz),
  _ntime(hist._ntime),
  _nimage(hist._nimage),
  _isPeriodic(false),
  _tryToMap(true),
  _ntimeAvail(hist._ntime),
  _xcart(),
  _xred(),
  _fcart(),
  _acell(),
  _rprimd(),
  _etotal(),
  _time(),
  _stress(),
  _spinat(),
  _typat(hist._typat),
  _znucl(hist._znucl),
  _imgdata(),
#ifdef HAVE_SPGLIB
  _symprec(hist._symprec),
#endif
#ifdef HAVE_CPPTHREAD
  _thread(),
  _endThread(false),
#endif
  _filename(hist._filename)
{

#ifdef HAVE_CPPTHREAD
  // Wait for gathering all data
  hist.waitTime(hist._ntime);
#endif
  _ntime = hist._ntime;
#ifdef HAVE_CPPTHREAD
  _ntimeAvail.store(hist._ntimeAvail.load());
#else
  _ntimeAvail = hist._ntimeAvail;
#endif

  _xcart = hist._xcart;
  _xred = hist._xred;
  _fcart = hist._fcart;
  _acell = hist._acell;
  _rprimd = hist._rprimd;
  _etotal = hist._etotal;
  _time = hist._time;
  _stress = hist._stress;
  _spinat = hist._spinat;
  _imgdata = hist._imgdata;
}

//
HistData::HistData(HistData&& hist) :
  _natom(hist._natom),
  _xyz(hist._xyz),
  _ntime(hist._ntime),
  _nimage(hist._nimage),
  _isPeriodic(hist._isPeriodic),
  _tryToMap(hist._tryToMap),
  _ntimeAvail(hist._ntime),
  _xcart(),
  _xred(),
  _fcart(),
  _acell(),
  _rprimd(),
  _etotal(),
  _time(),
  _stress(),
  _spinat(),
  _typat(hist._typat),
  _znucl(hist._znucl),
  _imgdata(),
#ifdef HAVE_SPGLIB
  _symprec(hist._symprec),
#endif
#ifdef HAVE_CPPTHREAD
  _thread(),
  _endThread(false),
#endif
  _filename(hist._filename)
{

#ifdef HAVE_CPPTHREAD
  // Wait for gathering all data
  hist.waitTime(hist._ntime);
#endif
  _ntime = hist._ntime;
#ifdef HAVE_CPPTHREAD
  _ntimeAvail.store(hist._ntimeAvail.load());
#else
  _ntimeAvail = hist._ntimeAvail;
#endif

  _xcart = std::move(hist._xcart);
  _xred = std::move(hist._xred);
  _fcart = std::move(hist._fcart);
  _acell = std::move(hist._acell);
  _rprimd = std::move(hist._rprimd);
  _etotal = std::move(hist._etotal);
  _time = std::move(hist._time);
  _stress = std::move(hist._stress);
  _spinat = std::move(hist._spinat);
  _imgdata = std::move(hist._imgdata);
}

//
HistData::~HistData() {
#ifdef HAVE_CPPTHREAD
  if ( _thread.joinable() ) {
    _endThread = true;
    _thread.join();
  }
#endif
}

//
HistData& HistData::operator=(const HistData& hist) {
#ifdef HAVE_CPPTHREAD
  // Wait for gathering all data
  if ( _thread.joinable() ) {
    _endThread = true;
    _thread.join();
    _endThread = false;
  }
#endif
  _natom = hist._natom;
  _xyz = hist._xyz;
  _typat = hist._typat;
  _znucl = hist._znucl;
#ifdef HAVE_SPGLIB
  _symprec = hist._symprec;
#endif
  _filename = hist._filename;

#ifdef HAVE_CPPTHREAD
  // Wait for gathering all data
  hist.waitTime(hist._ntime);
#endif
  _ntime = hist._ntime;
#ifdef HAVE_CPPTHREAD
  _ntimeAvail.store(hist._ntimeAvail.load());
#else
  _ntimeAvail = hist._ntimeAvail;
#endif

  _isPeriodic = hist._isPeriodic;
  _tryToMap = hist._tryToMap;
  _xcart = hist._xcart;
  _xred = hist._xred;
  _fcart = hist._fcart;
  _acell = hist._acell;
  _rprimd = hist._rprimd;
  _etotal = hist._etotal;
  _time = hist._time;
  _stress = hist._stress;
  _spinat = hist._spinat;
  _imgdata = hist._imgdata;
  
  return *this;
}

//
HistData& HistData::operator=(HistData&& hist) {
#ifdef HAVE_CPPTHREAD
  // Wait for gathering all data
  if ( _thread.joinable() ) {
    _endThread = true;
    _thread.join();
    _endThread = false;
  }
#endif
  _natom = hist._natom;
  _xyz = hist._xyz;
  _typat = std::move(hist._typat);
  _znucl = std::move(hist._znucl);
#ifdef HAVE_SPGLIB
  _symprec = hist._symprec;
#endif
  _filename = std::move(hist._filename);

#ifdef HAVE_CPPTHREAD
  // Wait for gathering all data
  hist.waitTime(hist._ntime);
#endif
  _ntime = hist._ntime;

#ifdef HAVE_CPPTHREAD
  _ntimeAvail.store(hist._ntimeAvail.load());
#else
  _ntimeAvail = hist._ntimeAvail;
#endif
  _isPeriodic = hist._isPeriodic;
  _tryToMap = hist._tryToMap;
  _xcart = std::move(hist._xcart);
  _xred = std::move(hist._xred);
  _fcart = std::move(hist._fcart);
  _acell = std::move(hist._acell);
  _rprimd = std::move(hist._rprimd);
  _etotal = std::move(hist._etotal);
  _time = std::move(hist._time);
  _stress = std::move(hist._stress);
  _spinat = std::move(hist._spinat);
  _imgdata = std::move(hist._imgdata);
  
  return *this;
}

void HistData::waitTime(unsigned t) const {
#ifdef HAVE_CPPTHREAD_YIELD
  bool ok = false;
  if ( t == _ntime && _ntimeAvail != _ntime)  {
    std::clog << "Making sure full file is loaded...";
    std::clog.flush();
    ok = true;
  }
  while(t >= _ntimeAvail && _ntimeAvail < _ntime) {
    std::this_thread::yield();
  }
  if ( t == _ntime && _thread.joinable() ) {
    _thread.join();
  }
  if ( ok ) 
    std::clog << "ok" << std::endl;
#else
  (void) t;
#endif
}

//
const double* HistData::getXcart(unsigned time, unsigned* natom) const {
  this->waitTime(time);
  if ( time >= _ntime )
    throw EXCEPTION(std::string("Out of range for time ")+utils::to_string(time)+
        std::string("/")+utils::to_string(_ntime),ERRDIV);
  if ( natom != nullptr )
    *natom = _natom;
  return &_xcart[time*_natom*_xyz];
}

//
const double* HistData::getXred(unsigned time, unsigned* natom) const {
  this->waitTime(time);
  if ( time >= _ntime )
    throw EXCEPTION(std::string("Out of range for time ")+utils::to_string(time)+
        std::string("/")+utils::to_string(_ntime),ERRDIV);
  if ( natom != nullptr )
    *natom = _natom;
  return &_xred[time*_natom*_xyz];
}

//
const double* HistData::getFcart(unsigned time, unsigned* natom) const {
  this->waitTime(time);
  if ( time >= _ntime )
    throw EXCEPTION(std::string("Out of range for time ")+utils::to_string(time)+
        std::string("/")+utils::to_string(_ntime),ERRDIV);
  if ( _fcart.size() == 0 )
    throw EXCEPTION("Fcart not available",ERRDIV);
  if ( natom != nullptr )
    *natom = _natom;
  return &_fcart[time*_natom*_xyz];
}

//
const double* HistData::getAcell(unsigned time) const {
  this->waitTime(time);
  if ( time >= _ntime )
    throw EXCEPTION(std::string("Out of range for time ")+utils::to_string(time)+
        std::string("/")+utils::to_string(_ntime),ERRDIV);
  return &_acell[time*_xyz];
}

//
const double* HistData::getRprimd(unsigned time) const {
  this->waitTime(time);
  if ( time >= _ntime )
    throw EXCEPTION(std::string("Out of range for time ")+utils::to_string(time)+
        std::string("/")+utils::to_string(_ntime),ERRDIV);
  return &_rprimd[time*_xyz*_xyz];
}

//

//
double HistData::getEtotal(unsigned time) const {
  this->waitTime(time);
  if ( time >= _ntime )
    throw EXCEPTION(std::string("Out of range for time ")+utils::to_string(time)+
        std::string("/")+utils::to_string(_ntime),ERRDIV);
  return _etotal[time];
}

//
const double* HistData::getStress(unsigned time) const {
  this->waitTime(time);
  if ( time >= _ntime )
    throw EXCEPTION(std::string("Out of range for stress ")+utils::to_string(time)+
        std::string("/")+utils::to_string(_ntime),ERRDIV);
  return &_stress[time*6];
}

//
const double* HistData::getSpinat(unsigned time, unsigned* natom) const {
  if ( _ntime == 0 ) return nullptr;
  this->waitTime(time);
  if ( time >= _ntime )
    throw EXCEPTION(std::string("Out of range for spinat ")+utils::to_string(time)+
        std::string("/")+utils::to_string(_ntime),ERRDIV);
  if ( natom != nullptr )
    *natom = _natom;
  return _spinat.empty() ? nullptr : &_spinat[time*_natom*_xyz];
}


//
double HistData::getTime(unsigned time) const {
  this->waitTime(time);
  if ( time >= _ntime )
    throw EXCEPTION(std::string("Out of range for time ")+utils::to_string(time)+
        std::string("/")+utils::to_string(_ntime),ERRDIV);
  return _time[time];
}

//
unsigned HistData::getNtypat() const {
  return _znucl.size();
}

//
const std::vector<int> HistData::znucl() const {
  return _znucl;
}

//
const std::vector<int> HistData::typat() const {
  return _typat;
}

//
unsigned HistData::getSpgNum(const unsigned itime, double symprec, std::string *name) const {
#ifdef HAVE_SPGLIB
  char symbol[11];
  double (*lattice)[3] = (double(*)[3]) &_rprimd[itime*9];
  double (*positions)[3] = (double(*)[3]) &_xred[itime*_natom*3];

  int num = spg_get_international(symbol,
      lattice,
      positions,
      &_typat[0],
      _natom,
      symprec);
  if ( num == 0 ) throw EXCEPTION("Spacegroup could not be found",ERRDIV);
  if ( name != nullptr ) *name = symbol;
  return (unsigned) num;
#else
  try {
    Findsym fsym;
    CifParser cif;
    fsym.tolerance(symprec*phys::b2A);
    geometry::mat3d lattice;
    lattice[0]=_rprimd[itime*9  ]*phys::b2A;
    lattice[1]=_rprimd[itime*9+1]*phys::b2A;
    lattice[2]=_rprimd[itime*9+2]*phys::b2A;
    lattice[3]=_rprimd[itime*9+3]*phys::b2A;
    lattice[4]=_rprimd[itime*9+4]*phys::b2A;
    lattice[5]=_rprimd[itime*9+5]*phys::b2A;
    lattice[6]=_rprimd[itime*9+6]*phys::b2A;
    lattice[7]=_rprimd[itime*9+7]*phys::b2A;
    lattice[8]=_rprimd[itime*9+8]*phys::b2A;
    fsym.rprim(lattice);
    std::vector<int> znuclat(_typat.size());
    bool meaningfull = false;
    if ( _znucl.size() != 0 ) {
      for ( unsigned iatom = 0 ; iatom < _typat.size() ; ++iatom )
        znuclat[iatom] = _znucl[_typat[iatom]-1]; // _typat starts at 1;
      meaningfull = true;
    }
    else
        znuclat = _typat;
    fsym.natom(_natom);
    fsym.typat(znuclat,meaningfull);
    std::vector<geometry::vec3d> xred(_natom);
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      xred[iatom][0] = _xred[itime*_natom*3+iatom*3  ];
      xred[iatom][1] = _xred[itime*_natom*3+iatom*3+1];
      xred[iatom][2] = _xred[itime*_natom*3+iatom*3+2];
    }
    fsym.xred(xred);
    fsym.findsym();
    std::stringstream out;
    out << fsym.cif();
    cif.parse(out);
    CifParser::DataBlock block(cif.getDataBlock(0));
    int num = utils::stoi(block.getTag("symmetry_Int_Tables_number"));
    if ( name != nullptr ) {
      *name = block.getTag("symmetry_space_group_name_H-M");
      utils::trim(*name,"'\"");
    }
    return (unsigned) num;
  }
  catch (Exception& e) {
    e.ADD("Symmetry analisis with findsym failed.\nConsider compiling with spglib",ERRDIV);
    throw e;
  }
  catch (...) {
    throw EXCEPTION("Symmetry analisis with findsym failed.\nConsider compiling with spglib",ERRDIV);
  }
#endif
}

//
std::string HistData::getSpgHM(const unsigned itime) const {
#ifdef HAVE_SPGLIB
  double (*lattice)[3] = (double(*)[3]) &_rprimd[itime*9];
  double (*positions)[3] = (double(*)[3]) &_xred[itime*_natom*9];
  SpglibDataset * dtset = spg_get_dataset(lattice,
      positions,
      &_typat[0],
      _natom,
      _symprec);
  int hall_number = dtset->hall_number;
  spg_free_dataset(dtset);
  SpglibSpacegroupType spgtype = spg_get_spacegroup_type(hall_number);
  return spgtype.international_full;
#else
  throw EXCEPTION("Spglib is required for symmetry analisis",ERRDIV);
  std::cout << itime << std::endl; //Useless, kill warning in debug mode
#endif
}

//
HistData* HistData::getHist(const std::string& infile, bool wait){
  HistData *hist = nullptr;
  Exception eloc;
  std::vector<std::pair<std::unique_ptr<HistData>,std::string> > allFormat;

  allFormat.push_back(std::make_pair(std::unique_ptr<HistData>(new HistDataDtset),"Dtset")); //0
  allFormat.push_back(std::make_pair(std::unique_ptr<HistData>(new HistDataNC),"Abinit _HIST"));   //1
  allFormat.push_back(std::make_pair(std::unique_ptr<HistData>(new HistDataHeff),"Heff"));   //2
  allFormat.push_back(std::make_pair(std::unique_ptr<HistData>(new VaspXML),"VaspXML"));     //3
  allFormat.push_back(std::make_pair(std::unique_ptr<HistData>(new Multibinit),"Multibinit"));     //4
  allFormat.push_back(std::make_pair(std::unique_ptr<HistData>(new HistDataXYZ),"XYZ"));     //5
  allFormat.push_back(std::make_pair(std::unique_ptr<HistData>(new HistDataOutNC),"Abinit _OUT.nc"));   //6
  allFormat.push_back(std::make_pair(std::unique_ptr<HistData>(new HistDataGSR),"Abinit _GSR.nc"));   //7
  allFormat.push_back(std::make_pair(std::unique_ptr<HistData>(new HistDataPolyDtset),""));   //8

  std::string file = infile;
  if (file.empty()) throw EXCEPTION("Filename is empty",ERRDIV);
  UriParser uri;
  if ( uri.parse(infile) ) {
    file = uri.getFile();
  }

  if ( file.find("_HIST") != std::string::npos || file.find("_HIST.nc") != std::string::npos ) allFormat[0].swap(allFormat[1]);
  else if ( file.find("_OUT.nc") != std::string::npos ) allFormat[0].swap(allFormat[6]);
  else if ( file.find("vasprun.xml") != std::string::npos ) allFormat[0].swap(allFormat[3]);
  else if ( file.find(".xml") != std::string::npos ) allFormat[0].swap(allFormat[4]);
  else if ( file.find(".xyz") != std::string::npos ) allFormat[0].swap(allFormat[5]);
  else if ( file.find(".dsp") != std::string::npos ) allFormat[0].swap(allFormat[2]);
  else if ( file.find("_GSR.nc") != std::string::npos ) allFormat[0].swap(allFormat[7]);
  else if ( file.find(".nc") != std::string::npos ) allFormat[0].swap(allFormat[8]);
  else if ( file.find("POSCAR") != std::string::npos ) allFormat[0].swap(allFormat[8]);
  else if ( file.find("_DEN") != std::string::npos ) allFormat[0].swap(allFormat[8]);
  else if ( file.find("_POT") != std::string::npos ) allFormat[0].swap(allFormat[8]);
  else if ( file.find("_OPT") != std::string::npos ) allFormat[0].swap(allFormat[8]);
  else if ( file.find(".yaml") != std::string::npos ) allFormat[0].swap(allFormat[8]);

  for ( auto& p : allFormat ) {
    try {
      p.first->readFromFile(file);
      hist = p.first.release();
    }
    catch (Exception &e) {
      hist = nullptr;
      /*#ifdef DEBUG*/
      eloc += e;
      if ( e.getReturnValue() == ERRABT ) {
        break;
      }
      eloc.ADD("Format is not "+p.second,ERRDIV);
    }
    if ( hist != nullptr ) {
      if ( ! p.second.empty() )
        std::clog << "Format is "+p.second << std::endl;
#ifdef HAVE_CPPTHREAD
      if ( wait ) {
        hist->waitTime(hist->_ntime);
      }
#else
      (void) wait;
#endif
      hist->basicChecks();
      return hist;
    }
  }

  eloc.ADD("Failed to build the HistData file",ERRDIV);
  throw eloc;
  return nullptr;
}

HistData& HistData::operator+=(HistData& hist) {
  if ( _nimage > 1 && hist._nimage > 1 && _nimage != hist._nimage )
    throw EXCEPTION("HistData have a different number of images",ERRABT);
  if ( _natom != hist._natom )
    throw EXCEPTION("HistData have a different number of atoms",ERRABT);
  if ( _znucl.size() != hist._znucl.size() )
    throw EXCEPTION("HistData have a different number of znucl",ERRABT);
  if ( _typat.size() != hist._typat.size() )
    throw EXCEPTION("HistData have a different number of typat",ERRABT);


  for ( unsigned z = 0 ; z < _znucl.size() ; ++z ) {
    bool found = false;
    for ( unsigned zo = 0 ; zo < hist._znucl.size() ; ++zo ) {
      if ( _znucl[z] == hist._znucl[zo] )
        found = true;
    }
    if ( !found )
      throw EXCEPTION("New HistData does not have znucl="+utils::to_string(z),ERRABT);
  }

#ifdef HAVE_CPPTHREAD
  // Wait for gathering all data
  this->waitTime(_ntime);
  hist.waitTime(hist._ntime);
#endif

  bool reorder = false;
  std::vector<unsigned> order(_natom,0);

  if (_tryToMap) {
    try {
      order = this->reorder(hist);
      for ( unsigned i=0 ; i < order.size() ; ++i ) {
        if ( i != order[i] ) {
          reorder = true;
          break;
        }
      }
    }
    catch ( Exception &e ){
      e.ADD("Unable to map structures",ERRABT);
    }
  }

  const unsigned prevNtime = _ntime;
  _ntime += hist._ntime;
  _ntimeAvail += hist._ntimeAvail;

  _xcart.resize(_ntime*_natom*_xyz);
  _xred.resize(_ntime*_natom*_xyz);

  bool dofcart;
  bool dospinat;

  dofcart = _fcart.empty() ^ hist._fcart.empty();
  dospinat = hist._spinat.empty() ^ _spinat.empty();

  if ( dofcart && _fcart.empty() ) _fcart.resize(_xyz*_natom*prevNtime,0);
  if ( dofcart && hist._fcart.empty() ) hist._fcart.resize(_xyz*_natom*hist._ntimeAvail,0);

  if ( dospinat && _spinat.empty() ) _spinat.resize(_xyz*_natom*prevNtime,0);
  if ( dospinat && hist._spinat.empty() ) hist._spinat.resize(_xyz*_natom*hist._ntimeAvail,0);

  dofcart = !(_fcart.empty() || hist._fcart.empty());
  dospinat = !(hist._spinat.empty() || _spinat.empty());

  if ( !reorder ) {
    std::copy(hist._xcart.begin(), hist._xcart.end(),_xcart.begin()+prevNtime*_natom*_xyz);
    std::copy(hist._xred.begin(), hist._xred.end(),_xred.begin()+prevNtime*_natom*_xyz);
    if ( dofcart ) {
      _fcart.resize(_ntime*_natom*_xyz,0.);
      std::copy(hist._fcart.begin(), hist._fcart.end(),_fcart.begin()+prevNtime*_natom*_xyz);
    }
    else
      _fcart.clear();
    if ( dospinat ) {
      _spinat.resize(_ntime*_natom*_xyz,0.);
      std::copy(hist._spinat.begin(), hist._spinat.end(),_spinat.begin()+prevNtime*_natom*_xyz);
    }
    else
      _spinat.clear();
  }
  else {
    std::clog << "Reordering:" << std::endl;
    for ( unsigned i = 0; i < order.size() ; ++i )
      std::clog << i << "->" << order[i] << std::endl;
    unsigned start = prevNtime*_natom*_xyz;
    for ( unsigned itime = 0 ; itime < hist._ntime ; ++itime ) {
      for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
        for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
          _xred[start+itime*3*_natom+iatom*3+coord] = hist._xred[itime*3*_natom+order[iatom]*3+coord];
          _xcart[start+itime*3*_natom+iatom*3+coord] = hist._xcart[itime*3*_natom+order[iatom]*3+coord];
        }
      }
    }

    if ( dofcart ) {
      _fcart.resize(_ntime*_natom*_xyz,0.);
      for ( unsigned itime = 0 ; itime < hist._ntime ; ++itime ) {
        for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
          for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
            _fcart[start+itime*3*_natom+iatom*3+coord] = hist._fcart[itime*3*_natom+order[iatom]*3+coord];
          }
        }
      }
    }
    else {
      _fcart.clear();
    }

    if ( dospinat ) {
      _spinat.resize(_ntime*_natom*_xyz,0.);
      for ( unsigned itime = 0 ; itime < hist._ntime ; ++itime ) {
        for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
          for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
            _spinat[start+itime*3*_natom+iatom*3+coord] = hist._spinat[itime*3*_natom+order[iatom]*3+coord];
          }
        }
      }
    }
    else {
      _spinat.clear();
    }

  }

  _acell.resize(_ntime*_xyz);
  std::copy(hist._acell.begin(), hist._acell.end(),_acell.begin()+prevNtime*_xyz);

  _rprimd.resize(_ntime*_xyz*_xyz);
  std::copy(hist._rprimd.begin(), hist._rprimd.end(),_rprimd.begin()+prevNtime*_xyz*_xyz);

  _etotal.resize(_ntime);
  std::copy(hist._etotal.begin(), hist._etotal.end(),_etotal.begin()+prevNtime);

  _time.resize(_ntime);
  std::copy(hist._time.begin(), hist._time.end(),_time.begin()+prevNtime);

  _stress.resize(_ntime*6);
  std::copy(hist._stress.begin(), hist._stress.end(),_stress.begin()+prevNtime*6);

  if ( _nimage > 0 || hist._nimage > 0) {
    if ( _nimage == 0 && hist._nimage == 1 ) {
      _imgdata._acell.resize(_ntime*_xyz);//resize to _ntime to avoid resizing after the if 
      std::copy(_acell.begin(), _acell.begin()+prevNtime*_xyz,_imgdata._acell.begin());

      _imgdata._rprimd.resize(_ntime*9);
      std::copy(_rprimd.begin(), _rprimd.begin()+prevNtime,_imgdata._rprimd.begin());

      _imgdata._etotal.resize(_ntime);
      std::copy(_etotal.begin(), _etotal.begin()+prevNtime,_imgdata._etotal.begin());

      _imgdata._stress.resize(_ntime*6);
      std::copy(_stress.begin(), _stress.begin()+prevNtime,_imgdata._stress.begin());
      _nimage = 1;
      _imgdata._imgmov = hist._imgdata._imgmov;
    }
    else if ( _nimage == 1 && hist._nimage == 0 ) {
      hist._imgdata._acell.resize(hist._ntime*_xyz);
      std::copy(hist._acell.begin(), hist._acell.begin()+hist._ntime*_xyz,hist._imgdata._acell.begin());

      hist._imgdata._rprimd.resize(hist._ntime*9);
      std::copy(hist._rprimd.begin(), hist._rprimd.begin()+hist._ntime,hist._imgdata._rprimd.begin());

      hist._imgdata._etotal.resize(hist._ntime);
      std::copy(hist._etotal.begin(), hist._etotal.begin()+hist._ntime,hist._imgdata._etotal.begin());

      hist._imgdata._stress.resize(hist._ntime*6);
      std::copy(hist._stress.begin(), hist._stress.begin()+hist._ntime,hist._imgdata._stress.begin());
    }
    else if ( _nimage != hist._nimage )
      throw EXCEPTION("You should get this bug",ERRABT);

    _imgdata._acell.resize(_ntime*_nimage*_xyz);
    std::copy(hist._imgdata._acell.begin(), hist._imgdata._acell.end(),_imgdata._acell.begin()+prevNtime*_nimage*_xyz);

    _imgdata._rprimd.resize(_ntime*_nimage*9);
    std::copy(hist._imgdata._rprimd.begin(), hist._imgdata._rprimd.end(),_imgdata._rprimd.begin()+prevNtime*_nimage*9);

    _imgdata._etotal.resize(_ntime*_nimage);
    std::copy(hist._imgdata._etotal.begin(), hist._imgdata._etotal.end(),_imgdata._etotal.begin()+prevNtime*_nimage);

    _imgdata._stress.resize(_ntime*_nimage*6);
    std::copy(hist._imgdata._stress.begin(), hist._imgdata._stress.end(),_imgdata._stress.begin()+prevNtime*_nimage*6);
  }


  if ( (!_isPeriodic) ) this->periodicBoundaries(-1,false);

  //std::clog << "Appending " << hist._ntime << " ionic steps from file " << hist._filename << std::endl;

  return *this;
}

//
double HistData::getDistance(const unsigned iatom1, const unsigned iatom2, const unsigned itime) const {
  using namespace geometry;
  if ( itime > _ntimeAvail )
    throw EXCEPTION("Bad value for itime",ERRDIV);
  if ( iatom1>_natom || iatom2>_natom)
    throw EXCEPTION(std::string("Bad atom id : bigger than ")+utils::to_string(_natom),ERRDIV);
  else if ( iatom1<1 || iatom2<1 )
    throw EXCEPTION("Bad atom id : must not be < 1",ERRDIV);
  const int shift1 = itime*_natom*3+(iatom1-1)*3;
  const int shift2 = itime*_natom*3+(iatom2-1)*3;
  vec3d difference = {{
    _xred[shift2+0]-_xred[shift1+0],
      _xred[shift2+1]-_xred[shift1+1],
      _xred[shift2+2]-_xred[shift1+2]
  }};
  mat3d rprim;
  for ( unsigned i = 0 ; i < 9 ; ++i )
    rprim[i] = _rprimd[itime*9+i];
  if ( geometry::det(rprim) > 1e-6 ) {
    for ( unsigned dim = 0 ; dim < 3 ; ++dim ) {
      while ( difference[dim] > 0.5f ) difference[dim] -= 1.0;
      while ( difference[dim] <= -0.5f ) difference[dim] += 1.0;
    }

    return norm(rprim*difference);
  }
  else 
    return norm(difference);
}


//
double HistData::getAngle(const unsigned iatom1, const unsigned iatom2, const unsigned iatom3, const unsigned itime) const {
  using namespace geometry;
  if ( itime > _ntimeAvail )
    throw EXCEPTION("Bad value for itime",ERRDIV);
  if ( iatom1>_natom || iatom2>_natom || iatom3>_natom)
    throw EXCEPTION(std::string("Bad atom id : bigger than ")+utils::to_string(_natom),ERRDIV);
  else if ( iatom1<1 || iatom2<1 || iatom3<1)
    throw EXCEPTION("Bad atom id : must not be < 1",ERRDIV);
  const int shift1 = itime*_natom*3+(iatom1-1)*3;
  const int shift2 = itime*_natom*3+(iatom2-1)*3;
  const int shift3 = itime*_natom*3+(iatom3-1)*3;
  vec3d vec1 = {{
    _xred[shift2+0]-_xred[shift1+0],
      _xred[shift2+1]-_xred[shift1+1],
      _xred[shift2+2]-_xred[shift1+2]
  }};
  vec3d vec2 = {{
    _xred[shift2+0]-_xred[shift3+0],
      _xred[shift2+1]-_xred[shift3+1],
      _xred[shift2+2]-_xred[shift3+2]
  }};
  mat3d rprim;
  for ( unsigned i = 0 ; i < 9 ; ++i )
    rprim[i] = _rprimd[itime*9+i];

  if ( geometry::det(rprim) > 1e-6 ) {
    for ( unsigned dim = 0 ; dim < 3 ; ++dim ) {
      while ( vec1[dim] > 1.0f )   vec1[dim] -= 1.0;
      while ( vec1[dim] < -1.0f ) vec1[dim] += 1.0;
    }
    for ( unsigned dim = 0 ; dim < 3 ; ++dim ) {
      while ( vec2[dim] > 1.0f )   vec2[dim] -= 1.0;
      while ( vec2[dim] < -1.0f ) vec2[dim] += 1.0;
    }
    vec1 = rprim*vec1;
    vec2 = rprim*vec2;
  }
  return angle(vec1,vec2);
}

std::pair<std::vector<double>,std::vector<double>> HistData::getPDF(unsigned znucl1, unsigned znucl2, double Rmax, double dR, unsigned tbegin, unsigned tend) const {
  using namespace geometry;
  // Check timmes
  try {
    HistData::checkTimes(tbegin,tend);
  }
  catch (Exception &e) {
    e.ADD("Plot function calculations aborted",ERRDIV);
    throw e;
  }

  // Check Rmax compared to dR
  if ( Rmax <= dR )
    throw EXCEPTION("The maximal distance must be greater or equal to the shell thickness",ERRABT);


  // Number of bins
  unsigned nbin = (unsigned) std::ceil(Rmax/dR);
  double Rmax2 = (Rmax+dR)*(Rmax+dR);
  std::vector<double> G(nbin,0.);
  std::vector<double> r(nbin,0.);
  for ( unsigned bin = 0 ; bin < nbin ; ++bin ) r[bin] = (bin+1)*dR;

  double factor = 0.;
  unsigned ntime = tend-tbegin;

#ifdef HAVE_OMP
  unsigned nthread = 1;
#pragma omp parallel 
  {
#pragma omp single 
    {
      nthread = omp_get_num_threads();
    }
  }
#endif

#pragma omp parallel for  schedule(static), if(tend-tbegin>=nthread), reduction(+:factor), firstprivate(Rmax2)
  for ( int itime = tbegin ; itime < (int) tend ; ++itime ) {
    mat3d rprimd;
    std::copy(&_rprimd[itime*3*3],&_rprimd[itime*3*3+9],rprimd.begin());
    std::array<vec3d,3> rprimv;
    rprimv[0] = {{ rprimd[0], rprimd[3], rprimd[6] }};
    rprimv[1] = {{ rprimd[1], rprimd[4], rprimd[7] }};
    rprimv[2] = {{ rprimd[2], rprimd[5], rprimd[8] }};

    const vec3d acell = {{ norm(rprimv[0]), norm(rprimv[1]), norm(rprimv[2]) }};
    const int xShift =  std::max(1,0 + (int) std::floor(2.*Rmax/acell[0]+0.5));
    const int yShift =  std::max(1,0 + (int) std::floor(2.*Rmax/acell[1]+0.5));
    const int zShift =  std::max(1,0 + (int) std::floor(2.*Rmax/acell[2]+0.5));

    Supercell replica(Dtset(*this,itime),xShift,yShift,zShift);
    mat3d rprimdSupercell = replica.rprim();

    // Find all indices for typat1
    std::vector<unsigned> typ1;
    for ( unsigned t = 0 ; t < replica.natom() ; ++t ) {
      if ( (unsigned) _znucl[replica.typat()[t]-1] == znucl1 || znucl1 == 0 ) typ1.push_back(t);
    }
    if ( typ1.size() == 0 )
      continue;
      //throw EXCEPTION("There is no atom of type "+utils::trim(std::string(mendeleev::name[znucl1])),ERRABT);

    // Find all indices for znucl2
    std::vector<unsigned> typ2;
    for ( unsigned t = 0 ; t < replica.natom() ; ++t ) {
      if ( (unsigned) _znucl[replica.typat()[t]-1] == znucl2 || znucl2 == 0 ) typ2.push_back(t);
    }
    if ( typ2.size() == 0 )
      continue;
      //throw EXCEPTION("There is no atom of type "+utils::trim(std::string(mendeleev::name[znucl2])),ERRABT);

    factor += (double)(xShift*yShift*zShift)*det(rprimd)/(double)(typ1.size()*typ2.size());

    const auto &xred = replica.xred();
    std::vector<double> vxred(replica.natom()*3);
    for ( unsigned iatom = 0 ; iatom < replica.natom() ; ++iatom ) {
      vxred[iatom*3  ] = xred[iatom][0];
      vxred[iatom*3+1] = xred[iatom][1];
      vxred[iatom*3+2] = xred[iatom][2];
    }
#ifndef __INTEL_COMPILER
#pragma omp parallel for schedule(dynamic) collapse(2), default(shared), firstprivate(Rmax2)
#endif
    for ( unsigned atom1 = 0 ; atom1 < typ1.size() ; ++atom1 ) {
      for ( unsigned atom2 = 0 ; atom2 < typ2.size() ; ++atom2 ) {
        const unsigned iatom1 = typ1[atom1];
        const unsigned iatom2 = typ2[atom2];
        if ( iatom1 == iatom2 ) continue;

        //vec3d vecRed = {{xred[iatom1][0]-xred[iatom2][0], xred[iatom1][1]-xred[iatom2][1],xred[iatom1][2]-xred[iatom2][2]}};
        vec3d vecRed = {{vxred[iatom1*3+0]-vxred[iatom2*3+0], vxred[iatom1*3+1]-vxred[iatom2*3+1],vxred[iatom1*3+2]-vxred[iatom2*3+2]}};
        for ( unsigned dim = 0 ; dim < 3 ; ++dim ) {
          while ( vecRed[dim] > 0.5 ) vecRed[dim] -= 1.0;
          while ( vecRed[dim] <= -0.5 ) vecRed[dim] += 1.0;
        }
        vec3d vecCart = {{
          rprimdSupercell[0]*vecRed[0]+rprimdSupercell[1]*vecRed[1]+rprimdSupercell[2]*vecRed[2],
            rprimdSupercell[3]*vecRed[0]+rprimdSupercell[4]*vecRed[1]+rprimdSupercell[5]*vecRed[2],
            rprimdSupercell[6]*vecRed[0]+rprimdSupercell[7]*vecRed[1]+rprimdSupercell[8]*vecRed[2]
        }};
        const double R2 = vecCart[0]*vecCart[0]+vecCart[1]*vecCart[1]+vecCart[2]*vecCart[2]; // Tolerance for round error of sqrt
        unsigned bin;
        if ( R2 >=1e-6 && R2 <= Rmax2 && ( (bin = (unsigned) std::floor(std::sqrt(R2)/dR+0.5)-1) < nbin ) ) {
#pragma omp atomic 
          ++G[bin];
        }
      } // typ2
    } // typ1
  } // time

  const double pi4_3 = 4./3.*std::acos(-1.0);
  factor /= (double) ntime*ntime; // One ntime for mean factor, on for mean G
  for ( unsigned bin = 0 ; bin < nbin ; ++bin ) {
    const double R = r[bin]-0.5*dR;
    const double RdR = R+dR;
    G[bin] *= factor/(pi4_3*(RdR*RdR*RdR-R*R*R));
  }
  return std::make_pair(r,G);
}

std::list<std::vector<double>> HistData::getMSD(unsigned tbegin,unsigned tend) const {
  using namespace geometry;

  try {
    HistData::checkTimes(tbegin,tend);
  }
  catch (Exception &e) {
    e.ADD("Plot function calculations aborted",ERRDIV);
    throw e;
  }

  // Count number of each type
  std::vector<int> ntypat(_znucl.size()+1);
  for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom )
    ++ntypat[_typat[iatom]];
  ntypat[0] = _natom;
  
  // Make time
  const unsigned ntime = (tend-tbegin);
  unsigned ntau = ntime/2;
  if ( ntime == 1 ) throw EXCEPTION("Need more than 1 time step",ERRDIV);

  // Remove periodicity
  std::vector<double> fullxred(3*_natom*ntime);
  mat3d rprimd_av = {0.};
  geometry::mat3d chkrprimd;
  std::copy(_rprimd.begin(),_rprimd.begin()+9,&chkrprimd[0]);
  if ( geometry::det(chkrprimd) > 1e-6 ) {
    std::copy(_xred.begin()+(tbegin*3*_natom),_xred.begin()+(tend*3*_natom),fullxred.begin());
    for ( unsigned itime = 1 ; itime < ntime ; ++itime ) {
      for ( unsigned coord = 0 ; coord < 9 ; ++coord )
        rprimd_av[coord] += _rprimd[itime*9+coord];
    }
    for ( unsigned itime = 1 ; itime < ntime ; ++itime ) {
#pragma omp for schedule(static)
      for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
        for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
          const double diff = fullxred[(itime*_natom+iatom)*3+coord]-fullxred[((itime-1)*_natom+iatom)*3+coord] + 0.5;
          fullxred[(itime*_natom+iatom)*3+coord] -= std::floor(diff);
        }
      }
    }
    for ( unsigned coord = 0 ; coord < 9 ; ++coord )
      rprimd_av[coord] /= (double) ntime;
  }
  else {
    rprimd_av[0] = 1;
    rprimd_av[4] = 1;
    rprimd_av[8] = 1;
    std::copy(_xcart.begin()+(tbegin*3*_natom),_xcart.begin()+(tend*3*_natom),fullxred.begin());
  }


  // Result storage not use by threads
  std::list<std::vector<double>> msd(_znucl.size()+1,std::vector<double>(ntau,0.));
  std::vector<std::vector<double>> msd_tmp(ntau,std::vector<double>(_znucl.size()+1,0.));

  std::vector<double>r2(_natom*(ntime+1),0);
  std::vector<double>S1(_natom*ntau,0);
  std::vector<double>S2;

  //Convert to cartesian
#pragma omp parallel for firstprivate(rprimd_av)
  for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      const double a = fullxred[(itime*_natom+iatom)*3+0];
      const double b = fullxred[(itime*_natom+iatom)*3+1];
      const double c = fullxred[(itime*_natom+iatom)*3+2];
      const double carta = rprimd_av[0]*a+rprimd_av[1]*b+rprimd_av[2]*c;
      const double cartb = rprimd_av[3]*a+rprimd_av[4]*b+rprimd_av[5]*c;
      const double cartc = rprimd_av[6]*a+rprimd_av[7]*b+rprimd_av[8]*c;
      fullxred[(itime*_natom+iatom)*3+0] = carta;
      fullxred[(itime*_natom+iatom)*3+1] = cartb;
      fullxred[(itime*_natom+iatom)*3+2] = cartc;
      r2[itime*_natom+iatom] = carta*carta+cartb*cartb+cartc*cartc;
    }
  }

  for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      S1[iatom] += 2*r2[itime*_natom+iatom];
    }
  }

  for ( unsigned itime = 1 ; itime < ntau ; ++itime ) {
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      const double q = S1[(itime-1)*_natom+iatom]-r2[(itime-1)*_natom+iatom]-r2[(ntime-itime)*_natom+iatom];
      S1[itime*_natom+iatom]=q;
    }
  }

  S2 = HistData::acf(fullxred.begin(),fullxred.end(),3*_natom);
  if ( S2.size() != ntau*3*_natom ) 
    throw EXCEPTION("Hmm something is wrong in computing MSD", ERRDIV);

#pragma omp for
  for ( unsigned itime = 0 ; itime < ntau ; ++itime ) {
    const double scaling = 1./(ntime-itime);
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      double s2 = 0;
      for ( unsigned c = 0 ; c < 3 ; ++c )
        s2 += S2[(itime*_natom+iatom)*3+c];
      msd_tmp[itime][_typat[iatom]] += S1[itime*_natom+iatom]*scaling-2*s2;
      msd_tmp[itime][0] += S1[itime*_natom+iatom]*scaling-2*s2;
    }
  }

  // Transpose and scale
  for ( unsigned ityp = 0 ; ityp < _znucl.size()+1 ; ++ityp ) {
    const double natom = ntypat[ityp] ;
    auto lmsd = msd.begin();
    std::advance(lmsd,ityp);
    for ( unsigned itime = 0 ; itime < ntau ; ++itime ) {
      //#pragma omp atomic
      lmsd->at(itime) += (msd_tmp[itime][ityp]/(natom));
    }
  }


    /*
    // Use temporary array in the correct order to reduce memory latencies
    std::vector<std::vector<double>> msd_tmp(ntau,std::vector<double>(_znucl.size()+1,0.));
#pragma omp for schedule(dynamic), firstprivate(rprimd_av)
    for ( unsigned itau = 0 ; itau < ntau ; ++itau ) {
      std::vector<double>diff(2*3*_natom);
      auto &msdtau = msd_tmp[itau];
      for ( unsigned itime = 0 ; itime < ntime-itau ; ++itime ) {
        std::copy(fullxred.begin()+(itime*3*_natom),fullxred.begin()+((itime+1)*3*_natom),diff.begin());
        std::copy(fullxred.begin()+((itime+itau)*3*_natom),fullxred.begin()+((itime+itau+1)*3*_natom),diff.begin()+3*_natom);
        for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
          const double a = diff[iatom*3+0] - diff[3*(_natom+iatom)+0];
          const double b = diff[iatom*3+1] - diff[3*(_natom+iatom)+1];
          const double c = diff[iatom*3+2] - diff[3*(_natom+iatom)+2];
          const double D = a*a+b*b+c*c;
          msdtau[0            ] += D;
          msdtau[_typat[iatom]] += D;
        }
      }
    }
    // Transpose and scale
    for ( unsigned ityp = 0 ; ityp < _znucl.size()+1 ; ++ityp ) {
      const double natom = ntypat[ityp] ;
      auto lmsd = msd.begin();
      std::advance(lmsd,ityp);
      for ( unsigned itau = 0 ; itau < ntau ; ++itau ) {
#pragma omp atomic
        lmsd->at(itau) += (msd_tmp[itau][ityp]/((ntime-itau)*natom));
      }
    }
  }
  */
  return msd;
}

void HistData::printThermo(unsigned tbegin, unsigned tend, std::ostream &out) {
  throw EXCEPTION("Thermodynamics analysis not available for this file",ERRABT);
  (void) out;
  (void) tbegin;
  (void) tend;
}

void HistData::plot(unsigned tbegin, unsigned tend, std::istream &stream, Graph *gplot, Graph::GraphSave save) {
  std::string function;
  Graph::Config config;

  try {
    HistData::checkTimes(tbegin,tend);
  }
  catch (Exception &e) {
    e.ADD("Plot function calculations aborted",ERRDIV);
    throw e;
  }

  stream >> function;

  std::string line;
  size_t pos = stream.tellg();
  std::getline(stream,line);
  stream.clear();
  stream.seekg(pos);
  ConfigParser parser;
  parser.setSensitive(true);
  parser.setContent(line);

  unsigned ntime = tend-tbegin;
  std::vector<double> &x = config.x;
  std::list<std::vector<double>> &y = config.y;
  std::list<std::pair<std::vector<double>,std::vector<double>>> &xy = config.xy;
  std::list<std::string> &labels = config.labels;
  std::vector<unsigned> &colors = config.colors;
  std::string &filename = config.filename;
  std::string &xlabel = config.xlabel;
  std::string &ylabel = config.ylabel;
  std::string &title = config.title;
  //bool &doSumUp = config.doSumUp;

  try {
    std::string tunit = parser.getToken<std::string>("tunit");
    if ( tunit == "fs" ) {
      xlabel = "Time [fs]";
      x.resize(ntime);
      for ( unsigned i = tbegin ; i < tend ; ++i ) x[i-tbegin]=_time[i]*phys::atu2fs;
    }
    else if ( tunit == "step" ) {
      xlabel = "Time [step]";
      x.resize(ntime);
      for ( unsigned i = tbegin ; i < tend ; ++i ) x[i-tbegin]=i;
    }
    else {
      throw EXCEPTION("Unknow time unit, allowed values fs and step",ERRDIV);
    }
  }
  catch (Exception &e) {
    xlabel = "Time [step]";
    x.resize(ntime);
    for ( unsigned i = tbegin ; i < tend ; ++i ) x[i-tbegin]=i;
  }

  std::string sdunit = "bohr";
  if ( parser.hasToken("dunit") )
    sdunit = parser.getToken<std::string>("dunit");
  UnitConverter dunit = UnitConverter::getFromString(sdunit);
  dunit.rebase(UnitConverter::bohr);

  //RDF
  if ( function == "g(r)" ) {
    filename = "PDF";
    xlabel="R["+dunit.str()+"]";
    ylabel = "Radial Distribution Function [a.u]";
    title = "G(r)";
    double rmax = 0;
    double dr = 0;
    stream >> rmax;
    if ( !stream.fail() ) {
      stream >> dr;
      if (stream.fail() ) {
        dr = rmax/1000.;
        stream.clear();
      }
    }
    else {
      stream.clear();
      rmax = geometry::getWignerSeitzRadius(&_rprimd[0]);
      dr = rmax/1000.;
    }
    if ( rmax < 1e-2 || dr < 1e-10 )
      throw EXCEPTION("Rmax or dr is too small. This is either a bad input you set or a histdata without rprimd definition",ERRDIV);
    std::clog << std::endl << " -- Pair Distribution Function --" << std::endl;
    std::clog << " Rmax = " << rmax*dunit << "["+dunit.str()+"]\tdR = " << dr*dunit << "["+dunit.str()+"]" << std::endl << std::endl;;


    const unsigned nznucl =_znucl.size();

    std::pair<std::vector<double>,std::vector<double>> data;

    if ( nznucl > 1 ) {
      data = this->getPDF(0,0,rmax,dr,tbegin,tend);
      y.push_front(std::move(data.second));
      labels.push_front("All atoms");
    }

    for ( unsigned typ1 = 0 ; typ1 < nznucl ; ++typ1 ) {
      for ( unsigned typ2 = typ1 ; typ2 < nznucl ; ++typ2 ) {
        try {
          data = this->getPDF(_znucl[typ1],_znucl[typ2],rmax,dr,tbegin,tend);
          y.push_back(data.second);
          labels.push_back(utils::trim(std::string(mendeleev::name[_znucl[typ1]]))+std::string("-")
              +utils::trim(std::string(mendeleev::name[_znucl[typ2]])));
        }
        catch(Exception& e) {}
      }
    }
    if ( nznucl > 0) x = std::move(data.first);
    for ( auto &xx : x ) xx = xx*dunit;
  }

  else if ( function == "msd" ) {
    filename = "MSD";
    ylabel = "Mean Square Displacement ["+dunit.str()+"^2]";
    title = "MSD";
    std::clog << std::endl << " -- Mean Square Displacement --" << std::endl;

    y = this->getMSD(tbegin,tend);
    for ( auto curve = y.begin() ; curve != y.end() ; ++curve )
      for ( auto &d : *curve ) d = d*dunit*dunit;

    const double dtion = phys::atu2fs*( _time.size() > 1 ? _time[1]-_time[0] : 1);
    x.resize(y.front().size());
    for ( unsigned itime = 0 ; itime < y.front().size() ; ++itime )
      x[itime] = itime*dtion;

    for ( unsigned typ = 0 ; typ < y.size() ; ++typ ) {
      if ( typ == 0 )
        labels.push_back("All");
      else
        labels.push_back(utils::trim(std::string(mendeleev::name[_znucl[typ-1]])));
    }
  }

  // VACF
  else if ( function == "pacf" ){
    filename = "PACF";
    ylabel = "PACF ["+dunit.str()+"^2]";
    title = "PACF";
    std::clog << std::endl << " -- PACF --" << std::endl;

    y = this->getPACF(tbegin,tend);
    for ( auto curve = y.begin() ; curve != y.end() ; ++curve )
      for ( auto &d : *curve ) d = d*dunit*dunit;

    const double dtion = phys::atu2fs*( _time.size() > 1 ? _time[1]-_time[0] : 100);
    x.resize(y.front().size());
    for ( unsigned itime = 0 ; itime < y.front().size() ; ++itime )
      x[itime] = itime*dtion;

    for ( unsigned typ = 0 ; typ < y.size() ; ++typ ) {
      if ( typ == 0 )
        labels.push_back("All");
      else
        labels.push_back(utils::trim(std::string(mendeleev::name[_znucl[typ-1]])));
    }
  }

  // GYRATION factor
  else if ( function == "gyration" ) {
    if ( _nimage < 2 ) throw EXCEPTION("Your data contains only 1 image",ERRDIV);
    filename = "gyration_tensor";
    ylabel = "Gyration radius tensor ["+dunit.str()+"^2]";
    title = "Gyration radius tensor";
    std::clog << std::endl << " -- Gyration radius tensor --" << std::endl;

    y = this->getGyration(tbegin,tend);
    for ( auto curve = y.begin() ; curve != y.end() ; ++curve )
      for ( auto &d : *curve ) d = d*dunit*dunit;

    for ( unsigned typ = 0 ; typ < _znucl.size() ; ++typ ) {
      std::string specie = utils::trim(std::string(mendeleev::name[_znucl[typ]]));
      labels.push_back(specie+" xx");
      labels.push_back(specie+" xy");
      labels.push_back(specie+" xz");
      labels.push_back(specie+" yy");
      labels.push_back(specie+" yz");
      labels.push_back(specie+" zz");
    }
  }

  //POSITIONS
  else if ( function == "positions" ) {
    std::string plan;
    stream >> plan;
    char xaxis=plan[0];
    char yaxis=plan[1];
    unsigned coordx = xaxis-'x';
    unsigned coordy = yaxis-'x';
    auto pos = _xcart.begin();

    filename = std::string("Posisions") + xaxis;
    filename += yaxis;
    xlabel = "Positions X["+dunit.str()+"]";
    ylabel = "Positions Y["+dunit.str()+"]";
    xlabel[10] = xaxis;
    ylabel[10] = yaxis;
    title = "Trajectories (cartesian)";
    if ( stream.fail() || yaxis == xaxis || coordx > 2 || coordy > 2 ) {
      stream.clear();
      throw EXCEPTION("Please specify the plan in which you want the positions (x|y|z)(x|y|z)",ERRDIV);
    }
    std::string coord;
    stream >> coord;
    if ( !stream.fail() ){
      utils::tolower(coord);
      if ( coord == "red" || coord == "reduced" || coord == "xred" ) {
        pos = _xred.begin();
        title = "Trajectories (reduced)";
        xlabel = "Positions X";
        ylabel = "Positions Y";
        xlabel[10] = xaxis;
        ylabel[10] = yaxis;
      }
    }
    else 
      stream.clear();

    std::clog << std::endl << " -- Positions in the " << xaxis << yaxis << " plan --" << std::endl;
    xy.resize(_natom);
    auto p = xy.begin();
    std::vector<bool> alreadySet(_znucl.size(),false);
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom, ++p ) {
      auto typat = _typat[iatom]-1;
      colors.push_back(Graph::rgb(Agate::Mendeleev.color[_znucl[typat]]));
      if ( !alreadySet[typat] ) {
        labels.push_back(mendeleev::name[_znucl[typat]]);
        alreadySet[typat] = true;
      }
      else
        labels.push_back("");
      p->first.resize(tend-tbegin);
      p->second.resize(tend-tbegin);
    }
#pragma omp parallel for private(p)
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      p = xy.begin();
      std::advance(p,iatom);
      for ( unsigned itime = tbegin ; itime < tend ; ++itime ) {
        p->first[itime-tbegin]= pos[(itime*_natom+iatom)*3+coordx]*dunit;
        p->second[itime-tbegin]=pos[(itime*_natom+iatom)*3+coordy]*dunit;
      }
    }
  }

  ///ATOMIC DISTANCE
  else if ( function == "distance" ) {
    int iatom1, iatom2 = 0;
    stream >> iatom1 >> iatom2;
    if ( stream.fail() ) 
      throw EXCEPTION("Need two valid ids",ERRDIV);
    {
      std::stringstream str;
      str << "distance_" << iatom1 
        << "_" << iatom2;
      filename = str.str();
    }
    ylabel = "Distance["+dunit.str()+"]" ;
    title = std::string("Distance ") + utils::to_string(iatom1) + std::string("-") + utils::to_string(iatom2);
    std::clog << std::endl << " -- Distance --" << std::endl;
    std::vector<double> distance(ntime);
#pragma omp parallel for schedule(static)
    for ( unsigned itime = tbegin ; itime < tend ; ++itime ) {
      distance[itime-tbegin] = this->getDistance(iatom1,iatom2,itime);
    }
    y.push_back(std::move(distance));
    for ( auto curve = y.begin() ; curve != y.end() ; ++curve )
      for ( auto &d : *curve ) d = d*dunit;
  }

  ///VOLUME
  else if ( function == "V" ) {
    filename = "volume";
    ylabel = "Volume["+dunit.str()+"^3]";
    title = "Volume";
    std::clog << std::endl << " -- Volume --" << std::endl;
    std::vector<double> volume(ntime);
    std::vector<double> multiplicity(3,1.);
    try {
      multiplicity = parser.getToken<double>("multiplicity",3);
    }
    catch ( Exception &e ) {
      multiplicity.resize(3,1);
    }
    double scaleV = 1.;
    for ( auto m : multiplicity ) scaleV /= m;

#pragma omp parallel for schedule(static)
    for ( unsigned itime = tbegin ; itime < tend ; ++itime ) {
      geometry::mat3d rprimd;
      std::copy(&_rprimd[itime*3*3],&_rprimd[itime*3*3+9],rprimd.begin());
      volume[itime-tbegin] = scaleV*geometry::det(rprimd)*dunit*dunit*dunit;
    }
    y.push_back(std::move(volume));
  }

  ///ACELL 
  else if ( function == "acell" ) {
    filename = "latticeLengths";
    ylabel = "Lattice length ["+dunit.str()+"]";
    title = "Lattice parameters";
    std::clog << std::endl << " -- Lattice parameters --" << std::endl;
    std::vector<double> acell1(ntime);
    std::vector<double> acell2(ntime);
    std::vector<double> acell3(ntime);
    std::vector<double> multiplicity(3,1.);
    try {
      multiplicity = parser.getToken<double>("multiplicity",3);
    }
    catch ( Exception &e ) {
      multiplicity.resize(3,1);
    }
    for ( auto &m : multiplicity ) m = 1/m;

#pragma omp parallel for schedule(static)
    for ( unsigned itime = tbegin ; itime < tend ; ++itime ) {
      const double *rp = &_rprimd[itime*3*3];
      acell1[itime-tbegin] = multiplicity[0]*std::sqrt(rp[0]*rp[0]+rp[3]*rp[3]+rp[6]*rp[6])*dunit;
      acell2[itime-tbegin] = multiplicity[1]*std::sqrt(rp[1]*rp[1]+rp[4]*rp[4]+rp[7]*rp[7])*dunit;
      acell3[itime-tbegin] = multiplicity[2]*std::sqrt(rp[2]*rp[2]+rp[5]*rp[5]+rp[8]*rp[8])*dunit;
    }
    y.push_back(std::move(acell1));
    y.push_back(std::move(acell2));
    y.push_back(std::move(acell3));
    labels.push_back("a");
    labels.push_back("b");
    labels.push_back("c");
  }

  /// Angles
  else if ( function == "angle" ) {
    int iatom1, iatom2, iatom3 = 0;
    ylabel = "Angle[degree]";
    stream >> iatom1 >> iatom2 >> iatom3;
    std::clog << std::endl << " -- Angle --" << std::endl;
    if ( stream.fail() ) {
      stream.clear();
      filename = "angle";
      title = "Angles";
      std::vector<double> alpha(ntime);
      std::vector<double> beta(ntime);
      std::vector<double> gamma(ntime);

#pragma omp parallel for schedule(static)
      for ( unsigned itime = tbegin ; itime < tend ; ++itime ) {
        geometry::mat3d rprim;
        std::copy(&_rprimd[itime*3*3],&_rprimd[itime*3*3+9],&rprim[0]);
        geometry::vec3d angles = geometry::angle(rprim);
        alpha[itime-tbegin]=angles[0];
        beta[itime-tbegin]=angles[1];
        gamma[itime-tbegin]=angles[2];
      }
      y.push_back(std::move(alpha));
      y.push_back(std::move(beta));
      y.push_back(std::move(gamma));
      labels.push_back("alpha");
      labels.push_back("beta");
      labels.push_back("gamma");
    }
    
    else {
      std::stringstream str;
      str << "angle_" << iatom1 
        << "_" << iatom2
        << "_" << iatom3;
      filename = str.str();
      title = std::string("Angle ") + utils::to_string(iatom1) + std::string("-") + utils::to_string(iatom2)
        + std::string("-") + utils::to_string(iatom3);
      std::vector<double> angle(ntime);
#pragma omp parallel for schedule(static)
      for ( unsigned itime = tbegin ; itime < tend ; ++itime ) {
        angle[itime-tbegin] = this->getAngle(iatom1,iatom2,iatom3,itime);
      }
      y.push_back(std::move(angle));
    }
  }

  // Etotal
  else if ( function == "etotal" ) {
    filename = "etotal";
    ylabel = "Etot[Ha]";
    if (_imgdata._imgmov > 0 ) xlabel = "Image";
    title = "Total energy";
    std::clog << std::endl << " -- Total (electronic) energy --" << std::endl;
    y.push_back(std::vector<double>(_etotal.begin()+tbegin,_etotal.end()-(_ntime-tend)));
  }

  else if ( function == "stress" ) {
    filename = "stress";
    ylabel = "Stress [GPa]";
    title = "Stress tensor";
    std::clog << std::endl << " -- Stress tensor --" << std::endl;
    x.resize(_ntime);
    std::vector<double> s1(ntime);
    std::vector<double> s2(ntime);
    std::vector<double> s3(ntime);
    std::vector<double> s4(ntime);
    std::vector<double> s5(ntime);
    std::vector<double> s6(ntime);
    x.resize(_ntime);
#pragma omp parallel for schedule(static)
    for ( unsigned itime = tbegin ; itime < tend ; ++itime ) {
      s1[itime-tbegin] = _stress[itime*6+0]*phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21;
      s2[itime-tbegin] = _stress[itime*6+1]*phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21;
      s3[itime-tbegin] = _stress[itime*6+2]*phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21;
      s4[itime-tbegin] = _stress[itime*6+3]*phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21;
      s5[itime-tbegin] = _stress[itime*6+4]*phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21;
      s6[itime-tbegin] = _stress[itime*6+5]*phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21;
    }
    y.push_back(std::move(s1));
    y.push_back(std::move(s2));
    y.push_back(std::move(s3));
    y.push_back(std::move(s4));
    y.push_back(std::move(s5));
    y.push_back(std::move(s6));
    labels.push_back("sigma 1");
    labels.push_back("sigma 2");
    labels.push_back("sigma 3");
    labels.push_back("sigma 4");
    labels.push_back("sigma 5");
    labels.push_back("sigma 6");
  }


  else {
    throw EXCEPTION(std::string("Function ")+function+std::string(" not available yet"),ERRABT);
  }

  config.save = save;
  try {
    filename = parser.getToken<std::string>("output");
  }
  catch (Exception &e) {
    filename = utils::noSuffix(_filename)+std::string("_")+filename;
  }
  Graph::plot(config,gplot);
  if ( gplot != nullptr )
    gplot->clearCustom();
  stream.clear();
}

void HistData::checkTimes(unsigned tbegin, unsigned tend) const {
  if ( tbegin >= _ntime ) throw EXCEPTION("tbegin cannot be greater that the number of time steps", ERRABT);
  else if ( tbegin >= _ntimeAvail ) throw EXCEPTION("tbegin cannot be greater that the number of available time steps", ERRABT);
  else if ( tend <= tbegin ) throw EXCEPTION("tend <= tbegin not allowed", ERRABT);
  else if ( tend > _ntime ) throw EXCEPTION("tend cannot be greater that the number of time steps", ERRABT);
  else if ( tend > _ntimeAvail ) throw EXCEPTION("tend cannot be greater that the number of available time steps", ERRABT);
}

void HistData::interpolate(unsigned ninter, double amplitude) {
  if ( _nimage > 0 )
    throw EXCEPTION("Interpolation not yet implemented for nimage > 0", ERRDIV);
  this->waitTime(_ntime);
  int nsegment = _ntime-1;
  int newNTime = ninter*nsegment;
  unsigned natom3 = 3*_natom;

  _xcart.resize(natom3*newNTime);
  _xred.resize(natom3*newNTime);
  _acell.resize(_xyz*newNTime);
  _rprimd.resize(_xyz*_xyz*newNTime);
  _etotal.resize(newNTime);
  _time.resize(newNTime);
  _stress.resize(6*newNTime);
  if ( !_fcart.empty() ) _fcart.resize(natom3*newNTime);
  if ( !_spinat.empty() ) _spinat.resize(natom3*newNTime);

  double alpha = (amplitude)/(ninter-1);
  unsigned currentTime = newNTime-1;
  for( int lastStep = _ntime-1 ; lastStep > 0 ; --lastStep ) {
    int firstStep = lastStep-1;
    std::vector<double> xredLast(_xred.begin()+natom3*lastStep,_xred.begin()+natom3*(lastStep+1));
    std::vector<double> xcartLast(_xcart.begin()+natom3*lastStep,_xcart.begin()+natom3*(lastStep+1));
    std::vector<double> spinatLast;
    if (!_spinat.empty()) spinatLast.insert(spinatLast.begin(),_spinat.begin()+natom3*lastStep,_spinat.begin()+natom3*(lastStep+1));
    std::vector<double> fcartLast;
    if (!_fcart.empty()) fcartLast.insert(fcartLast.begin(),_fcart.begin()+natom3*lastStep,_fcart.begin()+natom3*(lastStep+1));

    std::vector<double> acellLast(_acell.begin()+3*lastStep,_acell.begin()+3*(lastStep+1));
    std::vector<double> rprimdLast(_rprimd.begin()+9*lastStep,_rprimd.begin()+9*(lastStep+1));
    std::vector<double> stressLast(_stress.begin()+6*lastStep,_stress.begin()+6*(lastStep+1));
    double etotalLast = _etotal[lastStep];
    double timeLast =_time[lastStep];
    //std::clog << "Between " << firstStep << " and " << lastStep << std::endl;
    for ( unsigned tinter = 0 ; tinter < ninter ; ++tinter ) {
      double beta = tinter*alpha;
      double gamma = 1-beta;
      //std::clog << "point " << tinter << " pour time " << currentTime << " : "  << gamma << "*last+" << beta << "*first" << std::endl;
      for ( unsigned iatomDir = 0 ; iatomDir < natom3 ; ++iatomDir ) {
        _xred[natom3*currentTime+iatomDir] =
            gamma*xredLast[iatomDir]
            +beta*_xred[natom3*firstStep+iatomDir];
        _xcart[natom3*currentTime+iatomDir] =
            gamma*xcartLast[iatomDir]
            +beta*_xcart[natom3*firstStep+iatomDir];
        if ( !_spinat.empty() )
          _spinat[natom3*currentTime+iatomDir] =
              gamma*spinatLast[iatomDir]
              +beta*_spinat[natom3*firstStep+iatomDir];
        if ( !_fcart.empty() )
          _fcart[natom3*currentTime+iatomDir] =
              gamma*fcartLast[iatomDir]
              +beta*_fcart[natom3*firstStep+iatomDir];
      }
      for ( unsigned dir = 0 ; dir < 3 ; ++dir )
        _acell[3*currentTime+dir] =
            gamma*acellLast[dir]+beta*_acell[3*firstStep+dir];
      for ( unsigned dir = 0 ; dir < 9 ; ++dir )
        _rprimd[9*currentTime+dir] =
            gamma*rprimdLast[dir]+beta*_rprimd[9*firstStep+dir];
      for ( unsigned dir = 0 ; dir < 3 ; ++dir )
        _stress[6*currentTime+dir] =
            gamma*stressLast[dir]+beta*_stress[6*firstStep+dir];

      _etotal[currentTime] =
          gamma*etotalLast+beta*_etotal[firstStep];
      _time[currentTime] =
          gamma*timeLast+beta*_time[firstStep];

      --currentTime;
    }
  }
  _ntime = newNTime;
  _ntimeAvail = _ntime;
}

void HistData::dump(const std::string& filename, unsigned tbegin, unsigned tend, unsigned step) const {
  throw EXCEPTION("Dumping not available for this format",ERRDIV);
  (void) filename;
  (void) tbegin;
  (void) tend;
  (void) step;
}

HistData* HistData::average(unsigned tbegin, unsigned tend) {
  this->checkTimes(tbegin, tend);
  HistData *av = new HistDataDtset;

  av->_natom = _natom;
  av->_xyz = _xyz;
  av->_ntime = 1;
  av->_ntimeAvail = 1;

  av->_time.push_back(0);
  av->_xcart.resize(_natom*3,0);
  av->_xred.resize(_natom*3,0);
  if ( !_fcart.empty() ) av->_fcart.resize(_natom*3,0);
  av->_acell.resize(3,0);
  av->_rprimd.resize(9,0);
  av->_etotal.resize(1,0);
  av->_stress.resize(6,0);
  if ( !_spinat.empty() ) av->_spinat.resize(_natom*3,0);

  av->_typat = _typat;
  av->_znucl = _znucl;

#ifdef HAVE_SPGLIB
  av->_symprec = _symprec;
#endif

  av->_filename = _filename;
  double inv_ntime = 1./static_cast<double>(tend-tbegin);

  for ( unsigned time = tbegin ; time < tend ; ++time ) {
    for ( unsigned val = 0 ; val < 3*_natom ; ++val ) {
      av->_xcart[val] += _xcart[time*3*_natom+val]*inv_ntime;
      av->_xred[val] += _xred[time*3*_natom+val]*inv_ntime;
      if ( !_fcart.empty() ) av->_fcart[val] += _fcart[time*3*_natom+val]*inv_ntime;
      if ( !_spinat.empty() ) av->_spinat[val] += _spinat[time*3*_natom+val]*inv_ntime;
    }
    av->_etotal[0] += _etotal[time]*inv_ntime;

    for ( unsigned val = 0 ; val < 3 ; ++val ) 
      av->_acell[val] += _acell[time*3+val]*inv_ntime;

    for ( unsigned val = 0 ; val < 9 ; ++val ) 
      av->_rprimd[val] += _rprimd[time*9+val]*inv_ntime;

    for ( unsigned val = 0 ; val < 6 ; ++val ) 
      av->_stress[val] += _stress[time*6+val]*inv_ntime;
  }
  return av;
}

bool HistData::getTryToMap() const
{
  return _tryToMap;
}

void HistData::setTryToMap(bool tryToMap)
{
  _tryToMap = tryToMap;
}

std::vector<double> HistData::acf(std::vector<double>::const_iterator begin, std::vector<double>::const_iterator end, const int howmany) {
  const int ntime = std::distance(begin,end)/howmany;
  unsigned ntau = ntime/2;
  if ( std::distance(begin,end) != howmany*ntime ) throw EXCEPTION("Vector has not the expected size",ERRDIV);
  if ( ntime == 1 ) throw EXCEPTION("Need more than 1 time step",ERRDIV);
#ifdef HAVE_FFTW3
  const int n  = 2*ntime-1;
  const double inv_vol = 1./(double)n;
  const int nh = n/2+1;
  const int idist = 1; const int odist = 1;
  const int *inembed = NULL; const int *onembed = NULL;
  const int istride = howmany; const int ostride = howmany;

  double *fft_in;
  fftw_complex *fft_out;
  fftw_plan plan_forward;
  fftw_plan plan_backward;

  fft_in = (double*) fftw_malloc( sizeof ( double ) * n * howmany);
  fft_out = (fftw_complex*) fftw_malloc( sizeof ( fftw_complex ) * nh * howmany );

#pragma omp parallel for
  for ( int u = 0 ; u < ntime * howmany ; ++u ) {
    fft_in[u] = begin[u];
  }
#pragma omp parallel for
  for ( int u = ntime * howmany ; u < n * howmany ; ++u ) {
    fft_in[u] = 0;
  }

#if defined(HAVE_OMP) && defined(HAVE_FFTW3_THREADS)
  fftw_plan_with_nthreads(omp_get_max_threads());
#endif

#pragma omp critical (acf_fft)
  {
    plan_forward = fftw_plan_many_dft_r2c(1, &n, howmany, 
        fft_in, inembed, istride, idist, 
        fft_out, onembed, ostride, odist, FFTW_ESTIMATE);
    plan_backward = fftw_plan_many_dft_c2r(1, &n, howmany, 
        fft_out, onembed, ostride, odist, 
        fft_in, inembed, istride, idist, FFTW_ESTIMATE);
  }

  fftw_execute(plan_forward);
  //Compute PSD
  for ( int u = 0 ; u < nh * howmany ; ++u ) {
    fft_out[u][0]=(fft_out[u][0]*fft_out[u][0]+fft_out[u][1]*fft_out[u][1])*inv_vol;
    fft_out[u][1]=0;
  }
  fftw_execute(plan_backward);
  std::vector<double> acf(&fft_in[0],&fft_in[ntau*howmany]);

#pragma omp critical (acf_fft)
  {
    fftw_destroy_plan(plan_forward);
    fftw_destroy_plan(plan_backward);
  }
  fftw_free(fft_in);
  fftw_free(fft_out);

#else
  std::vector<double> acf(ntau*howmany,0);
  for ( unsigned itau = 0 ; itau < ntau ; ++itau ) {
    auto acftau = acf.begin()+itau*howmany;
    for ( unsigned itime = 0 ; itime < ntime-itau ; ++itime ) {
      auto tab = begin+itime*howmany;
      auto tabtau = begin+(itime+itau)*howmany;
      for ( int i = 0 ; i < howmany ; ++i ) {
        acftau[i] += tab[i] * tabtau[i];
      }
    }
  }
#endif

  for ( unsigned itau = 0 ; itau < ntau ; ++itau ) {
    for ( int i = 0 ; i < howmany ; ++i ) {
      acf[itau*howmany+i] /= (double)(ntime-itau);
    }
  }
  return acf;
}

//
void HistData::periodicBoundaries(unsigned iitime, bool toPeriodic) {
  bool allTimes = (iitime == (unsigned) -1);
  if ( !allTimes && iitime >= _ntime )
    throw EXCEPTION("Out of range for time",ERRDIV);
  if ( !toPeriodic && iitime == 0 ) return;

  unsigned begin = (allTimes ? 0 : iitime );
  unsigned end = (allTimes ? _ntime : iitime+1 );

  this->waitTime(end);

  geometry::mat3d chkrprimd;
  std::copy(_rprimd.begin(),_rprimd.begin()+9,&chkrprimd[0]);
  if ( geometry::det(chkrprimd) < 1e-6 ) 
    throw EXCEPTION("No translation for this file, cannot change boundaries",ERRDIV);

  if ( toPeriodic ) {
    // Impose periodicity
    for ( unsigned itime = begin ; itime < end ; ++itime ) {
      double * xredT = &_xred[itime*_natom*3];
#pragma omp for schedule(static)
      for (unsigned iatom = 0 ; iatom < _natom ; ++iatom) {
        for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
          while ( xredT[iatom*3+coord] >= 1.0 ) xredT[iatom*3+coord] -= 1.0;
          while ( xredT[iatom*3+coord] < 0 ) xredT[iatom*3+coord] += 1.0;
        }
      }
    }
    if ( allTimes ) _isPeriodic = true;
  }
  else {
    // Remove periodicity
    for ( unsigned itime = (allTimes ? 1 : begin); itime < end ; ++itime ) {
#pragma omp for schedule(static)
      for (unsigned iatom = 0 ; iatom < _natom ; ++iatom) {
        for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
          const double diff = _xred[(itime*_natom+iatom)*3+coord]-_xred[((itime-1)*_natom+iatom)*3+coord] + 0.5;
          _xred[(itime*_natom+iatom)*3+coord] -= std::floor(diff);
        }
      }
    }
    if ( allTimes ) _isPeriodic = false;
  }
#pragma omp for schedule(static)
  for ( unsigned itime = begin ; itime < end ; ++itime ) {
    for (unsigned iatom = 0 ; iatom < _natom ; ++iatom) {
      _xcart[itime*3*_natom+iatom*3  ] = _rprimd[itime*9+0]*_xred[itime*3*_natom+iatom*3] + _rprimd[itime*9+1]*_xred[itime*3*_natom+iatom*3+1] + _rprimd[itime*9+2]*_xred[itime*3*_natom+iatom*3+2];
      _xcart[itime*3*_natom+iatom*3+1] = _rprimd[itime*9+3]*_xred[itime*3*_natom+iatom*3] + _rprimd[itime*9+4]*_xred[itime*3*_natom+iatom*3+1] + _rprimd[itime*9+5]*_xred[itime*3*_natom+iatom*3+2];
      _xcart[itime*3*_natom+iatom*3+2] = _rprimd[itime*9+6]*_xred[itime*3*_natom+iatom*3] + _rprimd[itime*9+7]*_xred[itime*3*_natom+iatom*3+1] + _rprimd[itime*9+8]*_xred[itime*3*_natom+iatom*3+2];
    }
  }
}

//
void HistData::centroid() {
  if ( _nimage < 2 ) 
    throw EXCEPTION("There is no image in this file",ERRCOM);
  this->waitTime(_ntime);
  double inv_nimage = 1./_nimage;
  _natom /= _nimage;
  _typat.resize(_natom);

  std::vector<double> xcart (_ntime*_natom*3,0);
  std::vector<double> xred  (_ntime*_natom*3,0);

#pragma omp for schedule(static)
  for ( unsigned itime = 0 ; itime < _ntime ; ++itime ) {
    for ( unsigned img = 0 ; img < _nimage ; ++img ) {
      for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
        for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
          const unsigned center = itime*_natom*3+iatom*3+coord;
          const unsigned myimg = itime*_natom*_nimage*3+_natom*img*3+iatom*3+coord;
          xcart [center] += _xcart [myimg]*inv_nimage;
          xred  [center] += _xred  [myimg]*inv_nimage;
        }
      }
    }
  } 
  _xcart = std::move(xcart);
  _xred  = std::move(xred );

  if ( _fcart.size() == _ntime*3*_natom*_nimage ) {
    std::vector<double> fcart (_ntime*_natom*3,0);
#pragma omp for schedule(static)
    for ( unsigned itime = 0 ; itime < _ntime ; ++itime ) {
      for ( unsigned img = 0 ; img < _nimage ; ++img ) {
        for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
          for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
            const unsigned center = itime*_natom*3+iatom*3+coord;
            const unsigned myimg = itime*_natom*_nimage*3+_natom*img*3+iatom*3+coord;
            fcart [center] += _fcart [myimg]*inv_nimage;
          }
        }
      }
    } 
    _fcart = std::move(fcart);
  }

  if ( _spinat.size() == _ntime*3*_natom*_nimage ) {
    std::vector<double> spinat(_ntime*_natom*3,0);
#pragma omp for schedule(static)
    for ( unsigned itime = 0 ; itime < _ntime ; ++itime ) {
      for ( unsigned img = 0 ; img < _nimage ; ++img ) {
        for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
          for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
            const unsigned center = itime*_natom*3+iatom*3+coord;
            const unsigned myimg = itime*_natom*_nimage*3+_natom*img*3+iatom*3+coord;
            spinat[center] += _spinat[myimg]*inv_nimage;
          }
        }
      }
    } 
    _spinat= std::move(spinat);
  }
  _nimage = 1;
}

std::list<std::vector<double>> HistData::getGyration(unsigned tbegin,unsigned tend) const {
  if ( _nimage < 2 ) 
    throw EXCEPTION("Need more than 1 image",ERRDIV);
  unsigned natomImg = _natom/_nimage;
  double inv_nimage = 1./_nimage;
  std::list<std::vector<double>> gyra(_znucl.size()*6,std::vector<double>(tend-tbegin));

  std::vector<double> ntypat(_znucl.size(),0);
  // Compute _nimage*natomImg typat
  for ( auto typat : _typat )
    ++ntypat[typat-1];

#pragma omp for schedule(static)
  for ( unsigned t = tbegin ; t < tend ; ++t ) {
    //Compute centroid
    std::vector<double> rc(natomImg*_xyz,0);
    const double *xcart = &_xcart[t*_natom*_xyz];
    for ( unsigned s = 0 ; s < _nimage ; ++s ) {
      const double *xcarts  = &xcart[s*natomImg*_xyz];
      for ( unsigned i = 0 ; i < natomImg ; ++i ) {
        for ( unsigned c = 0 ; c < _xyz ; ++c ) {
          rc[i*_xyz+c] += xcarts[i*_xyz+c] * inv_nimage;
        }
      }
    }

    std::vector<double> tmp(natomImg*6,0);
    for ( unsigned iatom = 0 ; iatom < natomImg ; ++ iatom ) {
      for ( unsigned s = 0 ; s < _nimage ; ++s ) {
        const double *xcarts  = &xcart[s*natomImg*_xyz];
        int indice = 0;
        for ( unsigned c1 = 0 ; c1 < 3 ; ++c1 ) {
          const double diff1 = (xcarts[iatom*3+c1] - rc[iatom*3+c1]);
          for ( unsigned c2 = c1 ; c2 < 3 ; ++ c2 ) {
            double g = diff1 * (xcarts[iatom*3+c2] - rc[iatom*3+c2]) * inv_nimage;
            tmp[iatom*6+(indice++)] += g;
          }
        }
      }
    }

    for ( unsigned iatom = 0 ; iatom < natomImg ; ++ iatom ) {
      unsigned typat = _typat[iatom]-1;
      double inv_ntypat = _nimage/ntypat[typat];
      auto g = gyra.begin();
      std::advance(g,typat*6);
      for ( unsigned xy = 0 ; xy < 6 ; ++xy ) {
        double contrib = tmp[iatom*6+xy]*inv_ntypat;
        g->at(t-tbegin) += contrib;
        std::advance(g,1);
      }
    }
  }
  return gyra;
}

//
void HistData::moveAtom(unsigned itime, unsigned iatom, double x, double y, double z) {
  if ( itime >= _ntimeAvail )
    throw EXCEPTION("Out of range for time",ERRDIV);
  if ( iatom >= _natom )
    throw EXCEPTION("This atom does not exist",ERRDIV);

  while ( x >= 1 ) --x;
  while ( y >= 1 ) --y;
  while ( z >= 1 ) --z;
  while ( x < 0 ) ++x;
  while ( y < 0 ) ++y;
  while ( z < 0 ) ++z;

  _xred[itime*3*_natom+iatom*3  ] = x;
  _xred[itime*3*_natom+iatom*3+1] = y;
  _xred[itime*3*_natom+iatom*3+2] = z;

  _xcart[itime*3*_natom+iatom*3  ] = _rprimd[itime*9+0]*x + _rprimd[itime*9+1]*y + _rprimd[itime*9+2]*z;
  _xcart[itime*3*_natom+iatom*3+1] = _rprimd[itime*9+3]*x + _rprimd[itime*9+4]*y + _rprimd[itime*9+5]*z;
  _xcart[itime*3*_natom+iatom*3+2] = _rprimd[itime*9+6]*x + _rprimd[itime*9+7]*y + _rprimd[itime*9+8]*z;

}

void HistData::typeAtom(unsigned iatom, unsigned typat) {
  if ( iatom >= _natom )
    throw EXCEPTION("Invalid atom id",ERRDIV);
  if ( typat == 0 || typat > _znucl.size() )
    throw EXCEPTION("Invalid type of atom",ERRDIV);
  _typat[iatom] = typat;
}

void HistData::shiftOrigin(unsigned itime, double x, double y, double z) {
  bool allTimes = (itime == (unsigned) -1);
  if ( !allTimes && itime >= _ntime )
    throw EXCEPTION("Out of range for time",ERRDIV);

  unsigned begin = (allTimes ? 0 : itime );
  unsigned end = (allTimes ? _ntime : itime+1 );

  this->waitTime(end);

  for ( unsigned iitime = begin ; iitime < end ; ++iitime ) {
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      _xred[iitime*3*_natom+iatom*3  ] -= x;
      _xred[iitime*3*_natom+iatom*3+1] -= y;
      _xred[iitime*3*_natom+iatom*3+2] -= z;

      _xcart[iitime*3*_natom+iatom*3  ] -= _rprimd[iitime*9+0]*x + _rprimd[iitime*9+1]*y + _rprimd[iitime*9+2]*z;
      _xcart[iitime*3*_natom+iatom*3+1] -= _rprimd[iitime*9+3]*x + _rprimd[iitime*9+4]*y + _rprimd[iitime*9+5]*z;
      _xcart[iitime*3*_natom+iatom*3+2] -= _rprimd[iitime*9+6]*x + _rprimd[iitime*9+7]*y + _rprimd[iitime*9+8]*z;
    }
  }
}

//
std::list<std::vector<double>> HistData::getPACF(unsigned tbegin, unsigned tend) const {
  double inv_ntime = 1./static_cast<double>(tend-tbegin);
  const unsigned ntime = tend-tbegin;

  // Remove periodicity
  std::vector<double> fullxred(3*_natom*ntime);
  geometry::mat3d rprimd_av = {0.};
  geometry::mat3d chkrprimd;
  std::copy(_rprimd.begin(),_rprimd.begin()+9,&chkrprimd[0]);
  if ( geometry::det(chkrprimd) > 1e-6 ) {
    std::copy(_xred.begin()+(tbegin*3*_natom),_xred.begin()+(tend*3*_natom),fullxred.begin());
    for ( unsigned itime = 1 ; itime < ntime ; ++itime ) {
      for ( unsigned coord = 0 ; coord < 9 ; ++coord )
        rprimd_av[coord] += _rprimd[itime*9+coord];
    }
#pragma omp parallel for collapse(3)
    for ( unsigned itime = 1 ; itime < ntime ; ++itime ) {
      for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
        for ( unsigned coord = 0 ; coord < 3 ; ++coord ) {
          const double diff = fullxred[(itime*_natom+iatom)*3+coord]-fullxred[((itime-1)*_natom+iatom)*3+coord] + 0.5;
          fullxred[(itime*_natom+iatom)*3+coord] -= std::floor(diff);
        }
      }
    }
    for ( unsigned coord = 0 ; coord < 9 ; ++coord )
      rprimd_av[coord] *= inv_ntime;
  }
  else {
    rprimd_av[0] = 1;
    rprimd_av[4] = 1;
    rprimd_av[8] = 1;
    std::copy(_xcart.begin()+(tbegin*3*_natom),_xcart.begin()+(tend*3*_natom),fullxred.begin());
  }


  std::vector<double> average(3*_natom,0); // time == tbegin
#pragma omp parallel for collapse(2)
  for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      const double a = fullxred[(itime*_natom+iatom)*3+0];
      const double b = fullxred[(itime*_natom+iatom)*3+1];
      const double c = fullxred[(itime*_natom+iatom)*3+2];
      const double carta = rprimd_av[0]*a+rprimd_av[1]*b+rprimd_av[2]*c;
      const double cartb = rprimd_av[3]*a+rprimd_av[4]*b+rprimd_av[5]*c;
      const double cartc = rprimd_av[6]*a+rprimd_av[7]*b+rprimd_av[8]*c;
      fullxred[(itime*_natom+iatom)*3+0] = carta;
      fullxred[(itime*_natom+iatom)*3+1] = cartb;
      fullxred[(itime*_natom+iatom)*3+2] = cartc;
    }
  }
#pragma omp parallel 
  {
    std::vector<double> averageLoc(3*_natom,0); // time == tbegin
#pragma omp for
    for ( unsigned itime = 0 ; itime < ntime ; ++itime ) {
      for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
        averageLoc[iatom*3+0] += fullxred[(itime*_natom+iatom)*3+0]*inv_ntime;
        averageLoc[iatom*3+1] += fullxred[(itime*_natom+iatom)*3+1]*inv_ntime;
        averageLoc[iatom*3+2] += fullxred[(itime*_natom+iatom)*3+2]*inv_ntime;
      }
    }
#pragma omp critical
    {
      for ( unsigned i = 0 ; i < 3*_natom ; ++ i) 
        average[i] += averageLoc[i];
    }
  }

#pragma omp parallel for
  for ( unsigned time = 0 ; time < ntime ; ++time ) {
    for ( unsigned val = 0 ; val < 3*_natom ; ++val ) {
      fullxred[time*3*_natom+val] -= average[val];
    }
  }

  auto begin = fullxred.begin();
  std::advance(begin,tbegin*3*_natom);
  auto end = begin;
  std::advance(end,ntime*3*_natom);

  std::vector<double> fullpacf;
  try {
    fullpacf = HistData::acf(begin,end,3*_natom);
  }
  catch ( Exception &e ) {
    e.ADD("PACF calculation failed",ERRDIV);
    throw e;
  }
  const unsigned ntau = fullpacf.size()/(3*_natom);

  // Count number of each type
  std::vector<int> ntypat(_znucl.size()+1);
  for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom )
    ++ntypat[_typat[iatom]];
  ntypat[0] = _natom;

  std::vector<std::vector<double>> pacf_tmp(ntau,std::vector<double>(_znucl.size()+1,0.));
#pragma omp parallel for
  for ( unsigned itau = 0 ; itau < ntau ; ++itau ) {
    auto &pacftau = pacf_tmp[itau];
    const double shift = itau*3*_natom;
    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      for ( unsigned c = 0 ; c < 3 ; ++c ) {
        pacftau[0] += fullpacf[shift+iatom*3+c];
        pacftau[_typat[iatom]] += fullpacf[shift+iatom*3+c];
      }
    }
  }

  std::list<std::vector<double>> pacf(_znucl.size()+1,std::vector<double>(ntau,0));
  for ( unsigned ityp = 0 ; ityp < _znucl.size()+1 ; ++ityp ) {
    const double natom_3 = 3*ntypat[ityp] ;
    auto lpacf = pacf.begin();
    std::advance(lpacf,ityp);
    for ( unsigned itau = 0 ; itau < ntau ; ++itau ) {
      lpacf->at(itau) += pacf_tmp[itau][ityp]/(natom_3);
    }
  }
  return pacf;
}

//
void HistData::decorrelate(unsigned tbegin, unsigned tend, unsigned ntime, double T, double mu, unsigned step) {
  try {
    HistData::checkTimes(tbegin,tend);
  }
  catch (Exception &e) {
    e.ADD("decorrelate calculations aborted",ERRDIV);
    throw e;
  }

  if ( ntime > tend-tbegin )
    throw EXCEPTION("Not enough time step in between tbegin and tend",ERRDIV);

  if ( T <= 0 )
    throw EXCEPTION("T should be positive",ERRDIV);

  if ( mu <= 0 || mu >= 1 )
    throw EXCEPTION("mu should be in between 0 and 1",ERRDIV);


  std::vector<unsigned> decorrelateTimes(ntime);
  std::default_random_engine generator;
  std::uniform_int_distribution<unsigned> distribution(tbegin,tend-1);
  for ( unsigned i = 0 ; i < ntime ; ++i ) {
    unsigned time;
    do {
      time = distribution(generator);
    }
    while( std::find(decorrelateTimes.begin(), decorrelateTimes.end(),time) != decorrelateTimes.end() );
    decorrelateTimes[i] = time;
  }
  std::sort(decorrelateTimes.begin(),decorrelateTimes.end());
  
  // Build a fake Hist
  HistData *hist = new HistDataDtset;
  hist->_xred.resize(3*_natom*ntime);
  hist->_xcart.resize(3*_natom*ntime);
  hist->_rprimd.resize(9*ntime);
  hist->_time.resize(ntime);
  hist->_typat = _typat;
  hist->_znucl = _znucl;
  hist->_natom = _natom;
  hist->_ntime = ntime;
  hist->_ntimeAvail = ntime;
  hist->_xyz = 3;


  auto setHist = [this] (HistData* hist, unsigned localTime, unsigned globalTime ) {
    std::copy(_rprimd.begin()+globalTime*9,_rprimd.begin()+(globalTime+1)*9,hist->_rprimd.begin()+localTime*9);
    std::copy(_xred.begin()+globalTime*3*_natom,_xred.begin()+(globalTime+1)*3*_natom,hist->_xred.begin()+localTime*3*_natom);
    std::copy(_xcart.begin()+globalTime*3*_natom,_xcart.begin()+(globalTime+1)*3*_natom,hist->_xcart.begin()+localTime*3*_natom);
  };

  auto computeE = [] (HistData* hist) -> double {
    auto y = hist->getPACF(0,hist->_ntime);
    // Only use the all PACF (which is 0)
    auto& pacf = y.front();
    double sum = 0.0;
    double renorm = 1./std::abs(pacf[0]);
    for ( auto a : pacf )  {
      sum += std::abs(a);
    }
    return std::abs(1.-sum*renorm);
  };

  for ( unsigned i = 0 ; i < decorrelateTimes.size() ; ++i ) {
    hist->_time[i] = i;
    setHist(hist,i,decorrelateTimes[i]);
  }
  std::istringstream toto("pacf");
  double Tl = 0.05*computeE(this)*(double)ntime/(double)_ntime;
  std::cerr << "T initiale " << Tl << std::endl;
  double E0 = computeE(hist);
  double E = E0;
  std::cerr << "E initiale " << E0 << std::endl;
  std::uniform_int_distribution<unsigned> decorrDistrib(0,ntime-1);
  std::uniform_real_distribution<double> unifor(0.,1.);
  unsigned k = 0;

  Graph::Config config;
  config.xlabel = "Step";
  config.ylabel = "Energy";
  config.doSumUp = false;
  config.save = Graph::GraphSave::PRINT;
  std::vector<double> &x = config.x;
  config.y.push_back(std::vector<double>());
  std::vector<double> &y = config.y.back();
  config.y.push_back(std::vector<double>());
  std::vector<double> &t = config.y.back();
  config.y.push_back(std::vector<double>());
  std::vector<double> &p = config.y.back();
  x.push_back(0);
  y.push_back(E0);
  t.push_back(10*Tl);
  p.push_back(1);
    
  Gnuplot gplot;

  bool stop = false;
  unsigned constant = 0;

  while ( !stop ) {
    for ( unsigned i = 0 ; i < step ; ++i ) {
      const unsigned toRemove = decorrDistrib(generator);
      const unsigned timeRemoved = decorrelateTimes[toRemove];
      const unsigned timeBefore = ( toRemove > 0 ? decorrelateTimes[toRemove-1] : 0 );
      const unsigned timeAfter = ( toRemove < ntime-1 ? decorrelateTimes[toRemove+1] : _ntime-1 );
      std::uniform_int_distribution<unsigned> localDistrib(timeBefore,timeAfter);
      const unsigned toAdd = localDistrib(generator);
      setHist(hist,toRemove,toAdd);
      E = computeE(hist);
      const double rand = unifor(generator);
      const double proba = std::min(1.,std::exp(-(E-E0)/Tl));
      if ( proba > rand ) {
        E0 = E;
        constant = 0;
      }
      else {
        setHist(hist,toRemove,timeRemoved);
        ++constant;
      }
      x.push_back(++k);
      y.push_back(E0);
      t.push_back(10*Tl);
      p.push_back(proba);
      Graph::plot(config,&gplot);
      if ( constant > 200 ) stop = true;
    }
    Tl *= mu;
  }
  config.filename = "MC";
  config.save = Graph::GraphSave::DATA;
  Graph::plot(config,&gplot);
  std::cerr << "E finale " << E0 << std::endl;
  if ( stop ) std::cerr << "Converged" << std::endl;
  hist->plot(0,decorrelateTimes.size(),toto,&gplot, Graph::GraphSave::DATA);
  delete hist;
}

//
std::vector<unsigned> HistData::reorder(const HistData &hist) const {
  using namespace geometry;
  if ( hist._natom != _natom )
    throw EXCEPTION("Bad number of atoms",ERRABT);

  std::vector<unsigned> order(_natom,0);

  /* !!! We work at itime = 0 only for both */

  mat3d rprim;
  for ( unsigned i = 0 ; i < 9 ; ++i )
    rprim[i] = _rprimd[i];

  unsigned nimage = std::max(_nimage,(unsigned)1);
  unsigned natom = _natom/nimage;
  for ( unsigned image = 0 ; image < nimage ; ++image ) {
#pragma omp parallel for schedule(dynamic)
    for ( unsigned mmatom = 0 ; mmatom < natom ; ++mmatom ) {
      unsigned matom = image*natom+mmatom;
      double closest = 9999999999;
      bool testz = false;
      for ( unsigned ooatom = 0 ; ooatom < natom ; ++ooatom ) {
      unsigned oatom = image*natom+ooatom;
        if ( _znucl[_typat[matom]-1] != hist._znucl[hist._typat[oatom]-1] ) continue;
        testz = true;
        geometry::vec3d difference = {{
          hist._xred[oatom*3  ]-_xred[matom*3  ],
            hist._xred[oatom*3+1]-_xred[matom*3+1],
            hist._xred[oatom*3+2]-_xred[matom*3+2],
        }};
        for ( unsigned i = 0 ; i < 3 ; ++i ) {
          while ( difference[i] < 0.5 ) ++difference[i];
          while ( difference[i] >= 0.5 ) --difference[i];
        }
        auto diffcart = rprim * difference;
        double distance = norm(diffcart);
        if ( distance < closest ) {
#pragma omp critical 
          {
            order[matom] = oatom;
            closest = distance;
          }
        }
      }
      if ( !testz )
        throw EXCEPTION("Cannot find a correct typat for atom "+utils::to_string(matom)+" in the appended HistData",ERRABT);
    }
  }
  //bool reorder = false;
  unsigned sum = 0;
  for ( unsigned oatom = 0 ; oatom < _natom ; ++oatom ) {
    sum += order[oatom];
    /*
    if ( order[oatom] != oatom ) {
      reorder = true;
    }
    */
  }
  if ( sum != (_natom*(_natom-1)/2) )
    throw EXCEPTION("Bad reordering",ERRABT);
  //if ( !reorder ) order.clear();
  return order;
}

//
HistData::ImgData::ImgData() :
  _imgmov(0),
  _acell(),
  _rprimd(),
  _etotal(),
  _stress()
{
  ;
}

//
void HistData::ImgData::clear() {
  _imgmov = 0;
  _acell.clear(); 
  _rprimd.clear();
  _etotal.clear();
  _stress.clear();
}

//
void HistData::ImgData::resize(unsigned n) {
  _acell.resize(3*n);
  _rprimd.resize(9*n);
  _etotal.resize(n);
  _stress.resize(6*n);
}

//
void HistData::basicChecks() {
  /*
   * This depends on time but time steps can be loaded asynchroneously ... so don't deal now with this
  for (auto a : _acell ) {
    if ( a < 0 ) throw EXCEPTION("acell contains a negative value.",ERRDIV);
  }

  for (unsigned i = 0 ; i < _rprimd.size() ; i+=9) {
    if (geometry::det(&_rpimd[i]) <= 0 )
      throw EXCEPTION("A negative det for rprimd has been found.", ERRDIV);
  }
  */
  
  for (auto t : _typat) {
    if ( t < 1 || t > (int)_znucl.size() )
      throw EXCEPTION("typat is out of range [1;ntypat]",ERRDIV);
  }
  
  for(auto z : _znucl) {
    if ( z < 0 || z > 119 )
      throw EXCEPTION("znucl is out of [0;119]",ERRDIV);
  }
}

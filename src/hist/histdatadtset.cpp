/**
 * @file src/histdatadtset.cpp
 *
 * @brief  Read a abinit or CIF or POSCAR dtset and convert it to a HistData 
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


#include "hist/histdatadtset.hpp"
#include "base/exception.hpp"
#include "io/dtset.hpp"
#include "io/poscar.hpp"
#include "io/configparser.hpp"
#include <fstream>

//
HistDataDtset::HistDataDtset() : HistData(){
  ;
}

HistDataDtset::HistDataDtset(const Dtset& dtset) {
  this->buildFromDtset(dtset);
  _nimage = 1;
}

//
HistDataDtset::~HistDataDtset() {
  ;
}

//
void HistDataDtset::readFromFile(const std::string& filename) {
  Dtset *dtset = nullptr;
  try{
    Exception ec;
    dtset = new Dtset;
    ConfigParser parser(filename);
    unsigned nimage = 0;
    unsigned imgmov = 0;
    try {
      parser.parse();
      try {
        nimage = parser.getToken<unsigned>(" nimage");
      }
      catch ( Exception &e ) {
        nimage = 1;
        (void)e;
      }
      if ( nimage > 1 ) {
        try {
          imgmov = parser.getToken<unsigned>("imgmov");
        }
        catch ( Exception &e ) {
          imgmov = 13;
          (void)e;
        }
      }
      dtset->readConfig(parser);
    }
    catch( Exception& e ) {
      ec+= e;
      ec.ADD("Not an Abinit dtset",e.getReturnValue());
      if ( e.getReturnValue() == ERRABT ) throw ec;
      try{
        dtset->setCif(filename);
        dtset->reBuildStructure(0.0001,false);
      }
      catch( Exception& ee){
        ec += ee;
        ec.ADD("Not a CIF file",ee.getReturnValue());
        delete dtset;
        dtset = nullptr;
        if ( ee.getReturnValue() == ERRABT ) throw ec;
      }
    }
    if ( dtset == nullptr ) {
      dtset = new Poscar;
      try {
        dtset->readFromFile(filename);
      }
      catch( Exception& e ) {
        ec += e;
        ec.ADD("Not a POSCAR file",e.getReturnValue());
        delete dtset;
        throw ec;
      }
    }

    this->buildFromDtset(*dtset);
    delete dtset;
    if ( nimage > 1 ) {
      bool imageIsTime = true;
      switch ( imgmov ) {
        //string
        case 2:
          // NEB
        case 5:
          imageIsTime = true;
          std::clog << "Reading String/NEB last time step only" << std::endl;
          break;
          //PIMD  Langevin
        case 9:
          //PIMD  Nose Hoover
        case 13:
          _nimage = nimage;
          imageIsTime = false;
          std::clog << "Reading PIMD" << std::endl;
          break;
      }
      std::clog << "Building images";
      for ( unsigned img = 2 ; img <= nimage ; ++ img ) {
        Dtset dtsetimg;
        dtsetimg.readConfig(parser,img);
        HistDataDtset hist(dtsetimg);
        *this += hist;
        std::clog << " ..." << img;
      }
      for ( unsigned img = 1 ; img <= nimage ; ++ img ) {
        std::string suffix = " ";
       if ( img > 1 ) suffix = "_"+utils::to_string(img)+"img";
        double etot = 0.;
        try {
          etot = parser.getToken<double>("etotal"+suffix);
        }
        catch (...) {
          etot = 0.;
        }
        _etotal[img-1] = etot;
      }
      std::clog << std::endl;
      if ( !imageIsTime ) {
        _ntime = 1;
        _typat.resize(_natom*nimage);
        for ( unsigned img = 1 ; img < nimage ; ++img ) {
          std::copy(&_typat[0],&_typat[_natom],&_typat[_natom*img]);
        }
        _natom *= nimage;
        _acell.resize(3);
        _rprimd.resize(9);
        _etotal.resize(1);
        _time.resize(1);
        _stress.resize(6);
        _fcart.resize(0);
      }
    }

  }
  catch( Exception& e ){
    e.ADD("Failed to build HistData from Dtset",e.getReturnValue());
    throw e;
  }
  catch(...) {
    throw EXCEPTION("Something really bad happened",ERRABT);
  }
  _filename = filename;
  _ntimeAvail = _ntime;
}

void HistDataDtset::buildFromDtset(const Dtset& dtset) {
    _natom = dtset.natom();
    _xyz = 3;
    _ntime = 1;
    _ntimeAvail = _ntime;
    _filename = "in memory";
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

    _acell[0] = dtset.acell()[0];
    _acell[1] = dtset.acell()[1];
    _acell[2] = dtset.acell()[2];

    for ( int vgt = 0 ; vgt < 6 ; ++vgt )
      _stress[vgt] = 0.;

    _time[0] = 0.;

    {
      geometry::mat3d rprim = dtset.rprim();
      _rprimd [0] = rprim[0];
      _rprimd [1] = rprim[1];
      _rprimd [2] = rprim[2];
      _rprimd [3] = rprim[3];
      _rprimd [4] = rprim[4];
      _rprimd [5] = rprim[5];
      _rprimd [6] = rprim[6];
      _rprimd [7] = rprim[7];
      _rprimd [8] = rprim[8];
    }
    _etotal[0] = 0.;

    {
      auto xcart = dtset.xcart();
      auto xred = dtset.xred();

      for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
        //_fcart[iatom*3  ] = 0.;
        //_fcart[iatom*3+1] = 0.;
        //_fcart[iatom*3+2] = 0.;
        _xcart[iatom*3  ] = xcart[iatom][0];
        _xcart[iatom*3+1] = xcart[iatom][1];
        _xcart[iatom*3+2] = xcart[iatom][2];
        _xred[iatom*3  ] = xred[iatom][0];
        _xred[iatom*3+1] = xred[iatom][1];
        _xred[iatom*3+2] = xred[iatom][2];
      }
    }
}

//
void HistDataDtset::plot(unsigned tbegin, unsigned tend, std::istream &stream, Graph *gplot, Graph::GraphSave save) {
  std::string function;
  auto pos = stream.tellg();

  try {
    HistData::plot(tbegin, tend, stream, gplot, save);
  }
  catch ( Exception &e ) {
    if ( e.getReturnValue() == ERRABT ) {
      stream.clear();
      stream.seekg(pos);
      stream >> function;

      Graph::Config config;
      unsigned ntime = tend-tbegin;
      std::vector<double> &x = config.x;
      std::list<std::vector<double>> &y = config.y;
      std::string &filename = config.filename;
      std::string &xlabel = config.xlabel;
      std::string &ylabel = config.ylabel;
      std::string &title = config.title;
      config.doSumUp = false;

      xlabel = "Image";
      x.resize(ntime);
      for ( unsigned i = tbegin ; i < tend ; ++i ) x[i-tbegin]=i;

      // Etotal
      if ( function == "etotal" ) {
        filename = "etotal";
        ylabel = "Etot[Ha]";
        title = "Total energy";
        std::clog << std::endl << " -- Total (electronic) energy --" << std::endl;
        y.push_back(std::vector<double>(_etotal.begin()+tbegin,_etotal.end()-(_ntime-tend)));
      }

      else {
        throw EXCEPTION(std::string("Function ")+function+std::string(" not available yet"),ERRABT);
      }

      Graph::plot(config,gplot);

    }
    else {
      //e.ADD("Bad things happen sometimes",ERRDIV);
      throw e;
    }
  }
}

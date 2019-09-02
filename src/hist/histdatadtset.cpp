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
#include "io/configparser.hpp"
#include <fstream>

//
HistDataDtset::HistDataDtset() : HistData(){
  ;
}

HistDataDtset::HistDataDtset(const Dtset& dtset) {
  this->buildFromDtset(dtset);
  _nimage = 0;
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
    unsigned ndtset = 0;
    try {
      parser.parse();
      if ( parser.hasToken("ndtset") ) ndtset = parser.getToken<unsigned>("ndtset");
      else dtset->readConfig(parser);
      try {
        nimage = parser.getToken<unsigned>(" nimage");
      }
      catch ( Exception &e ) {
        nimage = 0;
        (void)e;
      }
      if ( nimage > 0 ) {
        try {
          imgmov = parser.getToken<unsigned>("imgmov");
        }
        catch ( Exception &e ) {
          imgmov = 13;
          (void)e;
        }
      }
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

    if ( ndtset > 0 ) {
      std::clog << "Reading " << ndtset << " dtset" << std::endl;
      std::vector<unsigned> jdtset(ndtset);
      if ( parser.hasToken("jdtset") ) jdtset = parser.getToken<unsigned>("jdtset",ndtset);
      else if ( parser.hasToken("udtset") ) {
        std::vector<unsigned>udtset = parser.getToken<unsigned>("udtset",2);
        unsigned int idtset = 0;
        for ( unsigned i1 = 1 ; i1 <= udtset[0] ; ++i1 ) {
          for ( unsigned i2 = 1 ; i2 <= udtset[1] ; ++i2 ) {
            jdtset[idtset++] = i1*10+i2;
          }
        }
      }
      else {
        for( unsigned i1 = 0 ; i1 < ndtset ; ++i1 ) jdtset[i1] = i1+1;
      }

      for ( unsigned ij = 0 ; ij < jdtset.size() ; ++ij ) {
        unsigned j = jdtset[ij];
        Dtset dtsetj;
        dtsetj.readConfig(parser,0,j);
        HistDataDtset hist(dtsetj);
        try {
          hist._fcart = parser.getToken<double>("fcart"+utils::to_string(j),3*dtsetj.natom());
        } catch (Exception &e)
        {;}
        try {
          hist._stress = parser.getToken<double>("strten"+utils::to_string(j),6);
        } catch (Exception &e)
        {;}
        try {
          hist._etotal = parser.getToken<double>("etotal"+utils::to_string(j),1);
        } catch (Exception &e)
        {;}
        if ( ij == 0 ) *this = hist;
        else *this += hist;
      }
      _filename = filename;
      return;
    }

    if ( dtset == nullptr )
      throw ec;

    this->buildFromDtset(*dtset);
    delete dtset;
    try {
      _fcart = parser.getToken<double>("fcart",_natom*3);
    } catch (Exception &e)
    {;}
    try {
      _stress = parser.getToken<double>("strten",6);
    } catch (Exception &e)
    {;}
    try {
      _etotal[0] = parser.getToken<double>("etotal");
    } catch (Exception &e)
    {;}
      
    if ( nimage > 0 ) {
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
          imageIsTime = false;
          _imgdata._imgmov = imgmov;
          std::clog << "Reading PIMD" << std::endl;
          break;
      }
      _fcart.clear();
      std::clog << "Building images";
      for ( unsigned img = 2 ; img <= nimage ; ++ img ) {
        Dtset dtsetimg;
        dtsetimg.readConfig(parser,img);
        HistDataDtset hist(dtsetimg);
        *this += hist;
        std::clog << " ..." << img;
      }
      _fcart.resize(3*_natom*nimage);
      _imgdata.resize(nimage);
      for ( unsigned img = 1 ; img <= nimage ; ++ img ) {
        std::string suffix = " ";
        if ( img > 1 ) suffix = "_"+utils::to_string(img)+"img";

        try {
          auto fcart = parser.getToken<double>("fcart"+suffix,_natom*3);
          std::copy_n(fcart.begin(),3*_natom,_fcart.begin()+3*_natom*(img-1));
        } 
        catch (Exception &e) {
          std::fill_n(_fcart.begin()+3*_natom*(img-1),3*_natom,0.);
        }

        double etot = 0.;
        try {
          etot = parser.getToken<double>("etotal"+suffix);
        }
        catch (...) {
          etot = 0.;
        }

        std::vector<double> stress(6);
        try {
          stress = parser.getToken<double>("strten"+suffix,6);
        } 
        catch (Exception &e) {
          std::fill_n(stress.begin(),6,0.);
        }

        std::vector<double> acell(3);
        try {
          acell= parser.getToken<double>("acell"+suffix,3);
        } 
        catch (Exception &e) {
          std::copy_n(_acell.begin(),3,acell.begin());
        }

        std::vector<double> rprimd(9);
        try {
          rprimd= parser.getToken<double>("rprimd"+suffix,9);
        } 
        catch (Exception &e) {
          std::copy_n(_rprimd.begin(),9,rprimd.begin());
        }

        std::copy_n(acell.begin() ,3,_imgdata._acell.begin()+3*(img-1));
        std::copy_n(rprimd.begin(),9,_imgdata._rprimd.begin()+9*(img-1));
        std::copy_n(stress.begin(),6,_imgdata._stress.begin()+6*(img-1));
        _imgdata._etotal[img-1] = etot;

      }
      std::clog << std::endl;
      if ( !imageIsTime ) {
        _ntime = 1;
        _typat.resize(_natom*nimage);
        for ( unsigned img = 1 ; img < nimage ; ++img ) {
          std::copy(&_typat[0],&_typat[_natom],&_typat[_natom*img]);
        }
        _natom *= nimage;
        std::vector<double> mdtemp(2);
        _stress.resize(6);
        _etotal.resize(1);
        try {
          mdtemp = parser.getToken<double>("mdtemp",2);
        }
        catch (Exception &e) {
          _etotal[0] = _imgdata._etotal[0];
          for ( unsigned v = 0 ; v < 6 ; ++v ) 
            _stress[v] = _imgdata._stress[v];
        }
        _nimage = nimage;
      }
      else {
        _acell = _imgdata._acell;
        _rprimd =_imgdata._rprimd;
        _stress = _imgdata._stress;
        _etotal = _imgdata._etotal;
        _imgdata.clear();
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

void HistDataDtset::dump(HistData &hist, const std::string& filename, unsigned tbegin, unsigned tend, unsigned step) {
  unsigned ntime = hist.ntime(); //tend-tbegin;
  unsigned ndecimal = 1;
  hist.checkTimes(tbegin,tend);
  while ((ntime/=10)>0) ++ndecimal;
  try {
    for ( unsigned iitime = tbegin ; iitime < tend ; iitime += step ) {
      std::ostringstream output;
      output << filename << std::setw(ndecimal) << std::setfill('0') << iitime << ".in";
      std::clog << output.str() << std::endl;
      Dtset dtset(hist,iitime);
      dtset.dump(output.str().c_str());
    }
  }
  catch ( Exception &e ) {
    e.ADD("Dumping failed",ERRABT);
    throw e;
  }
}


/**
 * @file src/io/abihdr.cpp
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


#include "io/abihdr.hpp"
#include "base/exception.hpp"
#include "base/mendeleev.hpp"
#include <fstream>
#include <algorithm>
#include <numeric>

//
AbiHdr::AbiHdr() : Dtset(),
    _endHeader(0),
    // <H1>
    _codvsn(),
    _hdrform(0),
    _fform(0),
    // <H2>
    _bandtot(0),
    _date(0),
    _intxc(0),
    _ixc(0),
    //int _natom;
    _ngfft(),
    _nkpt(0),
    _nspden(0),
    _nspinor(0),
    _nsppol(0),
    _nsym(0),
    _npsp(0),
    //int _ntypat;
    _occopt(0),
    _pertcase(0),
    _usepaw(0),
    _ecut(0),
    _ecutdg(0),
    _ecutsm(0),
    _ecut_eff(0),
    _qptn(),
    //double _rprimd[9];
    _stmbias(0),
    _tphysel(0),
    _tsmear(0),
    _usewvl(0),
    _nshiftk_orig(0),
    _nshiftk(0),
    _mband(0),
    // <H3>
    _istwfk(),
    _nband(),
    _npwarr(),
    _so_psp(),
    _symafm(),
    _symrel(),
    //std::vector<> typat;
    _kptns(),
    _occ3d(),
    _tnons(),
    //std::vector<> znucltypat;
    _wtk(),
    // <H4>
    _residm(0),
    //std::vector<double> _xred;
    _etot(0),
    _fermie(0),
    _amu(),
    // <H5>
    _kptopt(0),
    _pawcpxocc(0),
    _nelect(0),
    _charge(0),
    _icoulomb(0),
    _kptrlatt(),
    _kptrlatt_orig(),
    _shiftk_orig(),
    _shiftk(),
    // <H6>
    _title(),
    _znuclpsp(),
    _zionpsp(),
    _pspso(),
    _pspdat(),
    _pspcod(),
    _pspxc(),
    _lmn_size(),
    _md5_pseudos()
{
  ;
}

//
AbiHdr::~AbiHdr() {
  ;
}

void AbiHdr::readFromFile(const std::string& filename) {
  std::ifstream file;
  
  file.open(filename,std::ios::in|std::ios::binary);

  if ( !file )
    throw EXCEPTION(std::string("File ")+filename+" could not be opened",ERRABT);

  unsigned int marker;                  // Size of the next variable.

  std::vector<int> isize;
  int ifullsize;
  std::vector<int> dsize;
  int dfullsize;
  std::vector<int> idummy;
  std::vector<double> ddummy;

  auto checkMarker = [&](int level, int charsize=0){
    file.read((char*)(&marker),sizeof(int));
    if ( level > 0 ) {
      ifullsize = std::accumulate(isize.begin(),isize.end(),0);
      dfullsize = std::accumulate(dsize.begin(),dsize.end(),0);
    }
    /*
    std::clog << marker << std::endl;
    std::clog << charsize*sizeof(char)<< std::endl;
    std::clog << ifullsize*sizeof(int) << std::endl;
    std::clog << dfullsize*sizeof(double) << std::endl;
    */
    if ( marker != (charsize*sizeof(char)+ifullsize*sizeof(int)+ (dfullsize)*sizeof(double)) )
      throw EXCEPTION(std::string("Bad header file: <")+(level<0?"/H":"H")+utils::to_string(std::abs(level))+">",ERRDIV);
    if ( level > 0 ) {
      idummy.resize(*std::max_element(isize.begin(),isize.end()));
      ddummy.resize(*std::max_element(dsize.begin(),dsize.end()));
    }
  };

 /**
  * FIRST RECORD
  * read(unit, err=10, iomsg=errmsg) hdr%codvsn,hdr%headform,fform
  */
  file.read((char*)(&marker),sizeof(int));
  unsigned int nchar = marker - 2*sizeof(int);
  //if ( marker != (6*sizeof(char)+2*sizeof(int)) )
  //  throw EXCEPTION("Bad header file: <H1>",ERRDIV);

  _codvsn.resize(nchar+1);
  file.read(&_codvsn[0],nchar);
  _codvsn[nchar]='\0';
  file.read((char*)&_hdrform,sizeof(int));
  file.read((char*)&_fform,sizeof(int));

  nchar = marker;
  file.read((char*)(&marker),sizeof(int));
  if ( marker != nchar )
    throw EXCEPTION("Bad header file: </H1>",ERRDIV);

  // Check codvsn
  /*
  std::clog << "Abinit unformatted file version " << _codvsn << std::endl;
  std::clog << "Header format " << _hdrform << std::endl;
  */

 /**
  * SECOND RECORD
  * read(unit, err=10, iomsg=errmsg) &
  * &  hdr%bantot, hdr%date, hdr%intxc, hdr%ixc, hdr%natom, hdr%ngfft(1:3),&
  * &  hdr%nkpt, hdr%nspden, hdr%nspinor, hdr%nsppol, hdr%nsym, hdr%npsp, hdr%ntypat, hdr%occopt, hdr%pertcase,&
  * &  hdr%usepaw, hdr%ecut, hdr%ecutdg, hdr%ecutsm, hdr%ecut_eff, hdr%qptn(1:3), hdr%rprimd,&
  * &  hdr%stmbias, hdr%tphysel, hdr%tsmear, hdr%usewvl, hdr%nshiftk_orig, hdr%nshiftk, hdr%mband
  */
  isize = {18,4};  //Not really consitent but too many variables !
  dsize = {19};
  checkMarker(2);

  file.read((char*)(&idummy[0]),18*sizeof(int));
  _bandtot   = idummy[0];
  _date      = idummy[1];
  _intxc     = idummy[2];
  _ixc       = idummy[3];
  _natom     = idummy[4];
  _ngfft[0]  = idummy[5];
  _ngfft[1]  = idummy[6];
  _ngfft[2]  = idummy[7];
  _nkpt      = idummy[8];
  _nspden    = idummy[9];
  _nspinor   = idummy[10];
  _nsppol    = idummy[11];
  _nsym      = idummy[12];
  _npsp      = idummy[13];
  _ntypat    = idummy[14];
  _occopt    = idummy[15];
  _pertcase  = idummy[16];
  _usepaw    = idummy[17];

  file.read((char*)(&ddummy[0]),19*sizeof(double));

  _ecut     = ddummy[0];
  _ecutdg   = ddummy[1];
  _ecutsm   = ddummy[2];
  _ecut_eff = ddummy[3];
  _qptn[0]  = ddummy[4];
  _qptn[1]  = ddummy[5];
  _qptn[2]  = ddummy[6];
  _rprim[0] = ddummy[7];
  _rprim[3] = ddummy[8];
  _rprim[6] = ddummy[9];
  _rprim[1] = ddummy[10];
  _rprim[4] = ddummy[11];
  _rprim[7] = ddummy[12];
  _rprim[2] = ddummy[13];
  _rprim[5] = ddummy[14];
  _rprim[8] = ddummy[15];
  _stmbias  = ddummy[16];
  _tphysel  = ddummy[17];
  _tsmear   = ddummy[18];
  _acell[0] = 1;
  _acell[1] = 1;
  _acell[2] = 1;
  _gprim = geometry::invertTranspose(_rprim);

  file.read((char*)(&idummy[0]),4*sizeof(int));
  _usewvl      = idummy[0];
  _nshiftk_orig = idummy[1];
  _nshiftk      = idummy[2];
  _mband        = idummy[3];

  /*
  std::clog << _natom << " atoms" << std::endl;
  std::clog << _ntypat << " types of atoms" << std::endl;
  std::clog << _nsppol << " spin polarization" << std::endl;
  std::clog << "ngfft is " << _ngfft[0] << " " << _ngfft[1] << " " << _ngfft[2] << std::endl;
  std::clog << "usepaw " << _usepaw << std::endl;
  geometry::print(_rprim);
  */

  checkMarker(-2);

  _znucl.resize(_ntypat);
  _typat.resize(_natom);
  _xcart.resize(3*_natom);
  _xred.resize(3*_natom);


  /**
   * THIRD RECORD
   * read(unit, err=10, iomsg=errmsg) &
   * & hdr%istwfk(:), hdr%nband(:), hdr%npwarr(:), &
   * & hdr%so_psp(:), hdr%symafm(:), hdr%symrel(:,:,:), &
   * & hdr%typat(:), hdr%kptns(:,:), occ3d, &
   * & hdr%tnons(:,:), hdr%znucltypat(:), hdr%wtk(:)
   */

  isize = {_nkpt,_nkpt*_nsppol,_nkpt,_npsp,_nsym,3*3*_nsym,(int)_natom}; 
  dsize = {3*_nkpt,_mband*_nkpt*_nsppol,3*_nsym,(int)_ntypat,_nkpt};
  std::vector<std::vector<int>*> iptr;
  std::vector<std::vector<double>*> dptr;
  checkMarker(3);

  iptr.clear();
  iptr.push_back(&_istwfk);
  iptr.push_back(&_nband);
  iptr.push_back(&_npwarr);
  iptr.push_back(&_so_psp);
  iptr.push_back(&_symafm);
  iptr.push_back(&_symrel);
  for ( int v = 0 ; v < 6 ; ++v ) {
    iptr[v]->resize(isize[v]);
    file.read((char*)(&(iptr[v]->at(0))),isize[v]*sizeof(int));
  }

  // Read _typat
  file.read((char*)(&_typat[0]),isize[6]*sizeof(int));

  dptr.clear();
  dptr.push_back(&_kptns);
  dptr.push_back(&_occ3d);
  dptr.push_back(&_tnons);
  for ( int v = 0 ; v < 3 ; ++v ) {
    dptr[v]->resize(dsize[v]);
    file.read((char*)(&(dptr[v]->at(0))),dsize[v]*sizeof(double));
  }

  file.read((char*)(&ddummy[0]),dsize[3]*sizeof(double));
  for ( unsigned t = 0 ; t < _ntypat ; ++t )
    _znucl[t] = (unsigned) ddummy[t];

  _wtk.resize(dsize[4]);
  file.read((char*)(&_wtk[0]),dsize[4]*sizeof(double));

  checkMarker(-3);

  /**
   * FORTH RECORD
   * read(unit, err=10, iomsg=errmsg) hdr%residm, hdr%xred(:,:), hdr%etot, hdr%fermie, hdr%amu(:)
   */
  isize = {};
  dsize = {1,3*(int)_natom,1,1,(int)_ntypat};

  checkMarker(4);

  file.read((char*)(&_residm),dsize[0]*sizeof(double));
  file.read((char*)(&ddummy[0]),dsize[1]*sizeof(double));
  for ( unsigned a = 0 ; a < _natom ; ++a )
    for( unsigned d = 0 ; d < 3 ; ++d )
      _xred[a][d]=ddummy[a*3+d];
  geometry::changeBasis(_rprim, _xcart, _xred, false);

  file.read((char*)(&_etot),dsize[2]*sizeof(double));
  file.read((char*)(&_fermie),dsize[3]*sizeof(double));
  _amu.resize(dsize[4]);
  file.read((char*)(&_amu[0]),dsize[4]*sizeof(double));
  for ( unsigned z = 0 ; z < _znucl.size() ; ++z )
    MendeTable.mass[_znucl[z]] = _amu[z];

  checkMarker(-4);

  /**
   * FIFTH RECORD
   * read(unit, err=10, iomsg=errmsg)&
   * hdr%kptopt,hdr%pawcpxocc,hdr%nelect,hdr%charge,hdr%icoulomb,&
   * hdr%kptrlatt,hdr%kptrlatt_orig, hdr%shiftk_orig,hdr%shiftk
   */
  isize = {1,1,1,9,9};
  dsize = {1,1,3*_nshiftk_orig,3*_nshiftk};

  checkMarker(5);
  file.read((char*)(&_kptopt)      ,isize[0]*sizeof(int));
  file.read((char*)(&_pawcpxocc)   ,isize[1]*sizeof(int));
  file.read((char*)(&_nelect)      ,isize[2]*sizeof(int));
  file.read((char*)(&_charge)      ,dsize[0]*sizeof(double));
  file.read((char*)(&_icoulomb)    ,dsize[1]*sizeof(double));
  file.read((char*)(_kptrlatt)     ,isize[3]*sizeof(int));
  file.read((char*)(_kptrlatt_orig),isize[4]*sizeof(int));
  _shiftk_orig.resize(dsize[2]);
  _shiftk.resize(dsize[3]);
  file.read((char*)(&_shiftk_orig[0]),dsize[2]*sizeof(double));
  file.read((char*)(&_shiftk[0])     ,dsize[3]*sizeof(double));

  checkMarker(-5);

  /**
   * SIXTH RECORD
   * ! Reading the records with psp information ---------------------------------
   * do ipsp=1,hdr%npsp
   * read(unit, err=10, iomsg=errmsg) &
   * &   hdr%title(ipsp), hdr%znuclpsp(ipsp), hdr%zionpsp(ipsp), hdr%pspso(ipsp), hdr%pspdat(ipsp), &
   * &   hdr%pspcod(ipsp), hdr%pspxc(ipsp), hdr%lmn_size(ipsp), hdr%md5_pseudos(ipsp)
   * end do
   */
  isize = {1,1,1,1,1};
  dsize = {1,1};
  _title.resize(_npsp);
  _znuclpsp.resize(_npsp);
  _zionpsp.resize(_npsp);
  _pspso.resize(_npsp);
  _pspdat.resize(_npsp);
  _pspcod.resize(_npsp);
  _pspxc.resize(_npsp);
  _lmn_size.resize(_npsp);
  _md5_pseudos.resize(_npsp);
  for ( int ipsp = 0 ; ipsp < _npsp ; ++ipsp ) {
    checkMarker(6*10+ipsp,132+32);
    char tmp[133];
    file.read(tmp,132*sizeof(char));
    tmp[132] = '\0';
    file.read((char*)(&_znuclpsp[ipsp]),isize[0]*sizeof(int));
    file.read((char*)(&_zionpsp[ipsp]) ,isize[1]*sizeof(int));
    file.read((char*)(&_pspso[ipsp])   ,isize[2]*sizeof(int));
    file.read((char*)(&_pspdat[ipsp])  ,isize[3]*sizeof(int));
    file.read((char*)(&_pspcod[ipsp])  ,isize[4]*sizeof(int));
    file.read((char*)(&_pspxc[ipsp])   ,dsize[0]*sizeof(double));
    file.read((char*)(&_lmn_size[ipsp]),dsize[1]*sizeof(double));
    file.read(tmp,32*sizeof(char));
    tmp[32] = '\0';
    _md5_pseudos[ipsp] = tmp;
    checkMarker(-(6*10+ipsp),132+32);
  }


  /**
   * Ignore all records before te one we want.
   * if (hdr%usepaw==1) then ! Reading the Rhoij tab if the PAW method was used.
   * call pawrhoij_io(hdr%pawrhoij,unit,hdr%nsppol,hdr%nspinor,hdr%nspden,hdr%lmn_size,hdr%typat,hdr%headform,"Read")
   * end if
   */
  if ( _usepaw ) {
    if ( _hdrform >= 44 && _hdrform < 56 ) {
      isize = {_nspden*(int)_natom};
      dsize = {};
      checkMarker(10);
      file.read((char*)(&idummy[0]),isize[0]*sizeof(int));
      checkMarker(-10);
      int bsize = std::accumulate(idummy.begin(),idummy.begin()+isize[0],0);
      checkMarker(11);
      isize = {bsize};
      dsize = {bsize};
      //file.seekg(file.tellg()+(long int)marker);
      file.read((char*)(&idummy[0]),isize[0]*sizeof(int));
      file.read((char*)(&ddummy[0]),dsize[0]*sizeof(double));
      checkMarker(-11);
    }
    else if ( _hdrform >= 56 ) {
      int my_cplx = 1;
      int my_nspden = 1;
      int my_qphase = 1;
      int bsize = 0;
      if ( _hdrform == 56 ) {
        isize = {(int)_natom,1};
        dsize = {};
        checkMarker(20);
        file.read((char*)(&idummy[0]),isize[0]*sizeof(int));
        file.read((char*)(&my_cplx),isize[1]*sizeof(int));
        checkMarker(-20);
      }
      else { 
        file.read((char*)&marker,sizeof(int));
        idummy.resize(_natom);
        unsigned int iread = 0;
        if ( marker >= _natom*sizeof(int) ) {
          file.read((char*)&idummy[0],_natom*sizeof(int));
          iread += _natom*sizeof(int);
        }
        if ( marker >= (_natom+1)*sizeof(int) ) {
          file.read((char*)&my_cplx,sizeof(int));
          iread += sizeof(int);
        }
        if ( marker >= (_natom+2)*sizeof(int) ) {
          file.read((char*)&my_nspden,sizeof(int));
          iread += sizeof(int);
        }
        if ( marker >= (_natom+3)*sizeof(int) ) {
          file.read((char*)&my_qphase,sizeof(int));
          iread += sizeof(int);
        }
        if ( marker != iread ) throw EXCEPTION("BUG PAW 1!!!!",ERRABT);
        file.read((char*)&marker,sizeof(int));
        if ( marker != iread ) throw EXCEPTION("BUG PAW 2!!!!",ERRABT);
      }
      bsize = std::accumulate(idummy.begin(),idummy.begin()+_natom,0);
      isize = { bsize };
      dsize = { bsize*my_nspden*my_cplx*my_qphase };
      checkMarker(21);
      //file.seekg(file.tellg()+(long int)marker);
      file.read((char*)(&idummy[0]),isize[0]*sizeof(int));
      file.read((char*)(&ddummy[0]),dsize[0]*sizeof(double));
      checkMarker(-21);
    }
  }
  _endHeader = file.tellg();
}

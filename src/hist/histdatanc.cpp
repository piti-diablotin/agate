/**
 * @file src/histdatanc.cpp
 *
 * @brief Read a _HIST file in netcdf
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


#include "hist/histdatanc.hpp"
#include "base/exception.hpp"
#include "base/utils.hpp"
#include "base/phys.hpp"
#include "base/mendeleev.hpp"
#include "base/geometry.hpp"
#include <algorithm>
#include <iomanip>
#include <fstream>
#ifdef HAVE_NETCDFCXX4
#include <memory>
#endif

#ifdef HAVE_NETCDF
#  ifdef HAVE_NETCDFCXX4
#    include <netcdf>
#  elif defined(HAVE_NETCDFCPP)
#    include <netcdfcpp.h>
#  else
#    include <netcdf.h>
#  endif
#endif

//
HistDataNC::HistDataNC() : HistDataMD()
{
}

HistDataNC::HistDataNC(const HistData& hist) : HistDataMD(hist)
{
}

//
HistDataNC::HistDataNC(HistData&& hist) : HistDataMD(hist)
{
}

//
HistDataNC::~HistDataNC() {
}

//
void HistDataNC::readFromFile(const std::string& filename) {
#ifndef HAVE_NETCDF
  throw EXCEPTION("NetCDF support is not available.\nConsider compiling the code with NetCDF support to read "+filename,ERRDIV);
#else

  double dtion = 100.0;
  bool has_znucl, has_typat, has_spinat, has_dtion = false;
  bool has_mdtemp = false;
  int ncid = 0;
  int varid = 0;
  int dimid = 0;
  size_t nimage = 0;
  int imgmov = 0;
  size_t fixedTime;
  bool imageIsTime = false;
  size_t natomImg;
  double mdtemp = 0;

  if ( nc_open(filename.c_str(), NC_NOWRITE, &ncid)) 
    throw EXCEPTION(std::string("File ")+filename+" could not be correctly opened",ERRDIV);

  if (  nc_inq_ndims(ncid, &dimid) || dimid < 4 || dimid > 8 ) {
    nc_close(ncid);
    throw EXCEPTION(std::string("Bad number of dimension: ")+utils::to_string(dimid)+
        std::string(" instead of being between 4 and 8"),ERRDIV);
  }

  if ( nc_inq_nvars(ncid, &varid) || varid < 11 ) {
    nc_close(ncid);
    throw EXCEPTION(std::string("Bad number of variables: ")+utils::to_string(varid)+
        std::string(" instead of at least 11"),ERRABT);
  }

  // Read dimensions.
  size_t dimval;
  nc_inq_dimid(ncid, "natom", &dimid);
  nc_inq_dimlen(ncid, dimid, &dimval);
  _natom = (int) dimval;
  nc_inq_dimid(ncid, "xyz", &dimid);
  nc_inq_dimlen(ncid, dimid, &dimval);
  _xyz = (int) dimval;
  if ( !nc_inq_dimid(ncid, "nimage", &dimid) ){
    nc_inq_dimlen(ncid, dimid, &dimval);
    nimage = (int) dimval;
    // Small check for consistency.
    int ndim_check = 0;
    nc_inq_varid(ncid, "etotal", &varid);
    if ( !nc_inq_varndims(ncid, varid,  &ndim_check ) ) {
      if ( ndim_check < 1 || ndim_check > 2 )
        throw EXCEPTION("Bad dimension for etotal with image",ERRABT);
      else if ( ndim_check == 1 ) {
        nimage = 0;
        imgmov = 0;
      }
    }
  }
  else {
    // Small check for consistency.
    int ndim_check = 0;
    nc_inq_varid(ncid, "etotal", &varid);
    if ( !nc_inq_varndims(ncid, varid,  &ndim_check ) ) {
      if ( ndim_check != 1 )
        throw EXCEPTION("Bad dimension for etotal with no image",ERRABT);
    }
  }

  if ( nc_inq_unlimdim(ncid, &dimid) ) {
    _ntime = 0;
    nc_inq_varid(ncid, "etotal", &varid);
    float etotal;
    size_t ntime=0;
    while (nc_get_var1_float(ncid, varid,  &ntime, &etotal)) {++ntime;}
    _ntime=--ntime;
  }
  else {
    //nc_inq_varid(ncid, "etotal", &varid);
    //nc_inq_vardimid(ncid, varid, &dimid);
    nc_inq_dimlen(ncid, dimid, &dimval);
    _ntime = (int) dimval;
  }

  if ( _xyz != 3 ) {
    nc_close(ncid);
    throw EXCEPTION("Bad value for xyz: "+utils::to_string(_xyz),ERRDIV);
  }

  //IMGMOV
  try {
    if ( nc_inq_varid(ncid, "imgmov", &varid) )
      throw Exception();
    size_t start[] = {0};
    if ( nc_get_var1_int(ncid, varid, start, &imgmov ) ) {
      imgmov = 0;
    }
  }
  catch (...) {
    imgmov = 0;
  }

  _filename = filename;

  fixedTime = _ntime-1;
  natomImg = _natom;
  if ( nimage > 0 ) {
    if ( imgmov == 0 ) {
      imgmov = 9;
      auto e = EXCEPTION("\"imgmov\" is not provided in the file. Assuming it is a PIMD run",ERRWAR);
      std::clog << e.fullWhat() << std::endl;
    }
    switch ( imgmov ) {
      //string
      case 2:
        // NEB
      case 5:
        _ntime = nimage;
        imageIsTime = true;
        dtion = 1;
        std::clog << "Reading String/NEB last time step only" << std::endl;
        break;
        //PIMD  Langevin
      case 9:
        //PIMD  Nose Hoover
      case 13:
        _natom *= nimage;
        _nimage = nimage;
        imageIsTime = false;
        std::clog << "Reading PIMD" << std::endl;
        break;
    }
  }

  // Allocate arrays
  _xcart  .resize(_ntime*_natom*_xyz);
  _xred   .resize(_ntime*_natom*_xyz);
  _fcart  .resize(_ntime*_natom*_xyz);
  _acell  .resize(_ntime*_xyz);
  _rprimd .resize(_ntime*_xyz*_xyz);
  _time   .resize(_ntime);
  _stress .resize(_ntime*6);
  _ekin   .resize(_ntime);
  _etotal .resize(_ntime);
  _velocities.resize(_ntime*_natom*_xyz);
  _entropy .resize(_ntime);
  _temperature.resize(_ntime);
  _pressure.resize(_ntime);

  auto get_var = [this](int ncid, size_t start[], size_t count[], double *data,const char name[]) {
    int varid;
    int status = nc_inq_varid(ncid, name, &varid);
    if ( status ) {
      nc_close(ncid);
      std::ostringstream msg;
      msg << "Error while inquiring " << name << " var in " << _filename << " with error " << status;
      throw EXCEPTION(msg.str(),ERRDIV);
    }
    status = nc_get_vara_double(ncid, varid, start, count, data);
    if ( status ) {
      nc_close(ncid);
      std::ostringstream msg;
      msg << "Error while reading " << name << " var in " << _filename << " with error " << status;
      throw EXCEPTION(msg.str(),ERRDIV);
    }
  };


  /**
   * Some variable that are not always in the HIST file 
   */
  // ZNUCL
  try {
    size_t ntypat;
    int err = 0;
    err += nc_inq_varid(ncid, "znucl", &varid);
    err += nc_inq_vardimid(ncid, varid, &dimid);
    err += nc_inq_dimlen(ncid, dimid, &ntypat);
    if ( err ) throw Exception();
    _znucl.resize(ntypat);
    size_t count[] = {ntypat};
    size_t start[] = {0};
    if ( nc_get_vara_int(ncid, varid, start, count, _znucl.data()) ) {
      throw EXCEPTION(std::string("Error while reading znucl var in ")+filename,ERRDIV);
    }
    has_znucl = true;
  }
  catch (...) {
    has_znucl = false;
  }
  // TYPAT
  try {
    _typat.resize(natomImg);
    nc_inq_varid(ncid, "typat", &varid);
    size_t count[] = {natomImg};
    size_t start[] = {0};
    if ( nc_get_vara_int(ncid, varid, start, count, _typat.data()) ) {
      throw EXCEPTION(std::string("Error while reading typat var in ")+filename,ERRDIV);
    }
    has_typat = true;
  }
  catch (...) {
    has_typat = false;
  }
  // SPINAT
  try {
    if ( nc_inq_varid(ncid, "spinat", &varid) == NC_NOERR ) {//no error means present
      _spinat.resize(_ntime*natomImg*_xyz);
      size_t count[] = {natomImg*_xyz};
      size_t start[] = {0};
      nc_inq_varid(ncid, "spinat", &varid);
      if ( nc_get_vara_double(ncid, varid, start, count, _spinat.data()) ) {
        throw EXCEPTION(std::string("Error while reading spinat var in ")+filename,ERRDIV);
      }
      has_spinat = true;
    }
    else throw Exception();
  }
  catch (...) {
    has_spinat = false;
  }
  // MDTEMP
  if ( nimage > 0 ) {
    try {
      if ( nc_inq_varid(ncid, "mdtemp", &varid) == NC_NOERR ) {//no error means present
        size_t count[] = {1};
        size_t start[] = {1};
        nc_inq_varid(ncid, "mdtemp", &varid);
        if ( nc_get_vara_double(ncid, varid, start, count, &mdtemp) ) {
          throw EXCEPTION(std::string("Error while reading mdtemp var in ")+filename,ERRDIV);
        }
        has_mdtemp = true;
      }
      else throw Exception();
    }
    catch (...) {
      has_mdtemp = false;
    }
  }
  else mdtemp = true; // Assume we have mdtemp if we don't need it
  //DTION
  try {
    if ( nc_inq_varid(ncid, "dtion", &varid) == NC_NOERR ) {
      size_t start[] = {0};
      if ( nc_get_var1_double(ncid, varid, start, &dtion ) ) {
        throw EXCEPTION(std::string("Error while reading dtion var in ")+filename,ERRDIV);
      }
      has_dtion = true;
    }
  }
  catch (...) {
    has_dtion = false;
  }
  nc_close(ncid);

  /**
   * Now try to open the _OUT.nc file to read some additionnal data
   */
  if ( !has_znucl || !has_typat || !has_spinat || !has_dtion || !has_mdtemp ) {
    std::string outfilename = filename.substr(0,filename.find("_HIST"))+"_OUT.nc";

    try{
      if ( nc_open(outfilename.c_str(), NC_NOWRITE, &ncid)) {
        throw EXCEPTION(outfilename+" file can not be found.",ERRWAR);
      }

      nc_inq_varid(ncid, "natom", &varid);
      int natom_check;
      size_t start[] = {0};
      if ( nc_get_var1_int(ncid, varid, start, &natom_check ) ) {
        nc_close(ncid);
        throw EXCEPTION(std::string("Error while reading natom var in ")+outfilename,ERRDIV);
      }
      if ( natom_check != (int)natomImg ) {
        nc_close(ncid);
        throw EXCEPTION("natom is different in _HIST file and _OUT.nc file",ERRDIV);
      }

      if ( !has_dtion) {
        if ( nc_inq_varid(ncid, "dtion", &varid) == NC_NOERR ) {
          if ( nc_get_var1_double(ncid, varid, start, &dtion ) )
            dtion = 100;
        }
        else dtion = 100;
      }

      if ( !has_spinat) {
        if ( nc_inq_varid(ncid, "spinat", &varid) == NC_NOERR ) { //no error means present
          _spinat.resize(_ntime*natomImg*_xyz);
          size_t count[] = {natomImg*_xyz};
          nc_inq_varid(ncid, "spinat", &varid);
          if ( nc_get_vara_double(ncid, varid, start, count, _spinat.data()) ) {
            nc_close(ncid);
            throw EXCEPTION(std::string("Error while reading spinat var in ")+outfilename,ERRDIV);
          }
          has_spinat = true;
        }
      }

      if ( !has_znucl || !has_typat) {
        nc_inq_varid(ncid, "znucl", &varid);
        size_t ntypat;
        nc_inq_vardimid(ncid, varid, &dimid);
        nc_inq_dimlen(ncid, dimid, &ntypat);

        _znucl.resize(ntypat);
        size_t count[] = {ntypat};
        if ( nc_get_vara_int(ncid, varid, start, count, _znucl.data()) ) {
          nc_close(ncid);
          throw EXCEPTION(std::string("Error while reading znucl var in ")+outfilename,ERRDIV);
        }
        has_znucl = true;

        _typat.resize(natomImg);
        nc_inq_varid(ncid, "typat", &varid);
        count[0] = {natomImg};
        if ( nc_get_vara_int(ncid, varid, start, count, _typat.data()) ) {
          nc_close(ncid);
          throw EXCEPTION(std::string("Error while reading typat var in ")+outfilename,ERRDIV);
        }
        has_typat = true;
      }

      if ( !has_mdtemp ) {
        if ( nc_inq_varid(ncid, "mdtemp", &varid) == NC_NOERR ) {//no error means present
          size_t count[] = {1};
          size_t start[] = {1};
          nc_inq_varid(ncid, "mdtemp", &varid);
          if ( nc_get_vara_double(ncid, varid, start, count, &mdtemp) ) {
            throw EXCEPTION(std::string("Error while reading mdtemp var in ")+outfilename,ERRDIV);
          }
          has_mdtemp = true;
        }
      }

      nc_close(ncid);
    }
    catch( Exception& e ) {
      if ( !has_znucl || !has_typat) {
        _znucl.clear();
        _znucl.insert(_znucl.begin(),1,0);
        _typat.clear();
        _typat.insert(_typat.begin(),_natom,1);
      }
      e.ADD("Ignoring file "+outfilename+".\nSome data may be missing",ERRWAR);
      std::clog << e.fullWhat() << std::endl;;
    }
    if ( !has_spinat ) _spinat.clear();
  }

  if ( nimage > 0 && !imageIsTime ) {
    _typat.resize(_natom);
    _spinat.resize(_natom*_xyz*_ntime);
    for ( unsigned img = 1 ; img < nimage ; ++img ) {
      std::copy(&_typat[0],&_typat[natomImg],&_typat[natomImg*img]);
      std::copy(&_spinat[0],&_spinat[natomImg*3],&_spinat[natomImg*3*img]);
    }
  }

  _ntimeAvail = 0;
  if ( imageIsTime ) dtion = 1;
#ifdef HAVE_CPPTHREAD
  _endThread = false;
  _thread = std::thread([this,dtion,get_var,imageIsTime,nimage,fixedTime,natomImg,mdtemp](){
#endif

      const double ekinImages = 3. * 0.5  * natomImg * phys::kB * mdtemp / phys::Ha; // In Ha
      const double omegaP2 = nimage * phys::kB*phys::kB * mdtemp*mdtemp / (phys::Ha*phys::Ha);/*/ ( phys::hbar*phys::hbar );*/ // In /s2
      std::vector<double> mass(natomImg);
      for ( unsigned i = 0 ; i < natomImg ; ++i ) {
        mass[i] = mendeleev::mass[_znucl[_typat[i]-1]]*phys::amu_emass;
      }

      /**
       * READ TIME DATA
       */
      int local_ncid;
      try {
        int local_ncid;
        bool has_spinat = !_spinat.empty();
        bool has_entropy = true;
        if ( nc_open(_filename.c_str(), NC_NOWRITE, &local_ncid)) 
          throw EXCEPTION(std::string("File ")+_filename+" could not be correctly opened",ERRDIV);
        const auto chunksize = std::max((size_t)1,10*100000/(sizeof(double)*_natom*_xyz));
        size_t nloop = _ntime/chunksize+1;
        for ( size_t itime = 0,  iloop = 0 ; iloop < nloop ; ++iloop, itime+=chunksize ) {
#ifdef     HAVE_CPPTHREAD
          if ( _endThread == true ) break;
#endif    
          if ( itime >= _ntime ) break;
          size_t adjust_count = ( itime+chunksize > _ntime ? _ntime-itime : chunksize );

          if ( nimage == 0 ) {
            {
              size_t start[] = {itime,0,0};
              size_t count[] = {adjust_count,_natom,_xyz};

              get_var(local_ncid,start,count,&_xcart     [itime*_natom*_xyz],"xcart");
              get_var(local_ncid,start,count,&_xred      [itime*_natom*_xyz],"xred");
              get_var(local_ncid,start,count,&_fcart     [itime*_natom*_xyz],"fcart");
              get_var(local_ncid,start,count,&_velocities[itime*_natom*_xyz],"vel");
              count[1] = _xyz;
              get_var(local_ncid,start,count,&_rprimd    [itime*_xyz*_xyz],"rprimd");
            }
            {
              size_t start[] = {itime,0};
              size_t count[] = {adjust_count,_xyz};
              get_var(local_ncid,start,count,&_acell  [itime*_xyz],"acell");
              count[1] = 6;
              get_var(local_ncid,start,count,&_stress [itime*6]   ,"strten");
            }
            {
              size_t start[] = {itime};
              size_t count[] = {adjust_count};
              get_var(local_ncid,start,count,&_time   [itime],"mdtime");
              get_var(local_ncid,start,count,&_etotal [itime],"etotal");
              get_var(local_ncid,start,count,&_ekin   [itime],"ekin");
              try {
                if ( has_entropy )
                  get_var(local_ncid,start,count,&_entropy[itime],"entropy");
              }
              catch ( Exception e ) {
                e.ADD("Ignoring error",ERRWAR);
                std::cerr << e.fullWhat() << std::endl;
                has_entropy = false;
              }
            }
          }
          else {
            if ( imageIsTime ) {
              {
                size_t start[] = {fixedTime,itime,0,0};
                size_t count[] = {1,adjust_count,_natom,_xyz};

                get_var(local_ncid,start,count,&_xcart     [itime*_natom*_xyz],"xcart");
                get_var(local_ncid,start,count,&_xred      [itime*_natom*_xyz],"xred");
                get_var(local_ncid,start,count,&_fcart     [itime*_natom*_xyz],"fcart");
                get_var(local_ncid,start,count,&_velocities[itime*_natom*_xyz],"vel");
                count[2] = _xyz;
                get_var(local_ncid,start,count,&_rprimd    [itime*_xyz*_xyz],"rprimd");
              }
              {
                size_t start[] = {fixedTime,itime,0};
                size_t count[] = {1,adjust_count,_xyz};
                get_var(local_ncid,start,count,&_acell  [itime*_xyz],"acell");
                count[2] = 6;
                get_var(local_ncid,start,count,&_stress [itime*6]   ,"strten");
              }
              {
                size_t start[] = {fixedTime,itime};
                size_t count[] = {1,adjust_count};
                for ( unsigned i = 0 ; i < adjust_count ; ++i ) _time[i+itime] = i/phys::atu2fs;
                get_var(local_ncid,start,count,&_etotal [itime],"etotal");
                get_var(local_ncid,start,count,&_ekin   [itime],"ekin");
                get_var(local_ncid,start,count,&_entropy[itime],"entropy");
              }
            }
            else {
              size_t start[] = {itime,0,0,0};
              size_t count[] = {adjust_count,nimage,natomImg,_xyz};

              get_var(local_ncid,start,count,&_xcart     [itime*_natom*_xyz],"xcart");
              get_var(local_ncid,start,count,&_xred      [itime*_natom*_xyz],"xred");
              get_var(local_ncid,start,count,&_fcart     [itime*_natom*_xyz],"fcart");
              get_var(local_ncid,start,count,&_velocities[itime*_natom*_xyz],"vel");
              count[1] = 1; // Read only 1 rprimd since in PIMD all images have the same rprimd
              count[2] = _xyz;
              get_var(local_ncid,start,count,&_rprimd    [itime*_xyz*_xyz],"rprimd");

              count[1] = 1;
              count[2] = _xyz;
              get_var(local_ncid,start,count,&_acell  [itime*_xyz],"acell");

              count[1] = nimage;
              count[2] = 6;
              std::vector<double> tmp0(adjust_count*nimage*6);
              get_var(local_ncid,start,count,&tmp0[0],"strten");

              count[1] = {nimage};
              std::vector<double> tmp1(adjust_count*nimage);
              std::vector<double> tmp2(adjust_count*nimage);
              std::vector<double> tmp3(adjust_count*nimage);
              get_var(local_ncid,start,count,&tmp1[0],"etotal");
              get_var(local_ncid,start,count,&tmp2[0],"ekin");
              get_var(local_ncid,start,count,&tmp3[0],"entropy");

              double inv_nimage = 1./nimage;
              // Compute Ekin and stress 
              std::vector<double> stressString(6*adjust_count,0);
              for ( unsigned t = 0 ; t < adjust_count ; ++t ) {
                //Compute centroid
                std::vector<double> rc(natomImg*_xyz,0);
                double *xcart = &_xcart[(itime+t)*_natom*_xyz];
                double *fcart = &_fcart[(itime+t)*_natom*_xyz];
                // Need volume
                const double volume = geometry::det(&_rprimd[(itime+t)*9]);
                for ( unsigned s = 0 ; s < nimage ; ++s ) {
                  double *xcarts  = &xcart[s*natomImg*_xyz];
                  double *xcartsp = &xcart[((s==(nimage-1))?0:s+1)*natomImg*_xyz];
                  for ( unsigned i = 0 ; i < natomImg ; ++i ) {
                    double diffImg[3];
                    for ( unsigned c = 0 ; c < _xyz ; ++c ) {
                      rc[i*_xyz+c] += xcarts[i*_xyz+c];
                      diffImg[c] = xcartsp[i*_xyz+c] - xcarts[i*_xyz+c];
                      stressString[t*6+c] += mass[i]*diffImg[c]*diffImg[c];
                    }
                    stressString[t*6+3] += mass[i]*diffImg[1]*diffImg[2];
                    stressString[t*6+4] += mass[i]*diffImg[0]*diffImg[2];
                    stressString[t*6+5] += mass[i]*diffImg[0]*diffImg[1];
                  }
                }

                for ( unsigned i = 0 ; i < _xyz*natomImg ; ++i )
                  rc[i] *= inv_nimage;

                for ( unsigned s = 0 ; s < 6 ; ++ s ) {
                  stressString[t*6+s] *= omegaP2/volume;
                }

                double ekinSpring = 0;
                for ( unsigned s = 0 ; s < nimage ; ++s ) {
                  double *xcarts = &xcart[s*natomImg*_xyz];
                  double *fcarts = &fcart[s*natomImg*_xyz];
                  for ( unsigned i = 0 ; i < natomImg ; ++i ) {
                    for ( unsigned c = 0 ; c < 3 ; ++c )
                      ekinSpring += (rc[i*_xyz+c]-xcarts[i*_xyz+c])*fcarts[i*_xyz+c];
                  }
                }
                ekinSpring *= 0.5 * inv_nimage;
                _ekin[itime+t] = ekinImages+ekinSpring;
              }

              // Compute average of some quantities
              for ( unsigned  i = 0 ; i < adjust_count ; ++i ) {
                for ( unsigned img = 0 ; img < nimage ; ++img ) {
                  _etotal [i+itime] += tmp1[i*nimage+img];
                  _entropy[i+itime] += tmp3[i*nimage+img];
                  for ( unsigned s = 0 ; s < 6 ; ++ s )
                    _stress[(i+itime)*6+s] += tmp0[i*nimage*6+img*6+s];
                }
                _etotal [i+itime] *= inv_nimage;
                _entropy[i+itime] *= inv_nimage;
                for ( unsigned s = 0 ; s < 6 ; ++ s ) {
                  _stress[(i+itime)*6+s] = _stress[(i+itime)*6+s]*inv_nimage + stressString[i*6+s];
                }
              }
              get_var(local_ncid,start,count,&_time   [itime],"mdtime");
            }
          } 

          //
          for ( size_t k = 0 ; k < adjust_count ; ++k ) {
            auto ltime = itime + k;
            // Transpose rprimd for [[v1x v1y v1z][v2x v2y v2z] [v3x v3y v3z]] to [[v1x v2x v3x] [v1y v2y v3y] [v1z v2z v3z]]
            double swap1 = _rprimd[ltime*9+1];
            double swap2 = _rprimd[ltime*9+2];
            double swap3 = _rprimd[ltime*9+5];
            _rprimd[ltime*9+1] = _rprimd[ltime*9+3];
            _rprimd[ltime*9+2] = _rprimd[ltime*9+6];
            _rprimd[ltime*9+5] = _rprimd[ltime*9+7];
            _rprimd[ltime*9+3] = swap1;
            _rprimd[ltime*9+6] = swap2;
            _rprimd[ltime*9+7] = swap3;

            //Scale mdtime
            //if ( !has_dtion )
              _time[ltime] *= dtion;

            ( _nimage > 1 ) ? this->computeVelocitiesPressureTemperature(ltime,dtion) :
              this->computePressureTemperature(ltime);

            // Duplicate spinat at while we don't have the time evolution.
            if ( has_spinat )
              std::copy(&_spinat[0],&_spinat[_natom*_xyz],&_spinat[ltime*_natom*_xyz]);
          }
          _ntimeAvail += adjust_count;
        }
        nc_close(local_ncid);
      } 
      catch (Exception &e) {
        std::cerr << e.fullWhat() << std::endl;
        if ( _ntimeAvail == 0 ) {
          _ntimeAvail = 1;
          _ntime = 0;
        }
        nc_close(local_ncid);
      }
    if ( _ntimeAvail == _ntime && _ntime > 1 ) {
      std::ostringstream thermo;
      this->printThermo(0,_ntime,thermo);
      std::cout << thermo.str() << std::endl;
    }
#ifdef HAVE_CPPTHREAD
  });
#endif

#endif
}

//
void HistDataNC::dump(const std::string& filename, unsigned tbegin, unsigned tend, unsigned step) const {
  unsigned ntime = tend-tbegin;
  try {
    HistData::checkTimes(tbegin,tend);
#ifdef HAVE_NETCDF
    int ncid, natomid, ntypatid, npspid, xyzid, timeid, sixid;
    int dimids[3]; // Maximal number of dimension for variables.
    int rval;

    auto put_var = [](int ncid, const char* name, std::string &unit, std::string &mnemo, nc_type type, int ndims, const int* dimids, const size_t *countp, const size_t tstart, const void* data) {
      int varid;
      int rval;
      size_t startp[ndims];
      for ( int i = 0 ; i < ndims ; ++i ) startp[i] = 0;
      startp[0] = tstart;
      if ( tstart == 0 ) {
        if ( (rval = nc_redef(ncid) ) )
          throw EXCEPTION(std::string("Error entering define mode for variable")+std::string(name)+std::string(" with error ")+std::string(nc_strerror(rval)),ERRABT);
        if ( (rval = nc_def_var(ncid,name,type,ndims,dimids,&varid) ) )
          throw EXCEPTION(std::string(std::string("Error creating variable ")+std::string(name)+std::string(" with error "))+std::string(nc_strerror(rval)),ERRABT);
        if ( unit != "" || mnemo != "" ) {
          if ( (rval = nc_put_att_text(ncid, varid, "unit", unit.size(), unit.c_str()) ) )
            throw EXCEPTION(std::string(std::string("Error creating attribut unit for variable ")+std::string(name)+std::string(" with error "))+std::string(nc_strerror(rval)),ERRABT);
          if ( (rval = nc_put_att_text(ncid, varid, "mnemonics", mnemo.size(), mnemo.c_str()) ) )
            throw EXCEPTION(std::string(std::string("Error creating attribut mnemonics for variable ")+std::string(name)+std::string(" with error "))+std::string(nc_strerror(rval)),ERRABT);
        }
        if ( (rval = nc_enddef(ncid) ) )
          throw EXCEPTION(std::string("Error ending define mode with error ")+std::string(nc_strerror(rval)),ERRABT);
      }
      else {
        if ( (rval = nc_inq_varid(ncid,name,&varid)) )
          throw EXCEPTION("Unable to get variable id for"+std::string(name),ERRABT);
      }
      if ( (rval = nc_put_vara(ncid,varid,startp,countp,data) ) )
        throw EXCEPTION(std::string("Error putting variable ")+std::string(name)+std::string(" with error ")+std::string(nc_strerror(rval)),ERRABT);
    };

    // Open file
    if ( (rval = nc_create(filename.c_str(), NC_CLOBBER, &ncid) ) )
      throw EXCEPTION(std::string("Error creating file ")+filename+std::string(" with error ")+utils::to_string(rval),ERRABT);

    // Define dimensions
    if ( (rval = nc_def_dim(ncid,"natom",_natom,&natomid) ) )
      throw EXCEPTION(std::string("Error creating dimension natom with error ")+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_def_dim(ncid,"ntypat",_znucl.size(),&ntypatid) ) )
      throw EXCEPTION(std::string("Error creating dimension ntypat with error ")+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_def_dim(ncid,"npsp",_znucl.size(),&npspid) ) )
      throw EXCEPTION(std::string("Error creating dimension npsp with error ")+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_def_dim(ncid,"xyz",_xyz,&xyzid) ) )
      throw EXCEPTION(std::string("Error creating dimension xyz with error ")+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_def_dim(ncid,"time",NC_UNLIMITED,&timeid) ) )
      throw EXCEPTION(std::string("Error creating dimension time with error ")+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_def_dim(ncid,"six",6,&sixid) ) )
      throw EXCEPTION(std::string("Error creating dimension six with error ")+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_enddef(ncid) ) )
      throw EXCEPTION(std::string("Error ending define mode with error ")+std::string(nc_strerror(rval)),ERRABT);

    size_t countp[3];
    std::string units;
    std::string mnemo;

    // Write Data
    //typat(natom)
    dimids[0]=natomid;
    countp[0]=_natom;
    units = "dimensionless";
    mnemo = "types of atoms";
    put_var(ncid,"typat",units,mnemo,NC_INT,1,dimids,countp,0,&_typat[0]);

    //znucl(npsp)
    double *znucl = new double[_znucl.size()];
    for ( unsigned z = 0 ; z < _znucl.size() ; ++z )
      znucl[z] = (double) _znucl[z];
    dimids[0]=npspid;
    countp[0]=_znucl.size();
    units = "atomic units";
    mnemo = "atomic charges";
    put_var(ncid,"znucl",units,mnemo,NC_DOUBLE,1,dimids,countp,0,znucl);
    delete[] znucl;

    //amu(ntypat)
    double *amu = new double[_znucl.size()];
    for ( unsigned z = 0 ; z < _znucl.size() ; ++z )
      amu[z] = mendeleev::mass[_znucl[z]];
    dimids[0]=ntypatid;
    countp[0]=_znucl.size();
    units = "atomic units";
    mnemo = "atomic masses";
    put_var(ncid,"amu",units,mnemo,NC_DOUBLE,1,dimids,countp,0,amu);
    delete[] amu;

    //dtion
    dimids[0]=0;
    double dtion = (_ntime > 1 ? _time[1]-_time[0] : 1);
    int varid;
    units = "atomic units";
    mnemo = "time step";
    if ( (rval = nc_redef(ncid) ) )
      throw EXCEPTION(std::string("Error entering define mode ")+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_def_var(ncid,"dtion",NC_DOUBLE,0,dimids,&varid) ) )
      throw EXCEPTION(std::string("Error creating variable dtion with error ")+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_put_att_text(ncid, varid, "unit", units.size(), units.c_str()) ) )
      throw EXCEPTION(std::string(std::string("Error creating attribut unit for variable dtion with error "))+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_put_att_text(ncid, varid, "mnemonics", mnemo.size(), mnemo.c_str()) ) )
      throw EXCEPTION(std::string(std::string("Error creating attribut mnemonics for variable dtion with error "))+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_enddef(ncid) ) )
      throw EXCEPTION(std::string("Error ending define mode with error ")+std::string(nc_strerror(rval)),ERRABT);
    if ( (rval = nc_put_var_double(ncid,varid,&dtion) ) )
      throw EXCEPTION(std::string("Error putting variable dtion with error ")+std::string(nc_strerror(rval)),ERRABT);

    dimids[0]=timeid;
    dimids[1]=natomid;
    dimids[2]=xyzid;
    countp[0]=ntime;
    if ( step == 1 ) step = ntime;
    else countp[0]=1;
    size_t tstart = -1;
    for ( unsigned iitime = tbegin ; iitime < tend ; iitime += step ) {
      ++tstart;
      countp[1]=_natom;
      countp[2]=_xyz;
      //xcart(time,natom,xyz)
      units = "bohr";
      mnemo = "vectors (X) of atom positions in CARTesian coordinates" ;
      put_var(ncid,"xcart",units,mnemo,NC_DOUBLE,3,dimids,countp,tstart,&_xcart[iitime*_natom*_xyz]);

      //xred(time,natom,xyz)
      units = "dimensionless" ;
      mnemo = "vectors (X) of atom positions in REDuced coordinates" ;
      put_var(ncid,"xred",units,mnemo,NC_DOUBLE,3,dimids,countp,tstart,&_xred[iitime*_natom*_xyz]);

      //fcart(time,natom,xyz)
      units = "Ha/bohr" ;
      mnemo = "atom Forces in CARTesian coordinates" ;
      if ( _fcart.size() < 3*_ntime*_natom){
        auto f = _fcart;
        f.resize(3*_ntime*_natom);
        put_var(ncid,"fcart",units,mnemo,NC_DOUBLE,3,dimids,countp,tstart,&f[iitime*_natom*_xyz]);
      }
      else
        put_var(ncid,"fcart",units,mnemo,NC_DOUBLE,3,dimids,countp,tstart,&_fcart[iitime*_natom*_xyz]);

      //fred(time,natom,xyz)
      units = "dimensionless" ;
      mnemo = "atom Forces in REDuced coordinates" ;
      if ( _fcart.size() < 3*_ntime*_natom){
        auto f = _fcart;
        f.resize(3*_ntime*_natom);
        put_var(ncid,"fred",units,mnemo,NC_DOUBLE,3,dimids,countp,tstart,&f[iitime*_natom*_xyz]);
      }
      else
        put_var(ncid,"fred",units,mnemo,NC_DOUBLE,3,dimids,countp,tstart,&_fcart[iitime*_natom*_xyz]);

      //vel(time,natom,xyz)
      units = "bohr*Ha/hbar" ;
      mnemo = "VELocity" ;
      put_var(ncid,"vel",units,mnemo,NC_DOUBLE,3,dimids,countp,tstart,&_velocities[iitime*_natom*_xyz]);

      //acell(time,xyz)
      units = "bohr" ;
      mnemo = "CELL lattice vector scaling" ;
      dimids[1]=xyzid;
      countp[1]=_xyz;
      put_var(ncid,"acell",units,mnemo,NC_DOUBLE,2,dimids,countp,tstart,&_acell[iitime*_xyz]);

      //rprimd(time,xyz,xyz)
      double *rprimd = new double[countp[0]*_xyz*_xyz];
      for ( unsigned itime = 0 ; itime < countp[0] ; ++itime ) {
        // Transpose rprimd for [[v1x v1y v1z][v2x v2y v2z] [v3x v3y v3z]] to [[v1x v2x v3x] [v1y v2y v3y] [v1z v2z v3z]]
        rprimd[itime*9+0] = _rprimd[(iitime+itime)*9+0];
        rprimd[itime*9+1] = _rprimd[(iitime+itime)*9+3];
        rprimd[itime*9+2] = _rprimd[(iitime+itime)*9+6];
        rprimd[itime*9+3] = _rprimd[(iitime+itime)*9+1];
        rprimd[itime*9+4] = _rprimd[(iitime+itime)*9+4];
        rprimd[itime*9+5] = _rprimd[(iitime+itime)*9+7];
        rprimd[itime*9+6] = _rprimd[(iitime+itime)*9+2];
        rprimd[itime*9+7] = _rprimd[(iitime+itime)*9+5];
        rprimd[itime*9+8] = _rprimd[(iitime+itime)*9+8];
      }
      units = "bohr" ;
      mnemo = "Real space PRIMitive translations, Dimensional" ;
      put_var(ncid,"rprimd",units,mnemo,NC_DOUBLE,3,dimids,countp,tstart,rprimd);

      for ( unsigned itime = 0 ; itime < countp[0] ; ++itime ) {
        rprimd[itime*9+0] = 0;
        rprimd[itime*9+1] = 0;
        rprimd[itime*9+2] = 0;
        rprimd[itime*9+3] = 0;
        rprimd[itime*9+4] = 0;
        rprimd[itime*9+5] = 0;
        rprimd[itime*9+6] = 0;
        rprimd[itime*9+7] = 0;
        rprimd[itime*9+8] = 0;
      }
      units = "bohr*Ha/hbar" ;
      mnemo = "VELocities of cell" ;
      put_var(ncid,"vel_cell",units,mnemo,NC_DOUBLE,3,dimids,countp,tstart,rprimd);
      delete[] rprimd;

      //etotal(time)
      units = "Ha" ;
      mnemo = "TOTAL Energy" ;
      put_var(ncid,"etotal",units,mnemo,NC_DOUBLE,1,dimids,countp,tstart,&_etotal[iitime]);

      //ekin(time)
      units = "Ha" ;
      mnemo = "Energy KINetic ionic" ;
      put_var(ncid,"ekin",units,mnemo,NC_DOUBLE,1,dimids,countp,tstart,&_ekin[iitime]);

      //entropy(time)
      units =  "";
      mnemo = "Entropy" ;
      put_var(ncid,"entropy",units,mnemo,NC_DOUBLE,1,dimids,countp,tstart,&_entropy[iitime]);

      //mdtime(time)
      units = "hbar/Ha" ;
      mnemo = "Molecular Dynamics TIME" ;
      {
        auto tmp_time = _time;
        for ( auto& t : tmp_time ) t /= dtion;
        put_var(ncid,"mdtime",units,mnemo,NC_DOUBLE,1,dimids,countp,tstart,&tmp_time[iitime]);
      }

      //strten(time,six)
      dimids[1]=sixid;
      countp[1]=6;
      units = "Ha/bohr^3" ;
      mnemo = "STRess tensor" ;
      put_var(ncid,"strten",units,mnemo,NC_DOUBLE,2,dimids,countp,tstart,&_stress[iitime*6]);
    }

    // Close file
    if ( (rval = nc_close(ncid) ) )
      throw EXCEPTION(std::string("Error closingfile ")+filename+std::string(" with error ")+utils::to_string(rval),ERRABT);


#else
    (void) filename;
#endif
  }
  catch ( Exception &e ) {
    e.ADD("Dumping failed",ERRABT);
    throw e;
  }
}

void HistDataNC::dump(HistData &hist, const std::string& filename, unsigned tbegin, unsigned tend, unsigned step) {
  HistDataMD *histmd;
  if ( ( histmd = dynamic_cast<HistDataMD*>(&hist) ) ) {
    reinterpret_cast<HistDataNC*>(histmd)->dump(filename,tbegin,tend,step);
  }
  else {
    HistDataNC histTmp(std::move(hist));
    histTmp.dump(filename,tbegin,tend,step);
    hist = std::move(histTmp);
  }
}


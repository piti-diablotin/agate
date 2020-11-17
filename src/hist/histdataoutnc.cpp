/**
 * @file src/histdataoutnc.cpp
 *
 * @brief Read a _OUT.nc file in netcdf
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
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


#include "hist/histdataoutnc.hpp"
#include "base/exception.hpp"
#include "base/utils.hpp"
#include "base/phys.hpp"
#include "base/mendeleev.hpp"
#include "base/geometry.hpp"
#include <algorithm>
#ifdef HAVE_NETCDFCXX4
#include <memory>
#include <typeinfo>
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
HistDataOutNC::HistDataOutNC() : HistData()
{
}

//
HistDataOutNC::~HistDataOutNC() {
}


//
void HistDataOutNC::readFromFile(const std::string& filename) {
#ifndef HAVE_NETCDF
  throw EXCEPTION("NetCDF support is not available.\nConsider compiling the code with NetCDF support to read "+filename,ERRDIV);
#else

  _xyz = 3;
  _ntime = 1;
  _ntimeAvail = _ntime;

  int ncid = 0;
  int varid = 0;
  int dimid = 0;


  if ( nc_open(filename.c_str(), NC_NOWRITE, &ncid)) 
    throw EXCEPTION(std::string("File ")+filename+" could not be correctly opened",ERRDIV);

  {
    if ( !nc_inq_varid(ncid, "ndtset", &varid) ) {
      size_t start[] = {0};
      if ( nc_get_var1_int(ncid, varid, start, (int*)&_ntime ) ) {
        nc_close(ncid);
        throw EXCEPTION(std::string("Issue reading ndtset in ")+filename,ERRABT);
      }
    }
  }


  // Read dimensions.
  {
    size_t start[] = {0};
    nc_inq_varid(ncid, "natom", &varid);
    int natom_tmp;
    if ( nc_get_var1_int(ncid, varid, start, &natom_tmp ) ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading natom var in ")+filename,ERRDIV);
    }
    _natom = natom_tmp;
  }

  // Allocate arrays
  _xcart  .resize(_ntime*_natom*_xyz);
  _xred   .resize(_ntime*_natom*_xyz);
  _fcart  .resize(_ntime*_natom*_xyz);
  _acell  .resize(_ntime*_xyz);
  _rprimd .resize(_ntime*_xyz*_xyz);
  _time   .resize(_ntime);
  _stress .resize(_ntime*6);
  _etotal .resize(_ntime);

  // Read data.
  {
    size_t start[] = {0};
    nc_inq_varid(ncid, "znucl", &varid);
    size_t ntypat;
    nc_inq_vardimid(ncid, varid, &dimid);
    nc_inq_dimlen(ncid, dimid, &ntypat);

    _znucl.resize(ntypat);
    size_t count[] = {ntypat};
    if ( nc_get_vara_int(ncid, varid, start, count, _znucl.data()) ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading znucl var in ")+filename,ERRDIV);
    }

    _typat.resize(_natom);
    nc_inq_varid(ncid, "typat", &varid);
    count[0] = _natom;
    if ( nc_get_vara_int(ncid, varid, start, count, _typat.data()) ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading typat var in ")+filename,ERRDIV);
    }
  }

  // Read data.
  for ( unsigned itime = 0 ; itime < _ntime ; ++itime ) {
    int status;
    std::string suffix = utils::to_string(itime+1);
    std::string token;
    std::string tokenS;
    {
      token = "xred";
      tokenS = token+suffix;
      size_t start[] = {0};
      size_t count[] = {_natom*_xyz};
      status = nc_inq_varid(ncid, tokenS.c_str(), &varid);
      if ( status ) status = nc_inq_varid(ncid, token.c_str(), &varid);
      status += nc_get_vara_double(ncid, varid, start, count, &_xred[itime*_xyz*_natom]);
      if ( status ) {
        nc_close(ncid);
        throw EXCEPTION(std::string("Error while reading xred var in ")+filename,ERRDIV);
      }

      token = "fcart";
      tokenS = token+suffix;
      status = nc_inq_varid(ncid, tokenS.c_str(), &varid);
      if ( status ) status = nc_inq_varid(ncid, token.c_str(), &varid);
      if ( status == 0 ) {
        status += nc_get_vara_double(ncid, varid, start, count, &_fcart[itime*_xyz*_natom]);
        if ( status ) _fcart.clear();
      }

      token = "spinat";
      tokenS = token+suffix;
      status = nc_inq_varid(ncid, tokenS.c_str(), &varid);
      if ( status ) status = nc_inq_varid(ncid, token.c_str(), &varid);
      if ( status == 0 ) {
        _spinat.resize(_natom*_xyz);
        count[0] = _natom*_xyz;
        status += nc_get_vara_double(ncid, varid, start, count, &_spinat[itime*_xyz*_natom]);
        if ( status ) {
          _spinat.clear();
          nc_close(ncid);
          throw EXCEPTION(std::string("Error while reading spinat var in ")+filename,ERRDIV);
        }
      }
    }
    {
      size_t start[] = {0};
      size_t count[] = {_xyz};
      token = "acell";
      tokenS = token+suffix;
      status = nc_inq_varid(ncid, tokenS.c_str(), &varid);
      if ( status ) status = nc_inq_varid(ncid, token.c_str(), &varid);
      status += nc_get_vara_double(ncid, varid, start, count, &_acell[itime*_xyz] );
      if ( status ) {
        nc_close(ncid);
        throw EXCEPTION(std::string("Error while reading acell var in ")+filename,ERRDIV);
      }
    }
    {
      size_t start[] = {0};
      size_t count[] = {_xyz*_xyz};
      token = "rprim";
      tokenS = token+suffix;
      status = nc_inq_varid(ncid, tokenS.c_str(), &varid);
      if ( status ) status = nc_inq_varid(ncid, token.c_str(), &varid);
      status += nc_get_vara_double(ncid, varid, start, count, &_rprimd[itime*_xyz*_xyz] );
      if ( status ) {
        nc_close(ncid);
        throw EXCEPTION(std::string("Error while reading rprimd var in ")+filename,ERRDIV);
      }
    }
    {
      size_t start[] = {0};
      size_t count[] = {1};
      token = "etotal";
      tokenS = token+suffix;
      status = nc_inq_varid(ncid, tokenS.c_str(), &varid);
      if ( status ) status = nc_inq_varid(ncid, token.c_str(), &varid);
      status += nc_get_vara_double(ncid, varid, start, count, &_etotal[itime] );
    }
    {
      size_t start[] = {0};
      size_t count[] = {6};
      token = "strten";
      tokenS = token+suffix;
      status = nc_inq_varid(ncid, tokenS.c_str(), &varid);
      if ( status ) status = nc_inq_varid(ncid, token.c_str(), &varid);
      status += nc_get_vara_double(ncid, varid, start, count, &_stress[itime*6] );
    }
    {
      size_t start[] = {0};
      size_t count[] = {_znucl.size()};
      std::vector<double> amu(_znucl.size());
      token = "amu";
      tokenS = token+suffix;
      status = nc_inq_varid(ncid, tokenS.c_str(), &varid);
      if ( status ) status = nc_inq_varid(ncid, token.c_str(), &varid);
      status += nc_get_vara_double(ncid, varid, start, count, &amu[0] );
      if ( status == 0 ) {
        for ( unsigned z = 0 ; z < _znucl.size() ; ++z )
          MendeTable.mass[_znucl[z]] = amu[z];
      }
    }
  }

  nc_close(ncid);

  // Transpose rprimd for [[v1x v1y v1z][v2x v2y v2z] [v3x v3y v3z]] to [[v1x v2x v3x] [v1y v2y v3y] [v1z v2z v3z]]
  for ( unsigned itime = 0 ; itime < _ntime ; ++itime ) {
    double swap1 = _rprimd[itime*9+1];
    double swap2 = _rprimd[itime*9+2];
    double swap3 = _rprimd[itime*9+5];
    _rprimd[itime*9+1] = _rprimd[itime*9+3];
    _rprimd[itime*9+2] = _rprimd[itime*9+6];
    _rprimd[itime*9+5] = _rprimd[itime*9+7];
    _rprimd[itime*9+3] = swap1;
    _rprimd[itime*9+6] = swap2;
    _rprimd[itime*9+7] = swap3;
    _rprimd[itime*9+0] *= _acell[itime*3+0];
    _rprimd[itime*9+1] *= _acell[itime*3+1];
    _rprimd[itime*9+2] *= _acell[itime*3+2];
    _rprimd[itime*9+3] *= _acell[itime*3+0];
    _rprimd[itime*9+4] *= _acell[itime*3+1];
    _rprimd[itime*9+5] *= _acell[itime*3+2];
    _rprimd[itime*9+6] *= _acell[itime*3+0];
    _rprimd[itime*9+7] *= _acell[itime*3+1];
    _rprimd[itime*9+8] *= _acell[itime*3+2];

    for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
      for ( unsigned icart = 0 ; icart < 3 ; ++icart ) {
        _xcart[itime*_natom*3+iatom*3+icart] = 0;
        for ( unsigned dir = 0 ; dir < 3 ; ++dir ) {
          _xcart[itime*_natom*3+iatom*3+icart] += 
            _rprimd[itime*9+geometry::mat3dind(icart+1,dir+1)]*_xred[itime*_natom*3+iatom*3+dir];
        }
      }
    }
  }

  _filename = filename;

  //Scale mdtime
  for ( unsigned itime = 0 ; itime < _ntime ; ++itime ) 
    _time[itime] = itime;

  _ntimeAvail = _ntime;
#endif
}

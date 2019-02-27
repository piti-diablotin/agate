/**
 * @file src/eigparserphbst.cpp
 *
 * @brief 
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


#include "io/eigparserphbst.hpp"
#include "base/exception.hpp"
#include "base/utils.hpp"
#include "base/mendeleev.hpp"
#include <iostream>
#include <fstream>
#include <utility>
#include <iomanip>
#include <sstream>
#ifdef HAVE_NETCDF
#include <netcdf.h>
#endif

//
EigParserPHBST::EigParserPHBST() : EigParserPhonons() {
  ;
}

//
EigParserPHBST::~EigParserPHBST() {
  ;
}

//
void EigParserPHBST::readFromFile(const std::string& filename) {
  _dtset.reset(new EtsfNC);
  _dtset->readFromFile(filename);
#ifdef HAVE_NETCDF
  unsigned kpts;
  unsigned nband;

  int ncid = 0;
  int varid = 0;
  int dimid = 0;

  if ( nc_open(filename.c_str(), NC_NOWRITE, &ncid)) 
    throw EXCEPTION(std::string("File ")+filename+" could not be correctly opened",ERRDIV);

  if (  nc_inq_ndims(ncid, &dimid) || dimid < 2 ) {
    nc_close(ncid);
    throw EXCEPTION(std::string("Bad number of dimension: ")+utils::to_string(dimid)+
        std::string(" instead of being at least 2"),ERRDIV);
  }

  if ( nc_inq_nvars(ncid, &varid) || varid < 2 ) {
    nc_close(ncid);
    throw EXCEPTION(std::string("Bad number of variables: ")+utils::to_string(varid)+
        std::string(" instead of being at least 2"),ERRABT);
  }

  size_t dimval;
  nc_inq_dimid(ncid, "number_of_phonon_modes", &dimid);
  nc_inq_dimlen(ncid, dimid, &dimval);
  nband = (unsigned) dimval;
  _nband = nband;
  nc_inq_dimid(ncid, "number_of_qpoints", &dimid);
  nc_inq_dimlen(ncid, dimid, &dimval);
  kpts = (unsigned) dimval;
  _hasSpin = false;

  _eunit = UnitConverter(UnitConverter::eV);

  geometry::vec3d prev_kpt = {{0.,0.,0.}};
  double length = 0.0;

  for ( unsigned ikpt = 0 ; ikpt < kpts ; ++ikpt ) {
    int status = nc_inq_varid(ncid, "qpoints", &varid);
    size_t start2[] = {ikpt,0};
    size_t count2[] = {1,3};
    geometry::vec3d kpt;
    status += nc_get_vara_double(ncid, varid, start2, count2, &kpt[0]);
    if ( status ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading qpoints var in ")+filename,ERRDIV);
    }
    std::vector<double> values(nband,0);
    std::vector<double> disp(nband*nband*2,0);
    status = nc_inq_varid(ncid, "phfreqs", &varid);
    size_t start3[] = {ikpt,0};
    size_t count3[] = {1,nband};
    status += nc_get_vara_double(ncid, varid, start3, count3, values.data());
    if ( status ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading phfreqs var in ")+filename,ERRDIV);
    }
    _eigens.push_back(std::move(values));

    status = nc_inq_varid(ncid, "phdispl_cart", &varid);
    size_t start4[] = {ikpt,0,0,0};
    size_t count4[] = {1,nband,nband,2};
    status += nc_get_vara_double(ncid, varid, start4, count4, disp.data());
    if ( status ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading phdisp_cart var in ")+filename,ERRDIV);
    }

    _eigenDisp.push_back(std::move(disp));
    if ( ikpt == 0 ) prev_kpt = kpt;
    length += geometry::norm(geometry::operator-(kpt,prev_kpt));
    _kpts.push_back(kpt);
    _lengths.push_back(length);
    prev_kpt = kpt;
  }
  this->renormalizeEigenDisp();

#else
  throw EXCEPTION("NetCDF support is off",ERRDIV);
  (void) filename;
#endif
}

/**
 * @file src/eigparsergsr.cpp
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


#include "io/eigparsergsr.hpp"
#include "base/exception.hpp"
#include "base/utils.hpp"
#include <iostream>
#include <fstream>
#include <utility>
#include <iomanip>
#include <sstream>
#ifdef HAVE_NETCDF
#include <netcdf.h>
#endif

//
EigParserGSR::EigParserGSR() : EigParserElectrons() {
  ;
}

//
EigParserGSR::~EigParserGSR() {
  ;
}

//
void EigParserGSR::readFromFile(const std::string& filename) {

  unsigned kpts;
  unsigned nband;
  unsigned nspin;
  std::string tmp;
  double kx=0.0, ky=0.0, kz=0.0;
  double value=0.0;

  try {
#ifdef HAVE_NETCDF
    int ncid = 0;
    int varid = 0;
    int dimid = 0;

    if ( nc_open(filename.c_str(), NC_NOWRITE, &ncid)) 
      throw EXCEPTION(std::string("File ")+filename+" could not be correctly opened",ERRDIV);

    size_t dimval;
    nc_inq_dimid(ncid, "max_number_of_states", &dimid);
    nc_inq_dimlen(ncid, dimid, &dimval);
    nband = (unsigned) dimval;
    _nband = nband;
    nc_inq_dimid(ncid, "number_of_kpoints", &dimid);
    nc_inq_dimlen(ncid, dimid, &dimval);
    kpts = (unsigned) dimval;
    nc_inq_dimid(ncid, "number_of_spins", &dimid);
    nc_inq_dimlen(ncid, dimid, &dimval);
    nspin = (unsigned) dimval;
    _hasSpin = ( nspin == 2 );

    int status = nc_inq_varid(ncid, "eigenvalues", &varid);
    size_t attlen;
    char *units = nullptr;
    status = nc_inq_attlen (ncid, varid, "units", &attlen);
    if ( status ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while inquiring attribute length of eigenvalues:units in ")+filename,ERRDIV);
    }

    units = new char[attlen+1];
    status = nc_get_att_text(ncid, varid, "units", units);
    if ( status ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while inquiring attribute eigenvalues:units in ")+filename,ERRDIV);
    }
    units[attlen] = '\0';
    std::string sunit(units);
    delete[] units;

    if ( sunit == "Hartree" || "atomic units")
      _eunit = UnitConverter(UnitConverter::Ha);
    else if ( sunit == "eV" )
      _eunit = UnitConverter(UnitConverter::eV);
    else
      _eunit = UnitConverter(UnitConverter::Ha);

    status = nc_inq_varid(ncid, "fermi_energy", &varid);
    units = nullptr;
    status = nc_inq_attlen (ncid, varid, "units", &attlen);
    if ( status ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while inquiring attribute fermi_energy:units in ")+filename,ERRDIV);
    }

    units = new char[attlen+1];
    status = nc_get_att_text(ncid, varid, "units", units);
    if ( status ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while inquiring attribute length of fermi_energy:units in ")+filename,ERRDIV);
    }
    units[attlen] = '\0';
    std::string funit(units);
    delete[] units;

    UnitConverter feunit(_eunit);
    if ( funit == "Hartree" || "atomic units")
      feunit = UnitConverter::Ha;
    else if ( sunit == "eV" )
      feunit = UnitConverter::eV;
    else
      feunit = UnitConverter::Ha;
    double fermi = 0;
    
    size_t start[] = {0};
    status += nc_get_var1_double(ncid, varid, start, &fermi);
    if ( status ) fermi = 0;

    for ( unsigned ispin = 0 ; ispin < nspin ; ++ispin ) {
      geometry::vec3d prev_kpt = {{0.,0.,0.}};
      double length = 0.0;

      for ( unsigned ikpt = 0 ; ikpt < kpts ; ++ikpt ) {
        status = nc_inq_varid(ncid, "reduced_coordinates_of_kpoints", &varid);
        size_t start2[] = {ikpt,0};
        size_t count2[] = {1,3};
        geometry::vec3d kpt;
        status += nc_get_vara_double(ncid, varid, start2, count2, &kpt[0]);
        if ( status ) {
          nc_close(ncid);
          throw EXCEPTION(std::string("Error while reading Kptns var in ")+filename,ERRDIV);
        }
        std::vector<double> values(nband,0);
        status = nc_inq_varid(ncid, "eigenvalues", &varid);
        size_t start3[] = {ispin,ikpt,0};
        size_t count3[] = {1,1,_nband};
        status += nc_get_vara_double(ncid, varid, start3, count3, values.data());
        if ( status ) {
          nc_close(ncid);
          throw EXCEPTION(std::string("Error while reading Eigenvalues var in ")+filename,ERRDIV);
        }
        for ( auto& v : values ) v -= fermi/feunit*_eunit;
        _eigens.push_back(std::move(values));
        if ( ikpt == 0 ) prev_kpt = kpt;
        length += geometry::norm(geometry::operator-(kpt,prev_kpt));
        _kpts.push_back(kpt);
        _lengths.push_back(length);
        prev_kpt = kpt;
      }
    }

#else
    throw EXCEPTION("NetCDF support is off",ERRDIV);
#endif
  }
  catch (Exception &e) {
    e.ADD(std::string("Error while reading _GSR file ")+filename,ERRDIV);
    throw e;
  }
  catch(...) {
    throw EXCEPTION(std::string("Something went wrong reading file ")
        +filename,ERRDIV);
  }
}



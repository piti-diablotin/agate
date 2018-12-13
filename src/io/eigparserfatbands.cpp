/**
 * @file src/./eigparserfatbands.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2018 Jordan Bieder
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


#include "io/eigparserfatbands.hpp"
#include "base/exception.hpp"
#include "base/utils.hpp"
#include "io/etsfnc.hpp"
#ifdef HAVE_NETCDF
#include <netcdf.h>
#endif

//
EigParserFatbands::EigParserFatbands() : EigParserElectrons(){
  ;
}

//
EigParserFatbands::~EigParserFatbands() {
  ;
}

//
void EigParserFatbands::readFromFile(const std::string& filename) {
  unsigned kpts;
  unsigned nband;
  unsigned nspin;

  _dtset.reset(new EtsfNC);
  _dtset->readFromFile(filename);
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

  nc_inq_dimid(ncid, "natsph", &dimid);
  nc_inq_dimlen(ncid, dimid, &dimval);
  if ( dimval != _dtset->natom() )
    throw EXCEPTION("natom is different than natsph.",ERRABT);

  nc_inq_dimid(ncid, "mbesslang", &dimid);
  nc_inq_dimlen(ncid, dimid, &dimval);
  _lmax = (int) dimval;
  _lmMask.resize(_lmax*_lmax,1);

  nc_inq_dimid(ncid, "dos_fractions_m_lastsize", &dimid);
  nc_inq_dimlen(ncid, dimid, &dimval);
  size_t nfraction;
  if ( (nfraction = (size_t)_lmax*_lmax*_dtset->natom()) != dimval )
    throw EXCEPTION("Unconsistent data for dos_fractions_m_lastsize",ERRABT);

  nc_inq_dimid(ncid, "number_of_spinor_components", &dimid);
  nc_inq_dimlen(ncid, dimid, &dimval);
  if ( dimval != 1 )
    throw EXCEPTION("nspinor is 2. This case is not yet implemented.",ERRDIV);

  _eunit = Ha;
  double efermi;
  int status = nc_inq_varid(ncid, "fermi_energy", &varid);
  size_t start[] = {0};
  status += nc_get_var1_double(ncid, varid, start, &efermi);

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
        throw EXCEPTION(std::string("Error while reading kpoints var in ")+filename,ERRDIV);
      }
      std::vector<double> values(nband,0);
      std::vector<double> fraction(_nband*nfraction,0);

      status = nc_inq_varid(ncid, "eigenvalues", &varid);
      size_t start3[] = {ispin,ikpt,0};
      size_t count3[] = {1,1,_nband};
      status += nc_get_vara_double(ncid, varid, start3, count3, values.data());
      if ( status ) {
        nc_close(ncid);
        throw EXCEPTION(std::string("Error while reading eigenvalues var in ")+filename,ERRDIV);
      }

      status = nc_inq_varid(ncid, "dos_fractions_m", &varid);
      size_t start4[] = {0,ispin,0,ikpt};
      size_t count4[] = {nfraction,1,_nband,1};
      status += nc_get_vara_double(ncid, varid, start4, count4, fraction.data());
      if ( status ) {
        nc_close(ncid);
        throw EXCEPTION(std::string("Error while reading dos_fractions var in ")+filename,ERRDIV);
      }
      for ( auto &eig : values ) eig -= efermi;
      _eigens.push_back(std::move(values));
      _fractions.push_back(std::move(fraction));

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

/**
 * @file src/./etsfnc.cpp
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


#include "io/etsfnc.hpp"
#include "base/exception.hpp"
#include "base/utils.hpp"
#include "base/phys.hpp"
#include "base/geometry.hpp"

#ifdef HAVE_NETCDF
#include <netcdf.h>
#endif


//
EtsfNC::EtsfNC() {
  ;
}

//
EtsfNC::~EtsfNC() {
  ;
}

void EtsfNC::readFromFile(const std::string& filename) {
#ifndef HAVE_NETCDF
  throw EXCEPTION("NetCDF support is not available.\nConsider compiling the code with NetCDF support to read "+filename,ERRDIV);
#else

  int ncid = 0;
  int varid = 0;
  int dimid = 0;
  int natomid = 0;
  int xyzid = 0;

  if ( nc_open(filename.c_str(), NC_NOWRITE, &ncid)) 
    throw EXCEPTION(std::string("File ")+filename+" could not be correctly opened",ERRDIV);

  // Read dimensions.
  {
    nc_inq_dimid(ncid, "number_of_atoms", &natomid);
    size_t natom;
    if ( nc_inq_dimlen(ncid, natomid, &natom ) ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading number_of_atoms var in ")+filename,ERRDIV);
    }
    _natom = natom;
  }
  {
    nc_inq_dimid(ncid, "number_of_vectors", &varid);
    size_t xyz;
    if ( nc_inq_dimlen(ncid, varid, &xyz ) ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading number_of_vectors var in ")+filename,ERRDIV);
    }
    if ( xyz != 3 ) 
      throw EXCEPTION(std::string("number_of_vectors should be 3 and is ")+utils::to_string(xyz),ERRDIV);
  }
  {
    nc_inq_dimid(ncid, "number_of_cartesian_directions", &xyzid);
    size_t xyz;
    if ( nc_inq_dimlen(ncid, xyzid, &xyz ) ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading number_of_cartesian_directions var in ")+filename,ERRDIV);
    }
    if ( xyz != 3 ) 
      throw EXCEPTION(std::string("number_of_cartesian_directions should be 3 and is ")+utils::to_string(xyz),ERRDIV);
  }

  // Allocate arrays
  std::vector<double> xred(_natom*3,0);
  
  // Read data.
  {
    size_t start[] = {0};
    nc_inq_varid(ncid, "atomic_numbers", &varid);
    size_t ntypat;
    nc_inq_vardimid(ncid, varid, &dimid);
    nc_inq_dimlen(ncid, dimid, &ntypat);
    _ntypat = (unsigned int) ntypat;

    _znucl.resize(ntypat);
    size_t count[] = {ntypat};
    if ( nc_get_vara_int(ncid, varid, start, count, _znucl.data()) ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading atomic_numbers var in ")+filename,ERRDIV);
    }

    _typat.resize(_natom);
    nc_inq_varid(ncid, "atom_species", &varid);
    count[0] = _natom;
    if ( nc_get_vara_int(ncid, varid, start, count, _typat.data()) ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while reading atom_species var in ")+filename,ERRDIV);
    }
  }

  auto get_var = [this,filename](int ncid, size_t start[], size_t count[], double *data,const char name[]) {
    int varid;
    int status = nc_inq_varid(ncid, name, &varid);
    if ( status ) {
      nc_close(ncid);
      std::ostringstream msg;
      msg << "Error while inquiring " << name << " var in " << filename << " with error " << status;
      throw EXCEPTION(msg.str(),ERRDIV);
    }
    status = nc_get_vara_double(ncid, varid, start, count, data);
    if ( status ) {
      nc_close(ncid);
      std::ostringstream msg;
      msg << "Error while reading " << name << " var in " << filename << " with error " << status;
      throw EXCEPTION(msg.str(),ERRDIV);
    }
  };

  size_t start[2] = {0};
  size_t count[2] = {_natom,3};
  get_var(ncid, start, count, xred.data(), "reduced_atom_positions");

  _xred.clear();
  for ( unsigned iatom = 0; iatom < _natom; ++iatom ) {
    _xred.push_back({{xred[iatom*3], xred[iatom*3+1], xred[iatom*3+2]}});
  }

  count[0] = 3;
  count[1] = 3;
  get_var(ncid, start, count, _rprim.data(), "primitive_vectors");
  double swap1 = _rprim[1];
  double swap2 = _rprim[2];
  double swap3 = _rprim[5];
  _rprim[1] = _rprim[3];
  _rprim[2] = _rprim[6];
  _rprim[5] = _rprim[7];
  _rprim[3] = swap1;
  _rprim[6] = swap2;
  _rprim[7] = swap3;

  _acell[0]=1;
  _acell[1]=1;
  _acell[2]=1;

  _gprim = geometry::invertTranspose(_rprim);
  geometry::changeBasis(_rprim, _xcart, _xred, false);

#endif
}

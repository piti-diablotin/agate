/**
 * @file src/eigparsereig.cpp
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


#include "io/eigparsereig.hpp"
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
EigParserEIG::EigParserEIG() : EigParser() {
  ;
}

//
EigParserEIG::~EigParserEIG() {
  ;
}

//
void EigParserEIG::readFromFile(const std::string& filename) {

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

    if (  nc_inq_ndims(ncid, &dimid) || dimid != 4 ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Bad number of dimension: ")+utils::to_string(dimid)+
          std::string(" instead of being 4"),ERRDIV);
    }

    if ( nc_inq_nvars(ncid, &varid) || varid != 3 ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Bad number of variables: ")+utils::to_string(varid)+
          std::string(" instead of being 3"),ERRABT);
    }

    size_t dimval;
    nc_inq_dimid(ncid, "mband", &dimid);
    nc_inq_dimlen(ncid, dimid, &dimval);
    nband = (unsigned) dimval;
    _nband = nband;
    nc_inq_dimid(ncid, "nkpt", &dimid);
    nc_inq_dimlen(ncid, dimid, &dimval);
    kpts = (unsigned) dimval;
    nc_inq_dimid(ncid, "nsppol", &dimid);
    nc_inq_dimlen(ncid, dimid, &dimval);
    nspin = (unsigned) dimval;
    _hasSpin = ( nspin == 2 );

    int status = nc_inq_varid(ncid, "Eigenvalues", &varid);
    size_t attlen;
    char *units = nullptr;
    status = nc_inq_attlen (ncid, varid, "units", &attlen);
    if ( status ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while inquiring attribute length of Eigenvalues:units in ")+filename,ERRDIV);
    }

    units = new char[attlen+1];
    status = nc_get_att_text(ncid, varid, "units", units);
    if ( status ) {
      nc_close(ncid);
      throw EXCEPTION(std::string("Error while inquiring attribute Eigenvalues:units in ")+filename,ERRDIV);
    }
    units[attlen] = '\0';
    std::string sunit(units);
    delete[] units;
    if ( sunit == "Hartree" )
      _eunit = Ha;
    else if ( sunit == "eV" )
      _eunit = eV;
    else
      _eunit = Ha;


    for ( unsigned ispin = 0 ; ispin < nspin ; ++ispin ) {
      geometry::vec3d prev_kpt = {{0.,0.,0.}};
      double length = 0.0;

      for ( unsigned ikpt = 0 ; ikpt < kpts ; ++ikpt ) {
        status = nc_inq_varid(ncid, "Kptns", &varid);
        size_t start2[] = {ikpt,0};
        size_t count2[] = {1,3};
        geometry::vec3d kpt;
        status += nc_get_vara_double(ncid, varid, start2, count2, &kpt[0]);
        if ( status ) {
          nc_close(ncid);
          throw EXCEPTION(std::string("Error while reading Kptns var in ")+filename,ERRDIV);
        }
        std::vector<double> values(nband,0);
        status = nc_inq_varid(ncid, "Eigenvalues", &varid);
        size_t start3[] = {ispin,ikpt,0};
        size_t count3[] = {1,1,_nband};
        status += nc_get_vara_double(ncid, varid, start3, count3, values.data());
        if ( status ) {
          nc_close(ncid);
          throw EXCEPTION(std::string("Error while reading Eigenvalues var in ")+filename,ERRDIV);
        }
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
    std::ifstream eig(filename.c_str(), std::ios::in);
    if ( !eig ) {
      e.ADD(std::string("Unable to read the file ")+filename,ERRABT);
      throw e;
    }

    // Read first line
    eig >> tmp; // Should read Eigenvalues
    if ( tmp.compare("Eigenvalues") != 0 ) {
      e.ADD(std::string("File ")+filename+
          std::string(" does not seem to be a correct _EIG file"),ERRDIV);
      throw e;
    }

    std::getline(eig,tmp,')'); //Should be the unit inside parenthesis
    utils::trim(tmp," ()\f\t\v");
    if ( tmp.compare("hartree") == 0 ) _eunit = Ha;
    else if ( tmp.compare("eV") == 0 ) _eunit = eV;
    else {
      e.ADD(std::string("Unknown energy unit ")+tmp,ERRDIV);
      throw e;
    }

    std::getline(eig,tmp,'=');
    eig >> kpts;
    std::getline(eig,tmp);
    if ( tmp.find("SPIN") != std::string::npos ) _hasSpin = true;
    nspin = ( _hasSpin ? 2 : 1 );
    if ( !_hasSpin )
      std::clog << "Will read " << kpts << " k-pts from file " << filename << std::endl;
    else
      std::clog << "Will read " << kpts << " k-pts for 2 spin components from file " << filename << std::endl;

    eig.seekg(0);

    try {
      for ( unsigned ispin = 0 ; ispin < nspin ; ++ispin ) {
        std::getline(eig,tmp); // Ignore first line;
        unsigned ikpt = 0;
        geometry::vec3d prev_kpt = {{0.,0.,0.}};
        double length = 0.0;
        while ( !eig.eof() && ikpt < kpts ) {
          //Parse first line
          std::getline(eig,tmp,'=');
          eig >> nband;
          if ( eig.fail() ) { e.ADD("Cannot read nband",ERRDIV); throw e;}
          if ( nband < _nband ) _nband = nband;
          std::getline(eig,tmp,'='); // wtk=
          std::getline(eig,tmp,'='); //kpt=
          eig >> kx >> ky >> kz;
          if ( eig.fail() )  {
            e.ADD(std::string("Cannot read coordinates for kpt ")
                +utils::to_string(ikpt+1),ERRDIV);
            throw e;
          }
          std::getline(eig,tmp); // end line;
          std::vector<double> values;
          try {
            for ( unsigned iband = 0 ; iband < nband ; ++iband ) {
              eig >> value;
              if ( eig.fail() ) {
                e.ADD(std::string("Cannot read enough bands: ")
                    +utils::to_string(iband-1)+std::string(" instead of ")
                    +utils::to_string(nband),ERRDIV);
                throw e;
              }
              values.push_back(value);
            }
          }
          catch(Exception& e) {
            e.ADD(std::string("Something went wrong reading kpt ")
                +utils::to_string(ikpt+1),ERRDIV);
            throw e;
          }
          _eigens.push_back(std::move(values));
          _kpts.push_back({{kx,ky,kz}});
          if ( ikpt == 0 ) prev_kpt = {{kx,ky,kz}};
          length += geometry::norm(geometry::operator-({{kx,ky,kz}},prev_kpt));
          _lengths.push_back(length);
          prev_kpt = {{kx,ky,kz}};
          ++ikpt;
        }
        if ( !eig.eof() )
          std::getline(eig,tmp); // Ignore first line;
      }
    }
    catch (Exception& e) {
      e.ADD(std::string("Error while reading _EIG file ")+filename,ERRDIV);
      throw e;
    }
    catch(...) {
      throw EXCEPTION(std::string("Something went wrong reading file ")
          +filename,ERRDIV);
    }
    eig.close();
  }
}

//
std::string EigParser::dump(unsigned options) const {
  std::ostringstream str;
  unsigned nkpt = _kpts.size();
  unsigned nspin = ( _hasSpin ? 2 : 1 );
  const unsigned w=16;

  if ( _hasSpin ) {
    nkpt /= 2;
    if ( nkpt*2 != _kpts.size() )
      throw EXCEPTION("Non-consistent data : number of bands different for spin-up and spin-down",ERRABT);
  }


  //Impose right alignment with 5 digit precision as floating point numbers.
  str.setf(std::ios::right,std::ios::adjustfield);
  str.setf(std::ios::fixed,std::ios::floatfield);
  str.precision(8);
  //Write header

  str << "#";
  if ( options & PRTIKPT )
    str << std::setw(w) << "ikpt";
  if ( options & PRTKPT ) {
    str << std::setw(w) << "kx";
    str << std::setw(w) << "ky";
    str << std::setw(w) << "kz";
  }
  str << std::setw(w) << "length";
  for ( unsigned ispin = 0 ; ispin < nspin ; ++ispin ) {
    std::string prefix = ( _hasSpin ? ( ispin == 0 ? "band-up " : "band-down " ) : "band " );
    for ( unsigned i = 1; i <= _nband ; ++i ) 
      str << std::setw(w) << prefix+utils::to_string(i);
  }
  str << std::endl;

  for ( unsigned ikpt = 0 ; ikpt < nkpt ; ++ikpt ) {
    str << " ";
    if ( options & PRTIKPT )
      str << std::setw(w) << ikpt+1;
    if ( options & PRTKPT ) {
      str << std::setw(w) << _kpts[ikpt][0];
      str << std::setw(w) << _kpts[ikpt][1];
      str << std::setw(w) << _kpts[ikpt][2];
    }
    str << std::setw(w) << _lengths[ikpt];
    for ( unsigned ispin = 0 ; ispin < nspin ; ++ispin ) {
      for ( unsigned i = 0; i < _nband ; ++i ) 
        str << std::setw(w) << _eigens[ikpt+ispin*nkpt][i];
    }
    str << std::endl;
  }
  return str.str();
}


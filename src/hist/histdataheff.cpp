/**
 * @file src/histdataheff.cpp
 *
 * @brief Read a *.dps file created by HEFF code
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


#include "hist/histdataheff.hpp"
#include "base/exception.hpp"
#include "base/geometry.hpp"
#include "base/mendeleev.hpp"
#include <fstream>

using namespace Agate;

//
HistDataHeff::HistDataHeff() {
  ;
}

//
HistDataHeff::~HistDataHeff() {
  ;
}

//
void HistDataHeff::readFromFile(const std::string& filename) {
  std::ifstream file;

  file.open(filename.c_str(),std::ios::in|std::ios::binary);

  if ( !file )
    throw EXCEPTION(std::string("File ")+filename+" could not be opened",ERRABT);

  _xyz = 3;

  unsigned int marker;                  // Size of the next variable.
  int unit_natom;
  double *masses = nullptr;    // masses of each atom
  double *positions = nullptr; // Position in the unit cell of each atom
  double rprim[9] = {0.};      // Primite unit cell
  int supercell[3] = {1};      // Translations

  // Read natom
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));
  if ( marker != sizeof(int) )
    throw EXCEPTION("Bad header file: _natom",ERRDIV);
  file.read(reinterpret_cast<char*>(&unit_natom),marker);
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));

  masses = new double[unit_natom];
  positions = new double[3*unit_natom];

  // Read masses
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));
  if ( marker != (sizeof(double)*unit_natom) )
    throw EXCEPTION("Bad header file: masses",ERRDIV);
  file.read(reinterpret_cast<char*>(masses),marker);
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));

  // Read positions
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));
  if ( marker != (sizeof(double)*unit_natom*3) )
    throw EXCEPTION("Bad header file: positions",ERRDIV);
  file.read(reinterpret_cast<char*>(positions),marker);
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));

  // Read rprimd
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));
  if ( marker != (sizeof(double)*9) )
    throw EXCEPTION("Bad header file: rprim",ERRDIV);
  file.read(reinterpret_cast<char*>(rprim),marker);
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));

  // Read Supercell
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));
  if ( marker != (sizeof(int)*3) )
    throw EXCEPTION("Bad header file: translations",ERRDIV);
  file.read(reinterpret_cast<char*>(supercell),marker);
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));

  char *dummy = nullptr;
  // Read date ...
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));
  if ( marker != (sizeof(int)*6) )
    throw EXCEPTION("Bad header file: date",ERRDIV);
  dummy = new char[marker];
  file.read(reinterpret_cast<char*>(dummy),marker);
  file.read(reinterpret_cast<char*>(&marker),sizeof(int));
  delete[] dummy;

  // Everything is going well.
  // Now read for each line displacement and strain
  _natom = unit_natom*supercell[0]*supercell[1]*supercell[2];
  unsigned int linelength = sizeof(int) +
    _natom*3*sizeof(double) + 6*sizeof(double);
  _ntime = 0;
  dummy = new char[linelength];
  int begin = file.tellg(); 
  bool valid = false;
  do {
    file.read(reinterpret_cast<char*>(dummy),sizeof(int));
    valid = (reinterpret_cast<unsigned int*>(dummy)[0] == linelength) ;
    file.read(dummy,linelength);
    valid &= (file.gcount() == linelength);
    file.read(reinterpret_cast<char*>(dummy),sizeof(int));
    valid &= (reinterpret_cast<unsigned int*>(dummy)[0] == linelength);
    ++_ntime;
  } while(file && valid);
  if ( file.fail() || !valid ) --_ntime;
  file.clear();
  delete[] dummy;
  file.seekg(begin);

  int nsweep;
  double *displacements = new double[3*_natom];
  double strain[6];
  //linelength -= 2*sizeof(marker);

  // Lookup for typat and _znucl.
  std::vector<int> typat_unit;
  for( int iatom = 0 ; iatom < unit_natom ; ++iatom ){
    unsigned type = (unsigned) -1;
    for ( unsigned sp = 0 ; sp < _znucl.size() ; ++sp ) {
      const int zcomp = _znucl[sp];
      if ( fabs(masses[iatom]-MendeTable.mass[zcomp])/MendeTable.mass[zcomp] < 1.e-3 ) {
        type = sp+1;
        break;
      }
    }

    if ( type == (unsigned) -1 ) {
      type = _znucl.size()+1;
      //find new znucl
      try {
        _znucl.push_back(Mendeleev::znucl(masses[iatom]));
      }
      catch ( Exception &e ) {
        std::clog << e.fullWhat() << std::endl;
        _znucl.push_back(0);
      }
    }
    typat_unit.push_back(type);
  }

  // Allocate arrays
  _ntime++; // Add a dummy time with reference structure
  _xcart  .resize(_ntime*_natom*_xyz);
  _xred   .resize(_ntime*_natom*_xyz);
  //_fcart  .resize(_ntime*_natom*_xyz);
  _acell  .resize(_ntime*_xyz);
  _rprimd .resize(_ntime*_xyz*_xyz);
  _time   .resize(_ntime);
  _etotal .resize(_ntime);
  _stress .resize(_ntime*6);

  {
    // Don't change acell
    _acell[0*3  ] = 1.;
    _acell[0*3+1] = 1.;
    _acell[0*3+2] = 1.;
    // Set stress to zero even if this is not true, but the values are not in this file.
    for ( int vgt = 0 ; vgt < 6 ; ++vgt )
      _stress[0*6+vgt] = 0.;

    // Build rprimd.
    _rprimd[0*9  ] = rprim[0]*supercell[0];
    _rprimd[0*9+1] = rprim[1]*supercell[1];
    _rprimd[0*9+2] = rprim[2]*supercell[2];
    _rprimd[0*9+3] = rprim[3]*supercell[0];
    _rprimd[0*9+4] = rprim[4]*supercell[1];
    _rprimd[0*9+5] = rprim[5]*supercell[2];
    _rprimd[0*9+6] = rprim[6]*supercell[0];
    _rprimd[0*9+7] = rprim[7]*supercell[1];
    _rprimd[0*9+8] = rprim[8]*supercell[2];

    _etotal[0] = 0.;

    geometry::mat3d rprimtmp;
    rprimtmp[0] = _rprimd[0*9  ];
    rprimtmp[1] = _rprimd[0*9+1];
    rprimtmp[2] = _rprimd[0*9+2];
    rprimtmp[3] = _rprimd[0*9+3];
    rprimtmp[4] = _rprimd[0*9+4];
    rprimtmp[5] = _rprimd[0*9+5];
    rprimtmp[6] = _rprimd[0*9+6];
    rprimtmp[7] = _rprimd[0*9+7];
    rprimtmp[8] = _rprimd[0*9+8];
    geometry::mat3d gprim = geometry::invert(rprimtmp);

    _time[0] = 0.;
    // Generate position and displace them
    int gatom = 0;
    _typat.clear();
    for ( int i = 0 ; i < supercell[0] ; ++i ) {
      for (int j = 0 ; j < supercell[1] ; ++j ) {
        for (int k = 0; k < supercell[2]; ++k) {
          const double di = static_cast<double>(i);
          const double dj = static_cast<double>(j);
          const double dk = static_cast<double>(k);
          const double xtrans = di*rprim[0] + dj*rprim[1] + dk*rprim[2];
          const double ytrans = di*rprim[3] + dj*rprim[4] + dk*rprim[5];
          const double ztrans = di*rprim[6] + dj*rprim[7] + dk*rprim[8];
          for (int iatom = 0 ; iatom < unit_natom ; ++iatom,++gatom) {
            const double xc = positions[iatom*3  ]+xtrans;
            const double yc = positions[iatom*3+1]+ytrans;
            const double zc = positions[iatom*3+2]+ztrans;
            _xcart[0*3*_natom+gatom*3  ] = xc;
            _xcart[0*3*_natom+gatom*3+1] = yc;
            _xcart[0*3*_natom+gatom*3+2] = zc;
            _xred[0*3*_natom+gatom*3  ] = gprim[0]*xc + gprim[1]*yc + gprim[2]*zc;
            _xred[0*3*_natom+gatom*3+1] = gprim[3]*xc + gprim[4]*yc + gprim[5]*zc;
            _xred[0*3*_natom+gatom*3+2] = gprim[6]*xc + gprim[7]*yc + gprim[8]*zc;
            _typat.push_back(typat_unit[iatom]);
          }
        }
      }
    }
  }

  for ( unsigned itime = 1 ; itime < _ntime ; ++ itime ) {
    // Don't change acell
    _acell[itime*3  ] = 1.;
    _acell[itime*3+1] = 1.;
    _acell[itime*3+2] = 1.;
    // Set stress to zero even if this is not true, but the values are not in this file.
    for ( int vgt = 0 ; vgt < 6 ; ++vgt )
      _stress[itime*6+vgt] = 0.;

    // Build rprimd.
    _rprimd[itime*9  ] = rprim[0]*supercell[0];
    _rprimd[itime*9+1] = rprim[1]*supercell[1];
    _rprimd[itime*9+2] = rprim[2]*supercell[2];
    _rprimd[itime*9+3] = rprim[3]*supercell[0];
    _rprimd[itime*9+4] = rprim[4]*supercell[1];
    _rprimd[itime*9+5] = rprim[5]*supercell[2];
    _rprimd[itime*9+6] = rprim[6]*supercell[0];
    _rprimd[itime*9+7] = rprim[7]*supercell[1];
    _rprimd[itime*9+8] = rprim[8]*supercell[2];

    _etotal[itime] = 0.;

    file.read(reinterpret_cast<char*>(&marker),sizeof(int));
    if ( marker != linelength )
      throw EXCEPTION("Bad beginning line for itime "+utils::to_string(itime),ERRDIV);
    file.read(reinterpret_cast<char*>(&nsweep),sizeof(int));
    file.read(reinterpret_cast<char*>(displacements),sizeof(double)*3*_natom);
    file.read(reinterpret_cast<char*>(strain),sizeof(double)*6);
    file.read(reinterpret_cast<char*>(&marker),sizeof(int));
    if ( marker != linelength )
      throw EXCEPTION("Bad ending line for itime "+utils::to_string(itime),ERRDIV);

    geometry::mat3d rprimtmp;
    rprimtmp[0] = (_rprimd[itime*9  ]+=(strain[0]*_rprimd[itime*9  ] + strain[5]*_rprimd[itime*9+3] + strain[4]*_rprimd[itime*9+6]));
    rprimtmp[1] = (_rprimd[itime*9+1]+=(strain[0]*_rprimd[itime*9+1] + strain[5]*_rprimd[itime*9+4] + strain[4]*_rprimd[itime*9+7]));
    rprimtmp[2] = (_rprimd[itime*9+2]+=(strain[0]*_rprimd[itime*9+2] + strain[5]*_rprimd[itime*9+5] + strain[4]*_rprimd[itime*9+8]));
    rprimtmp[3] = (_rprimd[itime*9+3]+=(strain[1]*_rprimd[itime*9+3] + strain[5]*_rprimd[itime*9  ] + strain[3]*_rprimd[itime*9+6]));
    rprimtmp[4] = (_rprimd[itime*9+4]+=(strain[1]*_rprimd[itime*9+4] + strain[5]*_rprimd[itime*9+1] + strain[3]*_rprimd[itime*9+7]));
    rprimtmp[5] = (_rprimd[itime*9+5]+=(strain[1]*_rprimd[itime*9+5] + strain[5]*_rprimd[itime*9+2] + strain[3]*_rprimd[itime*9+8]));
    rprimtmp[6] = (_rprimd[itime*9+6]+=(strain[2]*_rprimd[itime*9+6] + strain[4]*_rprimd[itime*9  ] + strain[3]*_rprimd[itime*9+3]));
    rprimtmp[7] = (_rprimd[itime*9+7]+=(strain[2]*_rprimd[itime*9+7] + strain[4]*_rprimd[itime*9+1] + strain[3]*_rprimd[itime*9+4]));
    rprimtmp[8] = (_rprimd[itime*9+8]+=(strain[2]*_rprimd[itime*9+8] + strain[4]*_rprimd[itime*9+2] + strain[3]*_rprimd[itime*9+5]));
    geometry::mat3d gprim = geometry::invert(rprimtmp);

    _time[itime] = static_cast<double>(nsweep);
    // Generate position and displace them
    int gatom = 0;
    //_typat.clear();
    for ( int i = 0 ; i < supercell[0] ; ++i ) {
      for (int j = 0 ; j < supercell[1] ; ++j ) {
        for (int k = 0; k < supercell[2]; ++k) {
          const double di = static_cast<double>(i);
          const double dj = static_cast<double>(j);
          const double dk = static_cast<double>(k);
          const double xtrans = di*rprim[0] + dj*rprim[1] + dk*rprim[2];
          const double ytrans = di*rprim[3] + dj*rprim[4] + dk*rprim[5];
          const double ztrans = di*rprim[6] + dj*rprim[7] + dk*rprim[8];
          for (int iatom = 0 ; iatom < unit_natom ; ++iatom,++gatom) {
            const double xc = positions[iatom*3  ]+xtrans+displacements[gatom*3  ];
            const double yc = positions[iatom*3+1]+ytrans+displacements[gatom*3+1];
            const double zc = positions[iatom*3+2]+ztrans+displacements[gatom*3+2];
            const double x = xc + (strain[0]*xc + strain[5]*yc + strain[4]*zc);
            const double y = yc + (strain[5]*xc + strain[1]*yc + strain[3]*zc);
            const double z = zc + (strain[4]*xc + strain[3]*yc + strain[2]*zc);
            _xcart[itime*3*_natom+gatom*3  ] = x;
            _xcart[itime*3*_natom+gatom*3+1] = y;
            _xcart[itime*3*_natom+gatom*3+2] = z;
            _xred[itime*3*_natom+gatom*3  ] = gprim[0]*x + gprim[1]*y + gprim[2]*z;
            _xred[itime*3*_natom+gatom*3+1] = gprim[3]*x + gprim[4]*y + gprim[5]*z;
            _xred[itime*3*_natom+gatom*3+2] = gprim[6]*x + gprim[7]*y + gprim[8]*z;
            //_fcart[itime*3*_natom+gatom*3  ] = 0.;
            //_fcart[itime*3*_natom+gatom*3+1] = 0.;
            //_fcart[itime*3*_natom+gatom*3+2] = 0.;
            //_typat.push_back(typat_unit[iatom]);
          }
        }
      }
    }
  }
  delete[] displacements;
  delete[] positions;
  delete[] masses;

  std::clog << "One time step is equivalent to " << nsweep/_ntime << " MC steps." << std::endl;

  file.close();
  _filename = filename;
  _ntimeAvail = _ntime;
}

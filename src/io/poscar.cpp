/**
 * @file poscar.cpp
 *
 * @brief Implementation of the poscar class
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


#include "io/poscar.hpp"
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <locale>
#include <cmath>
#include "base/phys.hpp"
#include "base/mendeleev.hpp"
#include "base/utils.hpp"

using Agate::Mendeleev;

//
Poscar::Poscar() : Dtset(),
  _title(),
  _names()
{
}

Poscar::Poscar(const HistData &hist, const unsigned itime) : Dtset(hist,itime),
  _title("_HIST2Poscar time "+utils::to_string(itime)),
  _names()
{
  if ( _znucl.size() >= 1 && _znucl[0]!=0 ) {
    _names = "    ";
    for ( auto& znucl : _znucl ) {
      _names += Mendeleev::name[znucl];
      _names += " ";
    }
    _names.erase(_names.end()-1);
  }
  else {
    _names = "    !!! UNDEFINED !!!";
  }
}


//
Poscar::Poscar(const Dtset& dtset) : Dtset(dtset),
  _title("Abinit2VASP"),
  _names()
{
  /*
  _natom = dtset.natom();
  _ntypat = dtset.ntypat();
  _typat = dtset.typat();
  _znucl = dtset.znucl();
  //_acell = dtset.acell();
  _rprim = dtset.rprim();
  _gprim = dtset.gprim();
  _xcart = dtset.xcart();
  _xred  = dtset.xred();
  _findsym = dtset.findsym();
  */
  _acell = {{ 1.0, 1.0, 1.0 }};
  if ( !_znucl.empty() ) {
    _znucl.resize(_ntypat);
    _names = "    ";
    for ( auto& znucl : _znucl ) {
      _names += Mendeleev::name[znucl];
      _names += " ";
    }
    _names.erase(_names.end()-1);
  }
}

//
Poscar& Poscar::operator = (const Dtset& dtset){
  _title = "Abinit2VASP";
  _natom = dtset.natom();
  _ntypat = dtset.ntypat();
  _typat = dtset.typat();
  _znucl = dtset.znucl();
  //_acell = dtset.acell();
  _acell = {{ 1.0, 1.0, 1.0 }};
  _rprim = dtset.rprim();
  _gprim = dtset.gprim();
  _xcart = dtset.xcart();
  _xred  = dtset.xred();
  _findsym = dtset.findsym();
  if ( !_znucl.empty() ) {
    _znucl.resize(_ntypat);
    _names = "    ";
    for ( auto& znucl : _znucl ) {
      _names += Mendeleev::name[znucl];
      _names += " ";
    }
    _names.erase(_names.end()-1);
  }
  return *this;
}

//
Poscar::~Poscar(){
}


//
void Poscar::readFromFile(const std::string& filename) {
  std::ifstream poscar(filename,std::ios::in);  
  try {
    if ( !poscar )
      throw EXCEPTION("Error while opening file " + filename, ERRDIV);

    std::string line;
    std::stringstream sstr;

    std::getline(poscar,_title);
    utils::trim(line);

    //Try to read the type of atoms in the title
    ///*
    {
      sstr.clear();
      sstr.str(_title);
      _znucl.clear();
      // Try to identify symbols 
      do {
        std::string tmp_str;
        sstr >> tmp_str;
        if ( tmp_str.empty() ) continue;
        try {
          unsigned sp = Mendeleev::znucl(tmp_str);
          _znucl.push_back(sp);
        }
        catch ( Exception& e ) {
          _names = line;
          _znucl.clear();
          break;
        }
      } while ( !sstr.eof() );
      sstr.clear();
    }
    //*/

    std::getline(poscar,line);
    utils::trim(line);
    sstr.str(line) ;
    sstr >> _acell[0];
    if ( sstr.fail() ) throw EXCEPTION("Failed to read lattice parameter (2nd line)",ERRDIV);
    //_acell[0] /= phys::b2A; // Universal factor, no unit
    sstr.clear();

    for ( unsigned i = 1 ; i < 4 ; ++i ) {
      std::getline(poscar,line);
      utils::trim(line);
      sstr.str(line) ;
      for ( unsigned j = 1 ; j < 4 ; ++j ) {
        sstr >> _rprim[geometry::mat3dind(i,j)];
        //_rprim[geometry::mat3dind(i,j)] *= _acell[i-1]; //Don't do that here since acell might be <0
        //_rprim[geometry::mat3dind(i,j)] /= phys::b2A; // Assume lattice vector in angstrom
        if ( sstr.fail() )
          throw EXCEPTION("Failed to read vector " + utils::to_string(i) 
              + " coordinate " + utils::to_string(j), ERRDIV);
      }
      sstr.clear();
    }
    if ( _acell[0] < 0.0 ) // We need to set the correct volume
      _acell[0] = cbrt(-_acell[0]/geometry::det(_rprim));
    for ( unsigned i = 0 ; i < 9 ; ++i )
        _rprim[i] *= _acell[0]/phys::b2A;

    unsigned itype = 0;
    unsigned ntype = 0;
    std::getline(poscar,line);
    utils::trim(line);
    /* Check if line is the type or something else */
    sstr.str(line);
    sstr >> ntype;
    if ( sstr.fail() ) {
        std::vector<int> znucl_tmp;
        sstr.clear();
        sstr.str(line);
        /* Try to identify symbols */
        do {
          std::string tmp_str;
          sstr >> tmp_str;
          if ( tmp_str.empty() ) continue;
          try {
            unsigned sp = Mendeleev::znucl(tmp_str);
            znucl_tmp.push_back(sp);
          }
          catch ( Exception& e ) {
            _names = line;
            znucl_tmp.clear();
            break;
          }
        } while ( !sstr.eof() );
        if ( !znucl_tmp.empty() ) _znucl = std::move(znucl_tmp);

      std::getline(poscar,line);
      utils::trim(line);
    }
    sstr.clear();

    utils::trim(line);
    sstr.str(line) ;
    do {
      sstr >> ntype;
      if ( sstr.fail() ) 
        throw EXCEPTION("Failed to read types", ERRDIV) ;
      _typat.insert(_typat.end(),ntype,++itype);
      _natom += ntype;
      ++_ntypat;
    } while ( !sstr.eof() );
    if ( !_znucl.empty() ) {
      _znucl.resize(_ntypat);
      _names = "    ";
      for ( auto& znucl : _znucl ) {
        _names += Mendeleev::name[znucl];
        _names += " ";
      }
    }

    std::getline(poscar,line);
    sstr.clear();
    utils::trim(line);
    sstr.str(line) ;
    std::locale loc;
    if ( std::toupper(sstr.str().at(0),loc) == 'D' ) { //Read reduce coordinates
      _xred.resize(_natom);
      sstr.clear();
      for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
        sstr.clear();
        std::getline(poscar,line);
        utils::trim(line);
        sstr.str(line) ;
        double x, y, z;
        sstr >> x ;
        if ( sstr.fail() ) 
          throw EXCEPTION("Failed to read reduced coordinate x for atom " + utils::to_string(iatom), ERRDIV) ;
        sstr >> y;
        if ( sstr.fail() ) 
          throw EXCEPTION("Failed to read reduced coordinate y for atom " + utils::to_string(iatom), ERRDIV) ;
        sstr >> z;
        if ( sstr.fail() ) 
          throw EXCEPTION("Failed to read reduced coordinate z for atom " + utils::to_string(iatom), ERRDIV) ;
        _xred[iatom] = {{x,y,z}};
      }
#ifdef HAVE_SHRINK_TO_FIT
      _xred.shrink_to_fit();
#endif
      _gprim = geometry::invertTranspose(_rprim);
      geometry::changeBasis(_rprim, _xcart, _xred, false);
    }
    else if ( std::toupper(sstr.str().at(0),loc) == 'C' || std::toupper(sstr.str().at(0),loc) == 'K' ) {
      _xcart.resize(_natom);
      sstr.clear();
      for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
        sstr.clear();
        std::getline(poscar,line);
        utils::trim(line);
        sstr.str(line) ;
        double x, y, z;
        sstr >> x ;
        if ( sstr.fail() ) 
          throw EXCEPTION("Failed to read cartesian coordinate x for atom " + utils::to_string(iatom), ERRDIV) ;
        sstr >> y;
        if ( sstr.fail() ) 
          throw EXCEPTION("Failed to read cartesian coordinate y for atom " + utils::to_string(iatom), ERRDIV) ;
        sstr >> z;
        if ( sstr.fail() ) 
          throw EXCEPTION("Failed to read cartesian coordinate z for atom " + utils::to_string(iatom), ERRDIV) ;
        _xcart[iatom] = {{x*_acell[0]/phys::b2A,y*_acell[0]/phys::b2A,z*_acell[0]/phys::b2A}};
      }
#ifdef HAVE_SHRINK_TO_FIT
      _xcart.shrink_to_fit();
#endif
      geometry::changeBasis(_rprim, _xcart, _xred, true);
      _acell[0] = 1.0;
      _acell[1] = 1.0;
      _acell[2] = 1.0;
    }
    else {
      throw EXCEPTION("Unkown coordinate system "+sstr.str(),ERRDIV);
    }
  }
  catch( Exception& e ) {
    std::string err_str = "Failed to build Poscar from file "+filename;
    e.ADD(err_str,ERRDIV);
    throw e;
  }

}


//
void Poscar::dump(std::ostream& out) const {

  // One should reorder the atoms to be sure they are ordered by type.
  std::vector<unsigned> order(_natom);
  {
    unsigned ind = 0;
    for ( int itype = 1 ; itype <= (int)_ntypat ; ++itype ) {
      for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
        if ( _typat[iatom] == itype ) order[ind++]=iatom;
      }
    }
  }

  try { 
    // Title
    if ( _title.empty() ) {
      out << PACKAGE_NAME << " version " << PACKAGE_VERSION << std::endl;
    }
    else
      out << _title << std::endl;

    out.setf(std::ios::right,std::ios::adjustfield);
    out.setf(std::ios::scientific,std::ios::floatfield);
    out.precision(14);

    // Lattice parameter
    //out << std::setw(25) << -geometry::det(_rprim)*(phys::b2A*phys::b2A*phys::b2A) << std::endl;
    out << std::setw(25) << 1 << std::endl;
    //out << std::setw(25) << _acell[0] << std::endl;

    // Vectors
    out << std::setw(27) << _rprim[geometry::mat3dind(1,1)]*phys::b2A << std::setw(23) << _rprim[geometry::mat3dind(1,2)]*phys::b2A << std::setw(23) << _rprim[geometry::mat3dind(1,3)]*phys::b2A << std::endl;
    out << std::setw(27) << _rprim[geometry::mat3dind(2,1)]*phys::b2A << std::setw(23) << _rprim[geometry::mat3dind(2,2)]*phys::b2A << std::setw(23) << _rprim[geometry::mat3dind(2,3)]*phys::b2A << std::endl;
    out << std::setw(27) << _rprim[geometry::mat3dind(3,1)]*phys::b2A << std::setw(23) << _rprim[geometry::mat3dind(3,2)]*phys::b2A << std::setw(23) << _rprim[geometry::mat3dind(3,3)]*phys::b2A << std::endl;

    // Species if present
    if ( !_names.empty() ) out << _names << std::endl;

    // type of each name
    out.setf(std::ios::left,std::ios::adjustfield);
    out << std::setw(6) << " ";
    for ( int ntypat = 1 ; ntypat <= (int)_ntypat ; ++ntypat ) {
      unsigned total = 0;
      for ( auto type : _typat) 
        if ( type == ntypat ) ++total;

      out << std::setw(6) << total;
    }
    out << std::endl;

    // Direct coordinates
    out << "Direct" << std::endl;
    out.setf(std::ios::right,std::ios::adjustfield);
    for ( auto& icoord : order )
      out << std::setw(25) << _xred[icoord][0] << std::setw(23) << _xred[icoord][1] << std::setw(23) << _xred[icoord][2] << std::endl;
  }
  catch (...) {
    throw EXCEPTION("Something went wrong during the dumping of Poscar", ERRDIV);
  }

}


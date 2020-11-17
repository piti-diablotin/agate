/**
 * @file src/ddb.cpp
 *
 * @brief 
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


#include "io/ddb.hpp"
#include "base/exception.hpp"
#include <string>
#include "base/utils.hpp"
#include <sstream>
#include <iomanip>
#include "io/configparser.hpp"
#include "io/ddbabinit.hpp"
#include "io/ddbphonopy.hpp"
#include <fstream>
#include "base/uriparser.hpp"
#include "base/fraction.hpp"

//
Ddb::Ddb() : Dtset(),
  _haveMasses(false),
  _nqpt(0),
  _blocks(),
  _zion()
{
  ;
}

//
Ddb::~Ddb() {
  ;
}


//
std::string Ddb::info() const {
  std::ostringstream rstr;
  Dtset::dump(rstr);
  rstr << " ** DDB Information ** \n-> "
    << utils::to_string(_blocks.size())
    << " qpt found.\n";
  rstr.precision(14);
  rstr.setf(std::ios::scientific, std::ios::floatfield);
  rstr.setf(std::ios::right, std::ios::adjustfield);
  for ( auto& block : _blocks ) {
    rstr << std::endl << "Q-pt: " << std::setw(8) << Fraction(block.first[0])
      << std::setw(8) << Fraction(block.first[1])
      << std::setw(8) << Fraction(block.first[2]) << std::endl;
    rstr << "  # elements: " << block.second.size() << std::endl;
  }
  return rstr.str();
}


//
const std::vector<Ddb::d2der>& Ddb::getDdb(const geometry::vec3d qpt) const {
  using namespace geometry;
  auto found = _blocks.end();
  for ( auto it = _blocks.begin() ; it != _blocks.end() ; ++ it) {
    if ( norm(qpt-it->first) < 1e-12 )
      found = it;
  }
  if ( found == _blocks.end() )
    throw EXCEPTION(std::string("Block not found for q-pt [")
        +utils::to_string(qpt[0])+std::string(",")
        +utils::to_string(qpt[1])+std::string(",")
        +utils::to_string(qpt[2])+std::string("]"),Ddb::ERFOUND);

  return found->second;
}


//
const std::vector<geometry::vec3d> Ddb::getQpts() const {
  std::vector<geometry::vec3d> qpts;
  for ( auto it = _blocks.begin() ; it != _blocks.end() ; ++it ) {
    qpts.push_back(it->first);
  }
  return qpts;
}

//
Ddb* Ddb::getDdb(const std::string& infile){
  Ddb *ddb = nullptr;
  Exception eloc;
  std::vector<std::pair<std::unique_ptr<Ddb>,std::string> > allFormat;

  allFormat.push_back(std::make_pair(std::unique_ptr<Ddb>(new DdbAbinit),"Abinit DDB")); //0
  allFormat.push_back(std::make_pair(std::unique_ptr<Ddb>(new DdbPhonopy),"Phonopy YAML"));   //1

  std::string file = infile;
  UriParser uri;
  if ( uri.parse(infile) ) {
    file = uri.getFile();
  }

  if ( file.find(".yaml") != std::string::npos ) allFormat[0].swap(allFormat[1]);

  for ( auto& p : allFormat ) {
    try {
      p.first->readFromFile(file);
      ddb = p.first.release();
    }
    catch (Exception &e) {
      ddb = nullptr;
      eloc += e;
      if ( e.getReturnValue() == ERRABT ) {
        break;
      }
      eloc.ADD("Format is not "+p.second,ERRDIV);
    }
    if ( ddb != nullptr ) {
      std::clog << "Format is "+p.second << std::endl;
      return ddb;
    }
  }

  eloc.ADD("Failed to build the DDB",ERRDIV);
  throw eloc;
  return nullptr;
}

void Ddb::dump(const geometry::vec3d qpt, std::string filename) {
  auto& d2 = this->getDdb(qpt);
  std::ofstream out;

  if ( filename == "" ) 
    filename = std::string("dynmat-")+geometry::to_string(qpt,false)+".out";

  out.open(filename);
  
  if ( !out ) throw EXCEPTION(std::string("Unable to create file ")+filename,ERRDIV);

  for ( unsigned iatom1 = 0 ; iatom1 < _natom ; ++iatom1 ) {
    for ( unsigned idir1 = 0 ; idir1 < 3 ; ++idir1 ) {
      for ( unsigned iatom2 = 0 ; iatom2 < _natom ; ++iatom2 ) {
        for ( unsigned idir2 = 0 ; idir2 < 3 ; ++idir2 ) {
          for ( auto &elt : d2 ) {
            auto &coord = elt.first;
            if ( 
                coord[0] == idir1 &&
                coord[1] == iatom1 &&
                coord[2] == idir2 &&
                coord[3] == iatom2
               )
              out << std::setw(12) << elt.second.real() << " " ;
          }
        }
      }
      out << std::endl;
    }
  }
  out.close();
  throw EXCEPTION(std::string("Dynamical matrix written to ")+filename, ERRCOM);
}

geometry::mat3d Ddb::getZeff(const unsigned iatom) const {
  using namespace geometry;
  const vec3d qpt = {{0,0,0}};
  
  if ( iatom >= _natom ) 
    throw EXCEPTION("Atom "+utils::to_string(iatom)+" is not in DDB", ERRDIV);

  auto data = this->getDdb(qpt);
  mat3d zeff;
  mat3d count;
  for ( auto &e : count ) e = 0e0;
  for ( auto &e : zeff ) e = 0e0;
  const double twopi = 2*phys::pi;
	
	/* Read values from ddb into _zeff*/
	for ( auto& elt : data ) {
    const unsigned idir1 = elt.first[0];
    const unsigned ipert1 = elt.first[1];
    const unsigned idir2 = elt.first[2];
    const unsigned ipert2 = elt.first[3];
    if ( idir1 < 3 && idir2 < 3 && 
        ( ( ipert1 == _natom+1 && ipert2 == iatom ) 
          || ( ipert2 == _natom+1 && ipert1 == iatom ) 
        )
       ) {
      // Store Efield along column and disp alon lines
      if ( ipert1 == _natom+1 ) { 
        zeff[mat3dind( idir1+1, idir2+1)] += elt.second.real();
        count[mat3dind( idir1+1, idir2+1)] += 1e0;
      }
      else {
        zeff[mat3dind( idir2+1, idir1+1)] += elt.second.real();
        count[mat3dind( idir2+1, idir1+1)] += 1e0;
      }
    }
	}  	

  mat3d rprimTranspose = transpose(_rprim);
  for ( unsigned i = 1 ; i < 4 ; ++i )
    for ( unsigned j = 1 ; j < 4 ; ++j ) {
      if ( count[mat3dind(i,j)] == 0 )
        throw EXCEPTION("Derivative not found for element "
            +utils::to_string(i)+","+utils::to_string(j),ERRDIV);
      zeff[mat3dind(j,i)] /= (twopi*count[mat3dind(j,i)]);
    }
  
  zeff = _gprim * (zeff * rprimTranspose);

	for ( unsigned idir = 1 ; idir <= 3 ; ++idir ) 
       zeff[mat3dind( idir, idir)] += _zion[_typat[iatom]-1];		

	return zeff;			
}

geometry::mat3d Ddb::getEpsInf() const {
  using namespace geometry;
  const vec3d qpt = {{0,0,0}};
  
  auto data = this->getDdb(qpt);
  mat3d epsinf;
  mat3d count;
  for ( auto &e : count ) e = 0e0;
  for ( auto &e : epsinf ) e = 0e0;
	
	/* Read values from ddb into epsilon infiny*/
	for ( auto& elt : data ) {
    const unsigned idir1 = elt.first[0];
    const unsigned ipert1 = elt.first[1];
    const unsigned idir2 = elt.first[2];
    const unsigned ipert2 = elt.first[3];
    if ( idir1 < 3 && idir2 < 3 && 
        ( ( ipert1 == _natom+1 && ipert2 == _natom+1 ) 
          || ( ipert2 == _natom+1 && ipert1 == _natom+1 ) 
        )
       ) {
      epsinf[mat3dind( idir1+1, idir2+1)] += elt.second.real();
      count[mat3dind( idir1+1, idir2+1)] += 1e0;
    }
	}  	

  mat3d rprimTranspose = transpose(_rprim);
  double volume = det(_rprim);
  for ( unsigned i = 1 ; i < 4 ; ++i )
    for ( unsigned j = 1 ; j < 4 ; ++j ) {
      if ( count[mat3dind(j,i)] == 0 )
        throw EXCEPTION("Derivative not found for element "
            +utils::to_string(i)+","+utils::to_string(j),ERRDIV);
      epsinf[mat3dind(j,i)] /= (-phys::pi*volume*count[mat3dind(j,i)]);
    }
  
  epsinf = _rprim * (epsinf * rprimTranspose);

	for ( unsigned idir = 1 ; idir <= 3 ; ++idir ) 
       epsinf[mat3dind( idir, idir)] += 1.0;

	return epsinf;			
}

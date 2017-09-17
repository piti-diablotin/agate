/**
 * @file src/ddb.cpp
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


#include "io/ddb.hpp"
#include "base/exception.hpp"
#include <string>
#include "base/utils.hpp"
#include <sstream>
#include <iomanip>
#include "io/configparser.hpp"
#include "io/ddbabinit.hpp"
#include "io/ddbphonopy.hpp"

//
Ddb::Ddb() : Dtset(),
  _haveMasses(false),
  _nqpt(0),
  _blocks()
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
  this->dump(rstr);
  rstr << " ** DDB Information ** \n-> "
    << utils::to_string(_blocks.size())
    << " qpt found.\n";
  rstr.precision(14);
  rstr.setf(std::ios::scientific, std::ios::floatfield);
  rstr.setf(std::ios::right, std::ios::adjustfield);
  for ( auto& block : _blocks ) {
    rstr << std::endl << "Q-pt: " << std::setw(24) << block.first[0]
      << std::setw(24) << block.first[1]
      << std::setw(24) << block.first[2] << std::endl;
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
Ddb* Ddb::getDdb(const std::string& file){
  Ddb *ddb = nullptr;
  Exception eloc;
  std::vector<std::pair<std::unique_ptr<Ddb>,std::string> > allFormat;

  allFormat.push_back(std::make_pair(std::unique_ptr<Ddb>(new DdbAbinit),"Abinit DDB")); //0
  allFormat.push_back(std::make_pair(std::unique_ptr<Ddb>(new DdbPhonopy),"Phonopy YAML"));   //1

  if ( file.find(".yaml") != std::string::npos ) allFormat[0].swap(allFormat[1]);

  for ( auto& p : allFormat ) {
    try {
      p.first->readFromFile(file);
      ddb = p.first.release();
    }
    catch (Exception &e) {
      ddb = nullptr;
      eloc += e;
      eloc.ADD("Format is not "+p.second,ERRDIV);
      if ( e.getReturnValue() == ERRABT ) {
        break;
      }
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

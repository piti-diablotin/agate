/**
 * @file src/./histdatapolydtset.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2018 Jordan Bieder
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


#include "hist/histdatapolydtset.hpp"
#include "io/poscar.hpp"
#include "io/etsfnc.hpp"
#include "io/abihdr.hpp"
#include "io/phonopydtset.hpp"
#include <memory>

//
HistDataPolyDtset::HistDataPolyDtset() {
  ;
}

//
HistDataPolyDtset::~HistDataPolyDtset() {
  ;
}

//
void HistDataPolyDtset::readFromFile(const std::string& filename) {
  Dtset *dtset;
  Exception eloc;
  std::vector<std::pair<std::unique_ptr<Dtset>,std::string> > allFormat;

  allFormat.push_back(std::make_pair(std::unique_ptr<Dtset>(new Poscar),"POSCAR")); //0
  allFormat.push_back(std::make_pair(std::unique_ptr<Dtset>(new AbiHdr),"Abinit Binary Header"));   //1
  allFormat.push_back(std::make_pair(std::unique_ptr<Dtset>(new EtsfNC),"ETSF"));   //2
  allFormat.push_back(std::make_pair(std::unique_ptr<Dtset>(new PhonopyDtset),"Phonopy"));     //3

  if      ( filename.find("POSCAR") != std::string::npos ) allFormat[0].swap(allFormat[0]);
  else if ( filename.find("_DEN") != std::string::npos ) allFormat[0].swap(allFormat[1]);
  else if ( filename.find("_OPT") != std::string::npos ) allFormat[0].swap(allFormat[1]);
  else if ( filename.find("_POT") != std::string::npos ) allFormat[0].swap(allFormat[1]);
  else if ( filename.find(".nc") != std::string::npos ) allFormat[0].swap(allFormat[2]);
  else if ( filename.find(".yaml") != std::string::npos ) allFormat[0].swap(allFormat[3]);

  for ( auto& p : allFormat ) {
    try {
      p.first->readFromFile(filename);
      dtset = p.first.release();
    }
    catch (Exception &e) {
      dtset = nullptr;
      eloc += e;
      if ( e.getReturnValue() == ERRABT ) {
        break;
      }
      eloc.ADD("Format is not "+p.second,e.getReturnValue());
    }
    if ( dtset != nullptr ) {
      std::clog << "Format is "+p.second << std::endl;
      this->buildFromDtset(*dtset);
      _filename = filename;
      delete dtset;
      return;
    }
  }
  throw eloc;
}

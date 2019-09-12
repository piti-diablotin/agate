/**
 * @file src/./dosdb.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2019 Jordan Bieder
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


#include "plot/dosdb.hpp"
#include "base/utils.hpp"
#include "base/exception.hpp"
#include <iostream>

//
DosDB::DosDB() :
  _db()
{
  ;
}

//
DosDB::~DosDB() {
  ;
}

void DosDB::buildFromPrefix(std::string prefix)
{
  std::vector<std::pair<long int, std::string>> files;
  std::string dir;
  try {
    auto pos = prefix.find_last_of("/\\");
    dir = prefix.substr(0,pos);
    pos = ( pos == std::string::npos ? 0 : pos+1);
    files = std::move(utils::ls(dir,prefix.substr(pos)+"_DOS.*"));
  }
  catch (...) {
    throw EXCEPTION("Problem getting file names",ERRDIV);
  }
  for ( auto f : files ) {
    try {
      ElectronDos edos;
      edos.readFromFile(dir+"/"+f.second);
      _db.push_back(edos);
    }
    catch(Exception &e) {
      e.ADD("Ignoring file "+f.second,ERRWAR);
      std::clog << e.fullWhat() << std::endl;
    }
  }
}


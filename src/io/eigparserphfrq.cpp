/**
 * @file src/eigparserphfrq.cpp
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


#include "io/eigparserphfrq.hpp"
#include "base/exception.hpp"
#include <fstream>
#include <sstream>

//
EigParserPHFRQ::EigParserPHFRQ() : EigParserPhonons() {
  ;
}

//
EigParserPHFRQ::~EigParserPHFRQ() {
  ;
}

//
void EigParserPHFRQ::readFromFile(const std::string& filename) {

  unsigned nband = -1;
  std::string tmp;
  const geometry::vec3d kpt({{0.0,0.0,0.0}});
  double value=0.0;

  std::ifstream phfrq(filename.c_str(), std::ios::in);
  if ( !phfrq ) {
    throw EXCEPTION(std::string("Unable to read the file ")+filename,ERRABT);
  }
  for ( std::string line ; std::getline(phfrq,line) ; ) {
    size_t pos_com = line.find_first_of("#");
    if ( pos_com != std::string::npos ) {
      line.resize(pos_com);
    }
    if ( line.size() == 0 ) continue;
    std::istringstream sline(line);
    double length;
    sline >> length;
    if ( sline.fail() )
      throw EXCEPTION("Unable to read length",ERRABT);
    _lengths.push_back(length);
    std::vector<double> values;
    while ( !sline.eof() ) {
      sline >> value;
      values.push_back(value);
    }
    if ( nband == (unsigned)-1 ) nband = values.size();
    if ( nband != values.size() )
      throw EXCEPTION("Bad number of band",ERRABT);
    _eigens.push_back(std::move(values));
    _kpts.push_back(kpt);
  }
  phfrq.close();
  _filename = filename;
  _nband = nband;
  _eunit = Units::Ha;
  _hasSpin = false;
}

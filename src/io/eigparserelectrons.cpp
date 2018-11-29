/**
 * @file src/./eigparserelectrons.cpp
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


#include "io/eigparserelectrons.hpp"

//
EigParserElectrons::EigParserElectrons() :EigParser() {
  ;
}

//
EigParserElectrons::~EigParserElectrons() {
  ;
}

std::vector<std::vector<double>> EigParserElectrons::getBandProjection(const unsigned iband, const unsigned ispin) const {
  ;
}

std::vector<unsigned> EigParserElectrons::getBandColor(const unsigned iband, const unsigned ispin, const std::vector<unsigned> umask) const { 
  ;
}

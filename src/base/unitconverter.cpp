/**
 * @file src/./energyunit.cpp
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


#include "base/unitconverter.hpp"
#include "base/phys.hpp"
#include "base/utils.hpp"
#include "base/exception.hpp"

const UnitConverter::UnitDefinition UnitConverter::dataBase[UnitConverter::_nunit] = {
  {eV , Energy, "eV"   ,1.},
  {Ha , Energy, "Ha"   ,phys::Ha2eV},
  {THz, Energy, "THz"  ,1e12*phys::Hz2eV},
  {pcm, Energy, "cm-1" ,1e2*phys::m2eV},
  {K, Energy, "K" ,phys::kB/phys::eV},
  {au, Conductivity, "au" ,1.},
  {pOhmpcm, Conductivity, "Ohm-1.cm-1" ,1./phys::Ohmcm},
  {amu, Mass, "amu" ,1.},
  {emu, Mass, "emu" ,phys::emass/phys::amu},
  {kg, Mass, "kg" ,1./phys::amu},
  {A, Length, "A" ,phys::A2b},
  {bohr, Length, "bohr" ,1.},
};

//
UnitConverter::UnitConverter() :
  _from(Ha),
  _fromIndex(getIndex(Ha)),
  _to(Ha),
  _toIndex(getIndex(Ha))
{
  ;
}

//
UnitConverter::UnitConverter(Unit u) :
  _from(u),
  _fromIndex(getIndex(u)),
  _to(u),
  _toIndex(getIndex(u))
{
  ;
}

//
UnitConverter::~UnitConverter() {
  ;
}

std::istream& operator>>(std::istream &in, UnitConverter &eunit) {
  std::string tmp;
  in >> tmp;
  eunit = UnitConverter::getFromString(tmp);
  return in;
}

std::ostream& operator<<(std::ostream &out, UnitConverter &eunit) {
  out << UnitConverter::dataBase[eunit._toIndex]._symbol;
  return out;
}

std::string operator+(std::string &str, UnitConverter &eunit) {
  return str+UnitConverter::dataBase[eunit._toIndex]._symbol;
}

UnitConverter& UnitConverter::operator=(Unit u) {
  int newIndex = getIndex(u);
  if ( dataBase[newIndex]._type == dataBase[_fromIndex]._type ) {
    _to = u;
    _toIndex = newIndex;
  }
  else
    throw EXCEPTION(dataBase[newIndex]._symbol+" is not compatible with "+dataBase[_fromIndex]._symbol,ERRDIV);
  return *this;
}

void UnitConverter::rebase(Unit u) {
  int newIndex = getIndex(u);
  if ( dataBase[newIndex]._type == dataBase[_toIndex]._type ) {
    _from = u;
    _fromIndex = newIndex;
  }
  else
    throw EXCEPTION(dataBase[newIndex]._symbol+" is not compatible with "+dataBase[_toIndex]._symbol,ERRDIV);
}

int UnitConverter::getIndex(Unit u) {
  for ( unsigned i = 0 ; i < _nunit ; ++i ) {
    if ( dataBase[i]._unit == u ) return i;
  }
  return -1;
}

UnitConverter UnitConverter::getFromString(const std::string unit) {
  return UnitConverter(getUnit(unit));
}

UnitConverter::Unit UnitConverter::getUnit(const std::string unit) {
  for ( unsigned i = 0 ; i < _nunit ; ++i ) {
    if ( utils::tolower(dataBase[i]._symbol) == utils::tolower(unit) ) 
      return dataBase[i]._unit;
  }
  throw EXCEPTION(std::string("Unknown unit ")+unit,ERRDIV);
  return dataBase[0]._unit;
}

std::string UnitConverter::str() const {
  return dataBase[_toIndex]._symbol;
}


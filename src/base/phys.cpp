/**
 * @file phys.cpp
 *
 * @brief Store some physical constantes
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

#include "base/phys.hpp"
#include "base/exception.hpp"

Units::Energy Units::getEnergyUnit(std::string unit) {
  Energy eunit;
  if ( unit == "ev" )
    eunit = eV;
  else if ( unit == "ha" )
    eunit = Ha;
  else if ( unit == "thz" )
    eunit = THz;
  else if ( unit == "cm-1" )
    eunit = pcm;
  else
    throw EXCEPTION("Unknown energy unit",ERRDIV);
  return eunit;
}

double Units::getFactor(Energy from, Energy to) {
  using namespace phys;
  double factor = 1.e0;
  if ( from == to ) return 1.0;
  // Convert to eV
  switch (from) {
    case eV :
      break;
    case Ha :
      factor = Ha2eV;
      break;
    case THz :
      factor = 1e12 * Hz2eV;
      break;
    case pcm :
      factor = 1e2 * m2eV; // m-> ev = m2eV/input 
      break;
    default :
      throw EXCEPTION("Unit not yet implemented",ERRDIV);
      break;
  }
  switch (to) {
    case eV :
      break;
    case Ha :
      factor /= Ha2eV;
      break;
    case THz :
      factor /= (1e12 * Hz2eV);
      break;
    case pcm :
      factor /= (1e2 * m2eV);
      break;
    default :
      throw EXCEPTION("Unit not yet implemented",ERRDIV);
      break;
  }
  return factor;
}

std::string Units::toString(Energy unit) {
  std::string str;
  switch (unit) {
    case eV : str = "eV";
              break;
    case Ha : str = "Ha";
              break;
    case THz : str = "THz";
               break;
    case pcm : str = "cm-1";
               break;
    default:
               throw EXCEPTION("Unknown energy unit",ERRDIV);
  }
  return str;
}

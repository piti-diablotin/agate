/**
 * @file src/./fraction.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2019 Jordan Bieder
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


#include "base/fraction.hpp"
#include <sstream>
#include <cmath>
#include <iostream>

//
int Fraction::inverse(double val)
{
  int sign = (val < 0 ) ? -1 : 1;
  if ( val < 0 ) val = -val;
  if (!std::isnormal(val)) return 0;
  double inverseD = (1./val);
  int inverseI = static_cast<int>(inverseD+1e-8);
  return (std::abs(static_cast<double>(inverseI)-inverseD) < 1e-6) ? sign*inverseI : 0;
}

int Fraction::digits(double val)
{
  if (!std::isnormal(val)) return 0;
  double frac = val;
  int factor = 1;
  int safe = 0;
  while ( std::abs(frac-std::round(frac))>1e-6 && (safe++) < 14) {
    ++factor;
    frac *= 10.;
  }
  factor--;
  if ( safe > 13 ) return 0;
  return factor;
}

int Fraction::pgcd(int a, int b) {
  if ( a <= 0 || b <= 0 ) return 0;
  if ( a < b ) std::swap(a,b);
  while (b!=0) {
    int r = a%b;
    a=b;
    b=r;
  }
  return a;
}

void Fraction::compute()
{
  int deno;
  int digit;
  // First test if it is an integer
  if ( std::abs(static_cast<double>(static_cast<int>(_float))-_float) < 1e-6 ) {
    _numerator = static_cast<int>(_float);
    _denominator = 1;
  }
  // Then test if it is an inverse 1/X
  else if ( (deno=inverse(_float)) != 0) {
    _denominator = deno;
    _numerator = 1;
  }
  // Test if it is a rational number
  else if ( (digit=digits(_float)) != 0 ) {
    double factor = std::pow(10,digit);
    _numerator = _float*factor;
    _denominator = factor;
  }
  // Test if it is a multiple of an inverse 2/X 3/X and so on
  else {
    double val = 1./_float;
    if ( (digit=digits(val)) != 0 ) {
      double factor = std::pow(10,digit);
      _denominator = val*factor;
      _numerator = factor;
    }
  }
  if ( _denominator != 0 && _numerator != 0 ) {
    int sign = _numerator*_denominator;
    _denominator = std::abs(_denominator);
    _numerator = std::abs(_numerator);
    int div = pgcd(_numerator,_denominator);
    _numerator/=div;
    _denominator/=div;
    if ( sign < 0 ) _numerator *= -1;
  }
}

Fraction::Fraction() :
  _numerator(0),
  _denominator(0),
  _float(INFINITY)
{
  this->compute();
}

Fraction::Fraction(int num, int denom) :
  _numerator(num),
  _denominator(denom),
  _float((double)num/(double)denom)
{
  this->compute();
}

Fraction::Fraction(double val) :
  _numerator(0),
  _denominator(0),
  _float(val)
{
  this->compute();
}

//
Fraction::~Fraction() {
  ;
}

std::string Fraction::toString() const
{
  std::ostringstream str;
  switch (_denominator) {
    case 0:
      str << _float;
      break;
    case 1:
      str << _numerator;
      break;
    default:
      str << _numerator << "/" << _denominator;
      break;
  }
  return str.str();
}

std::ostream& operator<<(std::ostream& stream, const Fraction& frac) {
  stream << frac.toString();
  return stream;
}

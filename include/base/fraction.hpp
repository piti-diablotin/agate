/**
 * @file include/./fraction.hpp
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


#ifndef FRACTION_HPP
#define FRACTION_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <string>
#include <ostream>

/** 
 *
 */
class Fraction {

  private :
    int _numerator;
    int _denominator;
    double _float;

  protected :

    static constexpr int primes[25] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};

    static int inverse(double val);

    static int digits(double val);

    static int pgcd(int a, int b);

    void compute();

  public :

    /**
     * Constructor.
     */
    Fraction();

    /**
     * Constructor.
     */
    Fraction(int num, int denom);

    /**
     * Constructor.
     */
    Fraction(double val);

    /**
     * Destructor.
     */
    virtual ~Fraction();

    /**
     * Transform the fraction to a nice string
     * @return a string containing the well-formated fraction
     */
    std::string toString() const;

    friend std::ostream& operator<<(std::ostream& stream, const Fraction& frac);

};

#endif  // FRACTION_HPP

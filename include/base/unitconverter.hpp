/**
 * @file include/base/energyunit.hpp
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


#ifndef ENERGYUNIT_HPP
#define ENERGYUNIT_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "base/phys.hpp"
#include <iostream>
#include <string>

/** 
 *
 */
class UnitConverter {

  public :

    enum Unit { 
      eV, Ha, THz, pcm, 
      au, pOhmpcm,
      amu, kg, emu,
      A, bohr
    };

    enum Type { Energy, Conductivity, Mass, Length };

  private :
    Unit _from;
    int _fromIndex;
    Unit _to;
    int _toIndex;

    typedef struct {
      Unit _unit;
      Type _type;
      std::string _symbol;
      double _toRef;
    } UnitDefinition;

    static const int _nunit = 11;
    static const UnitDefinition dataBase[_nunit]; 

    int getIndex(Unit u);

  protected :

  public :

    /**
     * Constructor.
     */
    UnitConverter();

    /**
     * Constructor.
     */
    UnitConverter(Unit u);

    /**
     * Destructor.
     */
    virtual ~UnitConverter();

    UnitConverter& operator=(Unit u);

    void rebase(Unit u);

    friend std::string operator+(std::string &str, UnitConverter &eunit);

    friend std::istream& operator>>(std::istream &in, UnitConverter &eunit);

    friend std::ostream& operator<<(std::ostream &out, UnitConverter &eunit);

    std::string str() const ;

      friend double operator*(double val, const UnitConverter &eunit) {
        double factor = 1.0;
        factor *= dataBase[eunit._fromIndex]._toRef;
        factor /= dataBase[eunit._toIndex]._toRef;
        return val*factor;
      }

      friend  double operator*(const UnitConverter &eunit, double val) {
        double factor = 1.0;
        factor *= dataBase[eunit._fromIndex]._toRef;
        factor /= dataBase[eunit._toIndex]._toRef;
        return val*factor;
      }

      friend double operator/(double val, const UnitConverter &eunit) {
        double factor = 1.0;
        factor *= dataBase[eunit._fromIndex]._toRef;
        factor /= dataBase[eunit._toIndex]._toRef;
        return val/factor;
      }

    static UnitConverter getFromString(const std::string unit);
    static Unit getUnit(const std::string unit);


};

#endif  // ENERGYUNIT_HPP

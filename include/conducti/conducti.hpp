/**
 * @file include/conducti/conducti.hpp
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


#ifndef CONDUCTI_HPP
#define CONDUCTI_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "conducti/abiopt.hpp"

/** 
 *
 */
class Conducti {

  private :
    int _nomega;
    double _omegaMin;
    double _omegaMax;
    double _smearing;

    int _bandMin;
    int _bandMax;
    double _eMin;
    double _eMax;
    std::vector<int> _histogram;

  protected :

  public :
    enum TensorIndex { XX, YY, ZZ, XY, XZ, YZ };

    /**
     * Constructor.
     */
    Conducti();

    /**
     * Destructor.
     */
    virtual ~Conducti();

    std::array<std::vector<double>,6> fullTensor(const AbiOpt &abiopt);

    std::array<std::vector<double>,3> diagonalTensor(const AbiOpt &abiopt);

    std::vector<double> traceTensor(const AbiOpt &abiopt);

};

#endif  // CONDUCTI_HPP

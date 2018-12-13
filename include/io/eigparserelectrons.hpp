/**
 * @file include/./eigparserelectrons.hpp
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


#ifndef EIGPARSERELECTRONS_HPP
#define EIGPARSERELECTRONS_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "io/eigparser.hpp"
#include "io/dtset.hpp"
#include <memory>

/** 
 *
 */
class EigParserElectrons : public EigParser {

  private :

  protected :
    int                              _lmax;
    std::vector<std::vector<double>> _fractions;
    std::vector<unsigned>            _lmMask;
    std::unique_ptr<Dtset>           _dtset;

  public :

    /**
     * Constructor.
     */
    EigParserElectrons();

    /**
     * Destructor.
     */
    virtual ~EigParserElectrons();

    /**
     * Use this function to only display l and m value given in arguement
     * @param umaskl umask for l number 0 -> _lmax-1
     * @param umaskm umask for m number -l -> l
     */
    void selectLM(int l, std::vector<int> &umaskm);

    /**
     * Getter color which represent the atom decomposition of the ith band
     * @param iband The band for which we want the eigen values;
     * @param ispin Spin of the band we want can only be 1 or 2
     * @param mask id of atoms that should not be considered if projection is asked
     * @return the RBG color to be used along this band
     */
    std::vector<unsigned> getBandColor(const unsigned iband, const unsigned ispin, std::vector<unsigned> mask = std::vector<unsigned>()) const;

};

#endif  // EIGPARSERELECTRONS_HPP

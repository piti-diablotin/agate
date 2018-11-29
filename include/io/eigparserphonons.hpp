/**
 * @file include/./eigparserphonons.hpp
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


#ifndef EIGPARSERPHONONS_HPP
#define EIGPARSERPHONONS_HPP

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
class EigParserPhonons : public EigParser {

  private :

  protected :
    std::unique_ptr<Dtset>             _dtset;
    std::vector< std::vector<double> > _eigenDisp;///< eigen displacement for each kpt.

  public :

    /**
     * Constructor.
     */
    EigParserPhonons();

    /**
     * Destructor.
     */
    virtual ~EigParserPhonons();

    /**
     * Getter color which represent the atom decomposition of the ith band
     * @param iband The band for which we want the eigen values;
     * @param ispin Spin of the band we want can only be 1 or 2
     * @param mask id of atoms that should not be considered if projection is asked
     * @return the RBG color to be used along this band
     */
    virtual std::vector<unsigned> getBandColor(const unsigned iband, const unsigned ispin, std::vector<unsigned> mask = std::vector<unsigned>()) const;

    /**
     * Getter proportion of the band for each atom and each kpt.
     * @param iband The band for which we want the projection
     * @param ispin Spin of the band we want can only be 1 for the moment
     * @return a vector for each kpt with the percentage of component for each atom
     * vector[nkpt][natom] is basically the result and between 0 and 1.
     */
    virtual std::vector<std::vector<double>> getBandProjection(const unsigned iband, const unsigned ispin) const;
};

#endif  // EIGPARSERPHONONS_HPP

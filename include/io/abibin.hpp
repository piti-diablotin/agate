/**
 * @file include/./abibin.hpp
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


#ifndef ABIBIN_HPP
#define ABIBIN_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "io/dtset.hpp"
#include "io/abihdr.hpp"
#include "base/geometry.hpp"

/** 
 *
 */
class AbiBin : public Dtset {

  private :

  protected :

    static const std::vector<int> densityFform;   ///< fform positive integer values corresponding to abinit "density" class files
    static const std::vector<int> potentialFform; ///< fform posiotive integer values corresponding to abinit "potential" class files
    int _nspden;                                  ///< Number of densities to read in the file
    int _ngfft[3];                                ///< Grid for FFT along a, b and c
    std::vector<double> _fftData;                 ///< Data point along a, b and c directions.
    AbiHdr _header;

  public :

    enum getDen { UP, DOWN, SUM, DIFF, X, Y, Z };

    /**
     * Constructor.
     */
    AbiBin();

    /**
     * Destructor.
     */
    virtual ~AbiBin();

    /**
     * Fill a Dtset from a _DEN file 
     * @param filename File of a unformatted fortran  binary file written by Abinit
     */
    virtual void readFromFile(const std::string& filename);

    enum gridDirection { A, B, C };

    /**
     * Get grid dimension
     * @param axis The direction to get the number of points
     * @result the number of point along the axis direction.
     */
    int getPoints(gridDirection dir);

    /**
     * Get grid vector
     * @param axis The direction to get the vector
     * @result the cartesian coordinates of the vector.
     */
    geometry::vec3d  getVector(gridDirection dir);

    /**
     * Get data for a plan at position i normal to the given direction
     * @param origin origin of the plan
     * @param dir normal to the plan
     * @param data container with the correct size and data.
     * Data is represented rowwise a then b then c
     * @return scaling parameter (max value used to rescale data)
     */
    double getData(int origin, gridDirection dir, getDen function, std::vector<double> &data);

    /**
     * Getter to get the number of density we have
     * @return the number of density. Can be 1 2 or 4 in case of spin orbit coupling
     */
    int getNspden() { return _nspden; }

};

#endif  // ABIBIN_HPP

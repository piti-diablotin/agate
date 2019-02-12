/**
 * @file include/conducti/abiopt.hpp
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


#ifndef ABIOPT_HPP
#define ABIOPT_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "io/abihdr.hpp"
#include <complex>

/** 
 *
 */
class AbiOpt {

  private :

    AbiHdr _header; ///< Header from abinit containing all information for dimension and system
    std::vector<std::array<std::vector<std::complex<double>>,3>> _nablas; ///< Nabla matrix for each kpt and each direction.
    std::vector<std::vector<double>> _eigens; ///< Eigen energies for each kpt;

  protected :

  public :

    /**
     * Constructor.
     */
    AbiOpt();

    /**
     * Read all information from file
     * @param filename Filename to read nabla operator
     */
    void readFromFile(const std::string &filename);

    /**
     * Destructor.
     */
    virtual ~AbiOpt();

    int nsppol() const { return _header.nsppol(); }
    int nkpt() const { return _header.nkpt(); }
    double fermie() const { return _header.fermie(); }
    int nband(int isppol, int ikpt) const;
    std::vector<double>::const_iterator occ(int isppol, int ikpt) const;
    const std::vector<double>& eigen(int isppol, int ikpt) const;
    const std::vector<std::complex<double>>& nabla(int isppol, int ikpt, int idir) const;
    const geometry::mat3d& rprim() const { return _header.rprim(); }
    const std::vector<double>& wtk() const { return _header.wtk(); }
    int mband() const { return _header.mband(); }

};

#endif  // ABIOPT_HPP

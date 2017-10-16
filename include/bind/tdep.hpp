/**
 * @file tdeo.hpp
 *
 * @brief Interface to the TDEP software in Abinit
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2017 Jordan Bieder
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

#ifndef TDEP_HPP
#define TDEP_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "io/dtset.hpp"
#include "hist/histdatamd.hpp"
#include "hist/histdata.hpp"
#include "base/geometry.hpp"
#include <memory>

/**
 * Call correctly the TDEP executable if available to compute the phonon spectra of a MD
 * Basically, for the momement only Phonons will be interfaced
 */
class Tdep {

  public :

    enum Mode { Debug, Normal };

  private :

    Mode                        _mode;      ///< Mode to run TDEP
    Dtset                       _unitcell;  ///< Unit cell to use in TDEP
    unsigned                    _bravais;   ///< Bravais lattice of the unit cell
    int                         _centering; ///< F I P A B C -3 -1 0 1 2 3
    std::unique_ptr<HistDataMD> _supercell; ///< Trajectory to use to compute the phonons. Only real MD can be used;
    unsigned                    _tbegin;    ///< First time to use
    unsigned                    _tend;      ///< Last time to use (excluded)
    unsigned                    _step;      ///< Step to use to advance in the Hist file
    double                      _rcut;      ///< Radius cutoff for computing shell inside tdep
    std::array<double,9>        _multiplicity; ///< Holds the multiplicity of the supercell with respect to unitcell

    void computeMultiplicity();


  public :

    /**
     * Constructor : initializes with empty data.
     */
    Tdep();

    /**
     * Destructor : nothing.
     */
    ~Tdep();

    void mode(Mode m);

    void unitcell(Dtset uc);

    void unitcell(HistData* hist, unsigned time);

    void supercell(HistDataMD* hist);

    void tbegin(unsigned t);
    
    void tend(unsigned t);

    void step(unsigned istep);

    void rcut(double r);

    void multiplicity(geometry::mat3d m);

    void tdep();

    const Dtset& unitcell() const {return _unitcell;}
    geometry::mat3d multiplicity() const {return _multiplicity;}

};

#endif  // TDEP_HPP

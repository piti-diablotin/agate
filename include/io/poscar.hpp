/**
 * @file poscar.hpp
 *
 * @brief Manage information for vasp POSCAR
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
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

#ifndef POSCAR_HPP 
#define POSCAR_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif


#include "io/dtset.hpp"

/**
 * Replace the load function to read the poscar file instead of abinit input
 */
class Poscar : public Dtset {

  private :
    std::string    _title;    ///< First line of the POSCAR file
    std::string    _names;    ///< If present, the line before the number of each type of atoms.

  public :

    /**
     * Constructor that builds an empty default Dtset.
     */
    Poscar();

    /**
     * Constructor that builds a dtset from a _HIST file taken the itime structure.
     * @param hist HistData structure that contains a MD simulation or just one shot.
     * @param itime Time to use from the hist to build the poscar
     */
    Poscar(const HistData& hist, const unsigned itime = 0);

    /**
     * Constructor that builds a Poscar based on a Dtset.
     * @param dtset the Data set to copy
     */
    explicit Poscar(const Dtset& dtset);

    /**
     * Copy a Poscar based on a Dtset.
     * @param dtset the Data set to copy
     */
    Poscar& operator = (const Dtset& dtset);

    /**
     * Destructor.
     */
    ~Poscar();

    /**
     * Fill a Dtset from an poscar file.
     * @param filename The name of the input file to read.
     */
    virtual void readFromFile(const std::string& filename);

    /**
     * Creat an poscar file with the current Dtset.
     * Dump the poscar file to the stream
     * @param out The ostream stream to write into.
     */
    virtual void dump(std::ostream& out) const;

    /**
     * Creat an poscar file with the current Dtset.
     * @param filename The name of the file to be written.
     */
    virtual void dump(const std::string& filename) const {
      Dtset::dump(filename);
    }


};

#endif  //POSCAR_HPP

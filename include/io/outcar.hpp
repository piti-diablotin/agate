/**
 * @file outcar.hpp
 *
 * @brief Manage information for vasp OUTCAR
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2020 Jordan Bieder
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

#ifndef OUTCAR_HPP 
#define OUTCAR_HPP

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
class Outcar : virtual public Dtset {

  public :

    /**
     * Constructor that builds an empty default Dtset.
     */
    Outcar();

    /**
     * Destructor.
     */
    ~Outcar();

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

#endif  //OUTCAR_HPP

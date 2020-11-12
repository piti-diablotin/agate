/**
 * @file include/plot/dosdb.hpp
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


#ifndef DOSDB_HPP
#define DOSDB_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "io/electrondos.hpp"
#include <unordered_map>

/** 
 *
 */
class DosDB {

  private :
    std::vector<ElectronDos> _db;
    std::unordered_map<unsigned,unsigned> _ordering;

  protected :

  public :

    /**
     * Constructor.
     */
    DosDB();

    /**
     * Destructor.
     */
    virtual ~DosDB();

    void buildFromPrefix(std::string prefix);

    std::vector<unsigned> list() const;

    void clear();

    const ElectronDos& total() const;

    const ElectronDos& atom(unsigned iatom) const;

};

#endif  // DOSDB_HPP

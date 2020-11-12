/**
 * @file include/histdatanc.hpp
 *
 * @brief Read _HIST file from a NetCDF file
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
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


#ifndef HISTDATANC_HPP
#define HISTDATANC_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <ostream>
#include "hist/histdatamd.hpp"

/** 
 * Handle _HIST file in NetCDF. 
 * Return an error when opening a NC file is not compiled with NetCDF support
 */
class HistDataNC : public HistDataMD {

  private :

    std::vector<double> _ekin_img;        ///< Kinetic energy 
    std::vector<double> _entropy_img;     ///< Electronic entropy


  protected :

  public :

    /**
     * Constructor.
     */
    HistDataNC();

    /**
     * Copy constructor.
     * @param hist Supposed to be copied
     */
    HistDataNC(const HistData& hist);

    /**
     * Copy constructor.
     * @param hist Supposed to be copied
     */
    HistDataNC(HistData&& hist);

    /**
     * Copy constructor.
     * @param hist Supposed to be copied
     */
    HistDataNC(const HistDataNC& hist) = delete;

    /**
     * Move constructor.
     * @param hist Supposed to be moved
     */
    HistDataNC(HistDataNC&& hist) = delete;

    /**
     * Destructor.
     */
    virtual ~HistDataNC();

    /**
     * Open a file and read it to fill the data
     * @param filename Name of the file
     */
    virtual void readFromFile(const std::string& filename);

    /**
     * Dump the full history into the current format
     * @param filename Name or base name of the file to creat.
     * @param tbegin first time to start with
     * @param tend Last time (not included) in the dumping
     */
    virtual void dump(const std::string& filename, unsigned tbegin, unsigned tend, unsigned step = 1) const;

    /**
     * Dump the full history into the current format
     * @param hist a random hist to dump in _HIST format.
     * @param filename Name or base name of the file to creat.
     * @param tbegin first time to start with
     * @param tend Last time (not included) in the dumping
     */
    static void dump(HistData &hist, const std::string& filename, unsigned tbegin, unsigned tend, unsigned step = 1);

};

#endif  // HISTDATANC_HPP

/**
 * @file include/histdataoutnc.hpp
 *
 * @brief Read _OUT.nc file from a NetCDF file
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


#ifndef HISTDATAOUTNC_HPP
#define HISTDATAOUTNC_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "hist/histdata.hpp"

/** 
 * Handle _OUT.nc file in NetCDF. 
 * Return an error when opening a NC file is not compiled with NetCDF support
 */
class HistDataOutNC : public HistData {

  private :

  protected :

  public :

    /**
     * Constructor.
     */
    HistDataOutNC();

    /**
     * Copy constructor.
     * @param hist Supposed to be copied
     */
    HistDataOutNC(const HistDataOutNC& hist) = delete;

    /**
     * Move constructor.
     * @param hist Supposed to be moved
     */
    HistDataOutNC(HistDataOutNC&& hist) = delete;

    /**
     * Destructor.
     */
    virtual ~HistDataOutNC();

    /**
     * Assignement operator
     * @param hist Supposed to be copied
     * @return The supposed new hist
     */
    HistDataOutNC& operator = (const HistDataOutNC& hist) = delete;

    /**
     * Move operator
     * @param hist Supposed to be moved
     * @return The supposed new hist
     */
    HistDataOutNC& operator = (HistDataOutNC&& hist) = delete;

    /**
     * Open a file and read it to fill the data
     * @param filename Name of the file
     */
    virtual void readFromFile(const std::string& filename);

    virtual bool hasEtotal() const { return true; }

    virtual bool hasStress() const { return true; }
};

#endif  // HISTDATAOUTNC_HPP

/**
 * @file include/hist/histdatagsr.hpp
 *
 * @brief  Read _GSR.nc files from abinit
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


#ifndef HISTDATAGSR_HPP
#define HISTDATAGSR_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "hist/histdata.hpp"

/** 
 * Handle _GSR.nc files in NetCDF format form abinit.
 */
class HistDataGSR : public HistData {

  private :

  protected :

  public :

    /**
     * Constructor.
     */
    HistDataGSR();

    /**
     * Copy constructor.
     * @param hist Supposed to be copied
     */
    HistDataGSR(const HistDataGSR& hist) = delete;

    /**
     * Move constructor.
     * @param hist Supposed to be moved
     */
    HistDataGSR(HistDataGSR&& hist) = delete;

    /**
     * Destructor.
     */
    virtual ~HistDataGSR();
    /**
     * Assignement operator
     * @param hist Supposed to be copied
     * @return The supposed new hist
     */
    HistDataGSR& operator = (const HistDataGSR& hist) = delete;

    /**
     * Move operator
     * @param hist Supposed to be moved
     * @return The supposed new hist
     */
    HistDataGSR& operator = (HistDataGSR&& hist) = delete;

    /**
     * Open a file and read it to fill the data
     * @param filename Name of the file
     */
    virtual void readFromFile(const std::string& filename);
};

#endif  // HISTDATAGSR_HPP

/**
 * @file include/histdatadtset.hpp
 *
 * @brief Build a fake HistData from a Dtset to visualise a structure
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


#ifndef HISTDATADTSET_HPP
#define HISTDATADTSET_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "hist/histdata.hpp"
#include "io/dtset.hpp"

/** 
 * Handle a basic Abinit dtset and convert it to the HistData format 
 * so the structure can be visualized.
 */
class HistDataDtset : public HistData {

  private :

  protected :

  public :

    /**
     * Constructor.
     */
    HistDataDtset();

    /**
     * Constructor.
     */
    HistDataDtset(const Dtset& dtset);

    /**
     * Destructor.
     */
    virtual ~HistDataDtset();

    /**
     * Open a file and read it to fill the data
     * @param filename Name of the file
     */
    virtual void readFromFile(const std::string& filename);

    /**
     * Build the hist type with a dtset
     * @param filename Name of the file
     */
    virtual void buildFromDtset(const Dtset& dtset);

    /**
     * Dump the full history into the current format
     * @param filename Name or base name of the file to creat.
     */
    //virtual void dump(const std::string& filename) const;

    virtual bool hasEtotal() const { return true; }

    virtual bool hasStress() const { return true; }
};

#endif  // HISTDATADTSET_HPP

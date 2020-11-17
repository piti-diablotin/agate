/**
 * @file include/ddbphonopy.hpp
 *
 * @brief Read a yaml file from phonopy output
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2016 Jordan Bieder
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


#ifndef DDBPHONOPY_HPP
#define DDBPHONOPY_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "io/ddb.hpp"
#include "io/dtset.hpp"

/** 
 * Read a qpoints.yaml file and  store all the second derivative information
 */
class DdbPhonopy : public Ddb {

  private :

  protected :

  public :

    /**
     * Constructor.
     */
    DdbPhonopy();

    /**
     * Destructor.
     */
    virtual ~DdbPhonopy();

    /**
     * Fill a DDB with the content of a file.
     * @param filename The name of the input file to read.
     */
    virtual void readFromFile(const std::string& filename);


    /**
     * Build the base Dtset from an other Dtset to get some structural information if needed
     * So it copies all info from dtset and remove the mass factor from the 2nd derivatives
     * @param dtset is the Dtset for the reference structure
     */
    virtual void buildFrom(const Dtset& dtset);
};

#endif  // DDBPHONOPY_HPP

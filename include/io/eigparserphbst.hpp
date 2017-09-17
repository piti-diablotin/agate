/**
 * @file include/eigparserphbst.hpp
 *
 * @brief 
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


#ifndef EIGPARSERPHBST_HPP
#define EIGPARSERPHBST_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "io/eigparser.hpp"

/** 
 *
 */
class EigParserPHBST : public EigParser {

  private :

  protected :

  public :

    /**
     * Constructor.
     */
    EigParserPHBST();

    /**
     * Destructor.
     */
    virtual ~EigParserPHBST();

    /**
     * File to read to get the eigen value (_EIG file from abinit)
     * @param filename Name of the _EIG file
     */
    virtual void readFromFile(const std::string& filename);
};

#endif  // EIGPARSERPHBST_HPP

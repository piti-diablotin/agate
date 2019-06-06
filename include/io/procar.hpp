/**
 * @file include/io/procar.hpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2019 Jordan Bieder
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


#ifndef PROCAR_HPP
#define PROCAR_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "io/eigparserelectrons.hpp"

/** 
 *
 */
class Procar : public EigParserElectrons {

  private :

  protected :

  public :

    /**
     * Constructor.
     */
    Procar();

    /**
     * Destructor.
     */
    virtual ~Procar();

    /**
     * File to read to get the eigen value (PROCAR from VASP or Abinit)
     * @param filename Name of the PROCAR file
     */
    virtual void readFromFile(const std::string& filename);
};

#endif  // PROCAR_HPP

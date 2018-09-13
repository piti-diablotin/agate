/**
 * @file include/./trimap.hpp
 *
 * @brief 
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


#ifndef TRIMAP_HPP
#define TRIMAP_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "graphism/trisphere.hpp"

/** 
 *
 */
class TriMap : public TriObj {

  private :

  protected :
    _float  *_unitColor;     ///< Unit object indices
    _uint    _vboColor;       ///< VBO for vertex[0] and indices[1]

  public :

    /**
     * Constructor.
     */
    TriMap(bool opengl);

    /**
     * Copy
     */
    TriMap(const TriMap& cloud) = delete;

    /**
     * Move
     */
    TriMap(TriMap&& map) = delete;

    /**
     * Copy
     */
    TriMap& operator = (const TriMap& map) = delete;

    /**
     * Move
     */
    TriMap& operator = (TriMap&& map) = delete;
    /**
     * Destructor.
     */
    virtual ~TriMap();

    void genUnit(double *origin, double *udir, double *vdir, int upoint, int vpoint);
};

#endif  // TRIMAP_HPP

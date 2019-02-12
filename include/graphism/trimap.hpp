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
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "graphism/triobj.hpp"
#include <vector>

/** 
 *
 */
class TriMap : public TriObj {

  private :

  protected :
    _float  *_unitColor;     ///< Unit object indices
    _uint    _vboColor;      ///< VBO for vertex[0] and indices[1]
    int _currentUpoint;      ///< Current number of points along u direction 
    int _currentVpoint;      ///< Current number of points along v direction 
    bool _refresh;           ///< Things must be updated or VBOs are ok ?

  public :

    /**
     * Constructor.
     */
    TriMap(bool opengl);

    /**
     * Copy
     */
    TriMap(const TriMap& map) = delete;

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

    /**
     * Draw a map on the screen
     * The grid must have been set up before with genUnit.
     * The values in the values array must be between -1 and 1.
     * @param values The values for each point of the grid. 
     * Values are row ordering if udir is the row direction and vdir the colomn direction.
     * iteration over vdir is the inner loop.
     * @param zero A color for the 0 value, each component is between 0 and 1
     * @param plus A color for the +1 value, each component is between 0 and 1
     * @param minus A color for the -1 value, each component is between 0 and 1
     * @param refresh If true, the color buffer is updated with values.
     */
    void draw(std::vector<double> &values, float zero[3], float plus[3], float minus[3],bool refresh);

    void pop(){;}
    void push(){;}
};

#endif  // TRIMAP_HPP

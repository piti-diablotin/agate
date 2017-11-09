/**
 * @file include/canvaslocal.hpp
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


#ifndef CANVASROT_HPP
#define CANVASROT_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "canvas/canvaspos.hpp"
#include "graphism/tricube.hpp"

/** 
 * This canvas is designed to only draw local information around octahedra
 * The atoms are not plotted, only a square is display at the position of the atom at the center of the octahedra
 * with colors related to the rotations of the octahedra.
 */
class CanvasLocal : public CanvasPos {

  private :
    enum LocalView { ANGLES, LENGTHS };

    float                   _octacolor[6]; ///< Store the two colors for plotting the cubes representing the rotations.
    TriCube                 _cube;         ///< A cube to visualize the rotations.
    LocalView               _view;
    std::vector<std::array<float,3>> _orientations; ///< Orientation of the octahedra with respect to cartesian axis.

  protected :

    /**
     * Alter/modify a canvas with respect to a keyword and stream containing data.
     * @param token keyword to know what to do
     * @param stream the stream with the other data to be parsed.
     */
    void my_alter(std::string token, std::istringstream &stream);

    
    /**
     * Convert the octahedra to the correct type
     */
    void convertOctahedra();

  public :

    /**
     * Constructor.
     */
    CanvasLocal(bool drawing);

    /**
     * Constructor from a canvaspos
     * Move everything and initialize octacolor and cube
     * @param canvas Canvas to move
     */
    CanvasLocal(CanvasPos &&canvas);

    /**
     * Destructor.
     */
    virtual ~CanvasLocal();

    /**
     * Refresh what to see on the screen
     * @param cam vector in which direction we are watching. Not used here.
     */

    virtual void refresh(const geometry::vec3d &cam, TextRender &render);

    /**
     * Construct the list of octahedra to draw
     * @param z the znucl to draw octahedra around
     */
    virtual void updateOctahedra(int z);

    /**
     * Display help message with all command that are use in this class
     * @param out the stream to write the help message
     */
    static void help(std::ostream &out);

};

#endif  // CANVASROT_HPP

/**
 * @file include/./canvasdensity.hpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2018 Jordan Bieder
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


#ifndef CANVASDENSITY_HPP
#define CANVASDENSITY_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "canvas/canvaspos.hpp"
#include "graphism/trimap.hpp"
#include "io/abibin.hpp"

/** 
 *
 */
class CanvasDensity :public CanvasPos {

  public:
    enum scaleFunc { linear, sqrt, log }; ///< How to scale the values when displaying. Apply none, sqrt or log function.

  private :

    int _origin;           ///< indice of the origin along the normal.
    int _npoints;          ///< number of points along the normal.
    int _ipoint;           ///< Current point along the normal.
    int _ibegin;           ///< Starting point for the animation
    int _iend;             ///< Last point for the animation.
    double _scaleValues;   ///< Scale values of density by this factor;
    AbiBin::gridDirection _normal; ///< Normal direction of the map.
    AbiBin _density;       ///< File containin density information.
    TriMap _map;           ///< map rendering with opengl
    float _colors[3][3];   ///< colors for maps for -1;0;1
    AbiBin::getDen _dispDen;
    scaleFunc _scaleFunction;                     ///< Scale function to apply to data before plotting

    

  protected :

    /**
     * Alter/modify a canvas with respect to a keyword and stream containing data.
     * @param token keyword to know what to do
     * @param stream the stream with the other data to be parsed.
     */
    virtual void my_alter(std::string token, std::istringstream &stream);

  public :

    /**
     * Constructor.
     */
    CanvasDensity(bool drawing);

    /**
     * Constructor from a canvaspos
     * Move everything and initialize octacolor and cube
     * @param canvas Canvas to move
     */
    CanvasDensity(CanvasPos &&canvas);

    /**
     * Destructor.
     */
    virtual ~CanvasDensity();

    /**
     * set additional data for density plotting
     */
    void setData();

    /**
     * Clear data related to density plottin
     */
    virtual void clear();

    /**
     * Set some data from a histdata
     * @param hist History of some parameters.
     */
    virtual void setHist(HistData& hist);

    /**
     * Refresh what to see on the screen
     * @param cam vector in which direction we are watching. Not used here.
     */
    virtual void refresh(const geometry::vec3d &cam, TextRender &render);

    /**
     * Go to next time step if available
     */
    virtual void nextStep();

    /**
     * Go to next time step if available
     */
    virtual void step(const int istep);

    /**
     * Go to previous time step if available
     */
    virtual void previousStep();

    /**
     * Set number of time step and check _tbegin/_tend
     * @param npoints The parser containing the parameters
     */
    virtual void setNTime(int npoints);

    /**
     * Go to next frame to draw
     * @param count The number of frame to go forward
     */
    virtual void nextFrame(const int count=1);

    /**
     * Get the time step
     * @return ipoint
     */
    virtual int itime() const;

    /**
     * Get the number of time step
     * @return npoints
     */
    virtual int ntime() const;

    virtual int tbegin() const;

    virtual int tend() const;

    /**
     * Display help message with all command that are use in this class
     * @param out the stream to write the help message
     */
    static void help(std::ostream &out);
    AbiBin::gridDirection normal() const;
    AbiBin::getDen dispDen() const;
    scaleFunc scaleFunction() const;
    double scaleValues() const;
    const float (*colors() const )[3];
};

#endif  // CANVASDENSITY_HPP

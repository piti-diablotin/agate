/**
 * @file include/canvasphonons.hpp
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


#ifndef CANVASPHONONS_HPP
#define CANVASPHONONS_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include "canvas/canvaspos.hpp"
#include "phonons/dispdb.hpp"
#include "phonons/supercell.hpp"
#include <map>

/** 
 *
 */
class CanvasPhonons : public CanvasPos {

  private :

    double    _amplitudeDisplacement; ///< pre-Factor to increase displacement amplitude.
    DispDB    _displacements; ///< List of all eigen displacement;
    Dtset     _reference;     ///< reference structure to build supercell
    Supercell _supercell;  ///< Supercell to draw with displacements;
    DispDB::qptTree _condensedModes;
    DispDB::qptTree::iterator _qptModes;
    unsigned _ntime;
    std::string _originalFile; ///< Opened file.

    /**
     * Alter/modify a canvas with respect to a keyword and stream containing data.
     * @param token keyword to know what to do
     * @param stream the stream with the other data to be parsed.
     */
    virtual void my_alter(std::string token, std::istringstream &stream);

    /**
     * Try to load displacement from a file assuming it is a DDB file
     * @param filename the name of the ddb file
     * @return True if the file does contains a DDB, false otherwise
     */
    bool readDdb(const std::string& filename);

    /** 
     * Search in _condensedModes the qpt and if it is found, then
     * _qptModes is set to these Modes. Otherwise, nothing is changed and
     * the function returns false.
     * @param qpt The qpt we are looking for in _condensedModes
     * @return true if the qpt was found false otherwise
     */
    bool selectQpt(geometry::vec3d qpt);

  protected :

  public :

    /**
     * Constructor.
     */
    CanvasPhonons(bool drawing);

    /**
     * Copy constructor from CanvasPos
     */
    CanvasPhonons(const CanvasPos& canvas);

    /**
     * Destructor.
     */
    virtual ~CanvasPhonons();

    /**
     * Define reference structure
     * This will actually try to build a histdata and use the next method
     * @param filename
     */
    virtual void openFile(const std::string& filename);

    /**
     * Add some data from a file 
     * This will actually try to build a histdata and append data to the existing histdata
     * @param filename
     */
    virtual void appendFile(const std::string& filename);

    /**
     * Build the animations with all the qpt in the _condensedModes
     */
    void buildAnimation();

    /**
     * Display help message with all command that are use in this class
     * @param out the stream to write the help message
     */
    static void help(std::ostream &out);
};

#endif  // CANVASPHONONS_HPP

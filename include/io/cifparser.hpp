/**
 * @file include/cifparser.hpp
 *
 * @brief Parse and read a cif stream.
 * Should be very general but is aimed at findsym.
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


#ifndef CIFSTREAM_HPP
#define CIFSTREAM_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include <istream>
#include <string>
#include <vector>
#include <map>

/** 
 * Read a CIF file with several data block but
 * will fail for save and global frame.
 */
class CifParser {

  public :  // Public methods follow

    /**
     * Definition of a small structure to store a data_ loop_
     */
    class DataLoop {
      public :
        unsigned                                _nfield; ///< Number of field in this structure (size of _header).
        unsigned                                _nentry; ///< Number of entries in the _data vector (_data.size()).
        std::vector<std::string>                _header; ///< Header for a loop_.
        std::vector< std::vector<std::string> > _data;   ///< Store data First vector is the line, second is the columns.
        DataLoop() : _nfield(0), _nentry(0), _header(), _data() {;} ///< Initialization
        ~DataLoop() {;}                                             ///< Destructor
        /**
         * Get the id of the header colum that correspondond to the desired field
         * @param field String of the header of the colum we are looking for
         * @return the vector indice of _header.
         */
        unsigned getColumn(const std::string& field) const ;
    };

    /**
     * Definition of a data block
     */
    class DataBlock {
      public :
        std::string                        _name;        ///< Name of the block
        std::map<std::string,std::string>  _tags;        ///< Lists all tags found in the stream.
        std::vector<DataLoop>              _dataLoops;   ///< Store loop data
        DataBlock() : _name(), _tags(), _dataLoops() {;} ///< Initialization
        DataBlock(const std::string& name) : _name(name),
                               _tags(), _dataLoops() {;} ///< Initialization
        ~DataBlock() {;}                                 ///< Destructor

        /**
         * Get the tag value
         * @param tag The tag to look for
         * @return the associated value to tag as a string. 
         */
        std::string getTag(const std::string& tag);

        /**
         * Find a DataLoop containing the given header name
         * @param headerName The header tag we are looking for
         * @return A const DataLoop which has the header tag in its header.
         */
        const DataLoop& getDataLoop(const std::string& headerName) const;
    };

  private :

    unsigned                           _ndataBlock; ///< Number of data blocks in the stream.
    std::vector<DataBlock>             _dataBlocks; ///< Contains all blocks of data.

  public :

    static const int ERFOUND = 100;                 ///< Error code if a block name is not found in the CIF stream.

    /**
     * Constructor.
     * Do nothing;
     */
    CifParser();

    /**
     * Destructor.
     */
    virtual ~CifParser();

    /**
     * Get the number of block read.
     * @return the number of block read.
     */
    inline unsigned ndataBlock() const {
      return _ndataBlock;
    }

    /**
     * Parse a stream to build the data
     * @param stream stream to aquire for reading.
     */
    void parse(std::istream& stream);

    /**
     * Get a block by its name.
     * If the block can not be found, then throw an exception with core CifParser::ERFOUND.
     * @return a const DataBlock of the desired block.
     */
    const DataBlock getDataBlock(const std::string& name) const;

    /**
     * Get a block by its name.
     * If the block can not be found, then throw an exception with core CifParser::ERFOUND.
     * @return a const DataBlock of the desired block.
     */
    const DataBlock getDataBlock(unsigned i) const;
};

#endif  // CIFSTREAM_HPP

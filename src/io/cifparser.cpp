/**
 * @file src/cifparser.cpp
 *
 * @brief Implementation of the CifParser class.
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


#include "io/cifparser.hpp"
#include "base/exception.hpp"
#include <sstream>
#include <iostream>
#include "base/utils.hpp"

//
CifParser::CifParser() :
  _ndataBlock(0),
  _dataBlocks()
{
}

//
CifParser::~CifParser() {
  ;
}

//
void CifParser::parse(std::istream& stream) {
  unsigned int nbLines = 0;
  DataBlock *dataBlock = nullptr;
  for ( std::string line ; std::getline(stream,line) ; ) {
    ++nbLines;
    utils::rtrim(line);
    if ( line.size() > 2048 ) // 2048+\n
      throw EXCEPTION(std::string("Line ")+
          utils::to_string(nbLines) + std::string(" has ") +
          utils::to_string(line.size()) + std::string(" characters which is larger than 2048"),
          ERRDIV);
    // Clear coms.
    //std::clog << line << "   >>   ";
    size_t pos_com = line.find_first_of("#");
    if ( pos_com != std::string::npos ) {
      line.resize(pos_com);
    }
    //Put string in stream
    //std::clog << line << "   >>   ";
    std::stringstream lstream(line);
    std::string word;
    lstream >> word;
    utils::tolower(word);
    //std::clog << word << std::endl;
    if ( lstream.fail() ) continue; // nothing to do with this line.
    try { 
      if ( word.size() > 5 && word.substr(0,5).compare("data_") == 0 ) { //datablock
        // Store the previous dataBlock
        if ( dataBlock != nullptr ) {
          _dataBlocks.push_back(std::move(*dataBlock));
          delete dataBlock;
        }
        // Suppose we store the name, the number of tags and dataloops
        dataBlock = new DataBlock(utils::tolower(word.substr(5)));
        ++_ndataBlock;
        //std::clog << "DATABLOCK " <<  dataBlock->_name << std::endl;
      }
      else if ( word.size() > 1 && word[0] == '_' ) { // tag
        if ( dataBlock == nullptr ) 
          throw EXCEPTION(std::string("Not in a data block at line ")
              + utils::to_string(nbLines),ERRABT);
        // increment for this datablock the number of tag
        char value[2048]; // max size authorized
        lstream.read(value,2048);
        std::string sval(value,static_cast<int>(lstream.gcount()));
        //std::clog << "tag " <<  sval << std::endl;
        utils::trim(sval);
        if ( sval.size() == 0 ) throw EXCEPTION(std::string("Missing value for tag ") + word +
            std::string(" at line ") + utils::to_string(nbLines),ERRWAR);
        dataBlock->_tags.insert(std::make_pair(utils::tolower(word.substr(1)),sval));
      }
      else if ( word.size() == 5 && word.compare("loop_") == 0 ) { // dataloop
        if ( dataBlock == nullptr ) 
          throw EXCEPTION(std::string("Not in a data block at line ")
              + utils::to_string(nbLines),ERRABT);
        DataLoop loop;
        // Get header
        //std::clog << "loop " << std::endl;
        int currentpos;
        do {
          currentpos = static_cast<int>(stream.tellg());
          std::getline(stream,line);
          utils::trim(line);
          if ( line[0] != '_' ) break;
          ++nbLines;
          if ( line.size() > 2049 ) // 2048+\n
            throw EXCEPTION(std::string("Line ")+
                utils::to_string(nbLines) + std::string(" has ") +
                utils::to_string(line.size()) + std::string(" characters which is larger than 2048"),
                ERRABT);
          utils::tolower(line);
          //std::clog << "header " << line << std::endl;
          loop._nfield++;
          loop._header.push_back(line.substr(1));
        } while ( !stream.eof() );
        //std::clog << "nfield=" << loop._nfield << std::endl;
        stream.seekg(currentpos);
        //Get data
        while (!stream.eof()) {
          lstream.clear();
          currentpos = static_cast<int>(stream.tellg());
          std::getline(stream,line);
          ++nbLines;
          utils::trim(line);
          if ( line.size() > 2048 ) // 2048+\n
            throw EXCEPTION(std::string("Line ")+
                utils::to_string(nbLines) + std::string(" has ") +
                utils::to_string(line.size()) + std::string(" characters which is larger than 2048"),
                ERRABT);
          //std::clog << "record " << line << std::endl;
          lstream.str(line); 
          lstream.exceptions( std::ios::failbit | std::ios::badbit);
          try {
            std::vector<std::string> entry(loop._nfield);
            for ( unsigned ifield = 0 ; ifield < loop._nfield ; ++ifield) {
              do {
                std::string tmppart;
                lstream >> tmppart;
                entry[ifield]+=tmppart;
              }
              while ( (entry[ifield][0] == '\'' || entry[ifield][0] == '"') && entry[ifield][0] != entry[ifield][entry[ifield].size()-1] && !lstream.fail());
              utils::trim(entry[ifield],"'\"");
            }
            loop._data.push_back(std::move(entry));
            ++loop._nentry;
          }
          catch (std::ios_base::failure e){
            //std::clog << "nentry " << loop._nentry << std::endl;
            stream.seekg(currentpos);
            --nbLines;
            break;
          }
          catch (...) {
            //std::clog << "nentry " << loop._nentry << std::endl;
            stream.seekg(currentpos);
            --nbLines;
            break;
          }
        }
        dataBlock->_dataLoops.push_back(std::move(loop));
      }
      else {
        std::string str_err = std::string("Don't know what to do with ") + word +
          std::string(" at line ") + utils::to_string(nbLines);
        throw EXCEPTION(str_err,ERRDIV);
      }
    }
    catch (Exception& e) {
      if ( e.getReturnValue() == ERRWAR )
        std::cerr << e.fullWhat() << "CIF :: Continuing reading stream" << std::endl;
      else {
        e.ADD("Failed to read CIF stream",e.getReturnValue());
        throw e;
      }
    }
  }
  if ( dataBlock != nullptr ) { // one dataBlock is left
    _dataBlocks.push_back(std::move(*dataBlock));
    delete dataBlock;
  }
}

//
const CifParser::DataBlock CifParser::getDataBlock(const std::string& name) const {
  std::string lowername = utils::tolower(name);
  for ( auto& block : _dataBlocks ) 
    if ( block._name.compare(lowername) == 0 )
      return (const DataBlock)block;
  throw EXCEPTION(std::string("Could not find data block with name ")+name,CifParser::ERFOUND);
}

//
const CifParser::DataBlock CifParser::getDataBlock(unsigned i) const {
  if ( i < _ndataBlock )
    return (const DataBlock)_dataBlocks[i];
  throw EXCEPTION("Data Block out of range",CifParser::ERFOUND);
}


//
unsigned CifParser::DataLoop::getColumn(const std::string& field) const {
  for ( unsigned i = 0; i < _header.size() ; ++i ) 
    if ( _header[i].compare(field) == 0 )
      return i;
  throw EXCEPTION(std::string("In loop_ could not find field ")+field,CifParser::ERFOUND);
}

//
std::string CifParser::DataBlock::getTag(const std::string& tag) {
  std::string value = _tags[utils::tolower(tag)];
  if ( value.empty() ) 
    throw EXCEPTION(std::string("Could not find tag ")+tag+std::string(" in block ")+_name,CifParser::ERFOUND);
  return value;
}

//
const CifParser::DataLoop& CifParser::DataBlock::getDataLoop(const std::string& headerName) const {
  std::string lowerHeaderName = utils::tolower(headerName);
  for ( auto &loop : _dataLoops )
    for ( auto &name : loop._header )
      if ( name == lowerHeaderName )
        return loop;
  throw EXCEPTION("Could not find a DataLoop with header "+headerName,CifParser::ERFOUND);
}


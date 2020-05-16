/**
 * @file configparser.hpp
 *
 * @brief Parser to read an input file
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

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif


#include <iostream>
#include <string>
#include <vector>

/**
 * ConfigParser is a file parser for input/config files
 */
class ConfigParser {

  private :

    bool        _caseSensitive; ///< If the token value is case sensitive or not
    bool        _isParsed;      ///< True if filename is in memory.
    std::string _filename;      ///< Name of the file to read.
    std::string _content;       ///< Content of the file without the comments and force in lower case
    std::string _contentOrig;   ///< Content of the file without the comments with the same case as the file.

    /**
     * Parse a string to check if there is a division.
     * Returns the result of the division
     * @param str The string that might contains a '/' character
     * @return The resultat of the operation
     */
    template<typename T>
    static T parseNumber(std::string& str);

  public :

    static const int ERNAME  = (1<<0); ///< Error number if an input variable is not found.
    static const int ERTYPE  = (1<<1); ///< Error number if the argument type of an input variable
                                       ///  can not be casted.
    static const int ERDIM   = (1<<2); ///< Error number if there is not enough argument for 
                                       ///< an input variable.
    static const int ERFOUND = (1<<3); ///< Error number if the token can not be found.

    enum Characteristic { NONE, ENERGY, LENGTH }; ///< Give a possible dimension to a token

    /**
     * Construct a ConfigParser that will read the input file
     * @param filename The file name of the input file
     */
    ConfigParser(const std::string& filename) _NOEXCEPT ;

    /**
     * Construct an empty ConfigParser that will return nothing
     */
    ConfigParser() _NOEXCEPT : _caseSensitive(false), _isParsed(false), _filename(""),_content(""), _contentOrig("") {;} 

    /**
     * Delete the copy constructor for safety
     */
    ConfigParser(const ConfigParser& cparser) = delete;

    /**
     * Delete the copy operator for safety
     */
    ConfigParser& operator = (const ConfigParser& cparser) = delete;

    /**
     * Destructor of the class.
     * It does nothing for the moment.
     */
    ~ConfigParser();

    /**
     * Set the configparser to read the file as input.
     * It does the same as the first constructor
     * @param filename The filename to read to get parameters.
     * @see ConfigParser()
     */
    void setFile(const std::string& filename) _NOEXCEPT;

    /**
     * Set the configparser content to a string content.
     * @param content The content to use to find token
     */
    void setContent(const std::string& content) _NOEXCEPT;

    /**
     * Set the case sensitivity of the parser
     * @param sensitive true or false
     */
    void setSensitive(const bool sensitive) _NOEXCEPT {_caseSensitive = sensitive;}

    /**
     * Load the entire file in memory removing the comments
     */
    void parse();

    /**
     * Get the corresponding values for an input variable.
     * This is a template function.
     * @param token Name of the input variable
     * @param size Number of elements to return
     * @param dim Characteristic of token. Read a unit and convert the returned values to internal unit according to dim value.
     * @return A vector of size values.
     */
    template<class T>
    std::vector<T> getToken(const std::string& token, const unsigned size, Characteristic dim = NONE) const;

    /**
     * Get the corresponding values for an input variable.
     * This is a template function.
     * @param token Name of the input variable
     * @param dim Characteristic of token. Read a unit and convert the returned value to internal unit according to dim value.
     * @return A vector of size values.
     */
    template<class T>
    T getToken(const std::string& token, Characteristic dim = NONE) const;

    /**
     * Check if the token can be found in the data
     * Usefull for optional token
     * return true if the toekn is found, false otherwise
     */
    bool hasToken(const std::string& token) const;
};

template<>
std::vector<std::string> ConfigParser::getToken(const std::string& token, const unsigned size, Characteristic dim) const;

template<>
std::string ConfigParser::getToken(const std::string& token, Characteristic dim) const;

template<>
bool ConfigParser::getToken(const std::string& token, Characteristic dim) const;

#include "io/configparser.hxx"

#endif // CONFIGPARSER_HPP

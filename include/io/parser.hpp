/**
 * @file parser.hpp
 *
 * @brief Handle the input command line
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

#ifndef PARSER_HPP
#define PARSER_HPP

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

#include "base/exception.hpp"
/**
 * Define a small structure for conveniency.
 * It is redundante with the struct option that will be used later.
 */
typedef struct {
  bool        _isSet;        ///< true if user defines this options.
  std::string _name;         ///< Long name.
  char        _letter;       ///< Short name.
  int         _hasArg;       ///< Do we expect an arguement.
  std::string _value;        ///< Value of the argument.
  std::string _description;  ///< Value of the argument.
} myOption;

/**
 * Parser for the input command line.
 */
class Parser{

  private:

    int         _argc;            ///< Number of input arguments.
    char**      _argv;            ///< List of the input arguments.
    std::string _binary;          ///< Name of the executable.
    char        _empty;           ///< Count for arguments with no shortcut.

    std::vector<myOption> _options; ///< List of all options that can be parsed.



  public:

    static const int ERCAL = (1<<0); ///< Error number returned if wrong input line. 
    static const int EROPT = (1<<1); ///< Error number returned if unknown option.
    static const int ERARG = (1<<2); ///< Error number returned if wrong argument or not usable.


    /**
     * Construct instance from input argc and argv.
     * @param argc number of parameters.
     * @param argv list of parameters.
     */
    Parser(int argc, char **argv);

    /**
     * Copy a parser object.
     * @param parser The object to copy.
     */
    Parser(const Parser& parser) = delete;
    
    /**
     * Copy a parser object.
     * @param parser The object to copy.
     * @return Can not be created.
     */
    Parser& operator = (const Parser& parser) = delete;

    /**
     * Do nothing
     */
    ~Parser();

    /**
     * Process the input command line.
     */
    void parse();

    /**
     * Add an option to the list that can be read from command line
     * This option will have no argument.
     * It is a boolean.
     * @param name Long name that can be used on command line
     * @param letter Short name (a letter) that can be used on command line
     * @param description Brief for the help
     */
    void setOption(std::string name, char letter, const std::string description);

    /**
     * Add an option to the list that can be read from command line
     * This option will have no argument and no shortcut.
     * It is a boolean.
     * @param name Long name that can be used on command line
     * @param description Brief for the help
     */
    void setOption(std::string name, const std::string description);

    /**
     * Add an option to the list that can be read from command line
     * This option will have an argument.
     * @param name Long name that can be used on command line
     * @param letter Short name (a letter) that can be used on command line
     * @param defaultValue Default value as a string no matter the desired type.
     * @param description Brief for the help
     */
    void setOption(std::string name, char letter, std::string defaultValue, const std::string description);

    /**
     * Add an option to the list that can be read from command line
     * This option will have an argument and no shortcut.
     * @param name Long name that can be used on command line
     * @param defaultValue Default value as a string no matter the desired type.
     * @param description Brief for the help
     */
    void setOption(std::string name, std::string defaultValue, const std::string description);

    /**
     * Get the option from command line.
     * @param option Name of the option.
     * @result If option has an argument then return it as template T.
     */
    template <class T>
    T getOption(std::string option);

    /**
     * Get the option from command line.
     * @param option Name of the option.
     * @result If option has no argument, return true if on command line or false.
     */
    bool isSetOption(std::string option);

    /**
     * Print the usage message to ostream.
     * @param out The out stream.
     * @param parser The object to print.
     */
    friend std::ostream& operator << (std::ostream& out, const Parser& parser);
};

template<>
bool Parser::getOption(std::string option);

template<>
std::string Parser::getOption(std::string option);

#include "io/parser.hxx"
#endif // PARSER_HPP

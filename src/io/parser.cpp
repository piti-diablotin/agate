/**
 * @file parser.cpp
 *
 * @brief Implementation of the Parser class
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


#include "io/parser.hpp"
#include <cstdlib>
#include <sstream>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include "getoptwin32.h"
#endif


//
Parser::Parser(int argc, char **argv) :
  _argc(argc),
  _argv(argv),
  _binary(argv[0]),
  _empty(-10),
  _options()
{
}


//
Parser::~Parser(){
}


//
void Parser::parse(){
  int nbOptions = _options.size();
  struct option *options = nullptr;
  std::string forGetopts = ":"; // Just to be safe
  options = new struct option[++nbOptions];

  size_t num = 0;
  for ( auto& opt : _options ){
    if ( opt._letter > 0 ) forGetopts += opt._letter;
    if ( opt._hasArg == 1 ) {
      options[num].name = opt._name.c_str();
      options[num].has_arg = required_argument;
      options[num].flag = nullptr;
      options[num++].val = opt._letter;
      forGetopts += ":";
    }
    else {
      options[num].name = opt._name.c_str();
      options[num].has_arg = no_argument;
      options[num].flag = 0;
      options[num++].val = opt._letter;
    }
  }
  options[num].name = nullptr;
  options[num].has_arg = 0;
  options[num].flag = nullptr;
  options[num].val = 0;


  char c;
  //int opt_num = 0;
  //std::clog << forGetopts << std::endl;

  while ( (c = getopt_long(_argc,_argv,forGetopts.c_str(),options,nullptr)) != -1 ){
    try {
      auto testOpt = _options.begin();
      for ( ; testOpt != _options.end() ; ++testOpt ) {
        if ( testOpt->_letter == c ) { // We've found the option
          testOpt->_value = ( testOpt->_hasArg == 1 ? optarg : "true" );
          testOpt->_isSet = true;
          break;
        }
      }
      if ( testOpt == _options.end() ) {
        switch (c) {
          case ':' : {
                       throw EXCEPTION("Missing argument.",Parser::ERARG);
                       break;
                     }
          default : {
                      throw EXCEPTION("Unknown option.",Parser::EROPT);
                      break;
                    }
        }
      }
    }
    catch ( Exception& e ){
      if ( e.getReturnValue() == Parser::ERARG ) {
        e.ADD("Stop reading command line.",Parser::ERARG);
        throw e;
      }
      else {
        e.ADD("Continueing.",ERRWAR);
        std::clog << e.fullWhat() << std::endl;
      }
    }
  }
  delete[] options;
}


//
void Parser::setOption(std::string name, char letter, const std::string description) {
  _options.push_back({false,name,letter,0,"false",description});
}


//
void Parser::setOption(std::string name, const std::string description) {
  _options.push_back({false,name,--_empty,0,"false",description});
}


//
void Parser::setOption(std::string name, char letter, std::string defaultValue, const std::string description){
  _options.push_back({false,name,letter,1,defaultValue,description});
}


//
void Parser::setOption(std::string name, std::string defaultValue, const std::string description){
  _options.push_back({false,name,--_empty,1,defaultValue,description});
}



/**
 * Get the option from command line convert to a string.
 * @param option Name of the option.
 * @result If option has an argument then return it as a string.
 */
template<>
std::string& Parser::getOption(std::string option){
  for ( auto& testOpt : _options ) {
    if ( testOpt._name == option ) {
      if ( testOpt._hasArg == 1 ) {
        return testOpt._value;
      }
      else {
        std::ostringstream str_tmp;
        str_tmp << "Option " << option << " has no argument.";
        throw EXCEPTION(str_tmp.str(),ERRDIV);
      }
    }
  }
  std::ostringstream str_tmp;
  str_tmp << "Unknown option ";
  str_tmp << option;
  throw EXCEPTION(str_tmp.str(),ERRDIV);
}


/**
 * Get the option from command line convert to a bool.
 * @param option Name of the option.
 * @result If option has an argument then return it as a boolean.
 */
template<>
bool Parser::getOption(std::string option) {
  for ( auto& testOpt : _options ) {
    if ( testOpt._name == option ) {
      if ( testOpt._hasArg == 0 ) {
        return (testOpt._value == "true" ? true : false );
      }
      else {
        std::ostringstream str_tmp;
        str_tmp << "Option " << option << " has an argument.";
        throw EXCEPTION(str_tmp.str(),ERRDIV);
      }
    }
  }
  std::ostringstream str_tmp;
  str_tmp << "Unknown option ";
  str_tmp << option;
  throw EXCEPTION(str_tmp.str(),ERRDIV);
}


//
bool Parser::isSetOption(std::string option) {
  for ( auto& testOpt : _options ) {
    if ( testOpt._name == option ) {
      return testOpt._isSet;
    }
  }
  std::ostringstream str_tmp;
  str_tmp << "Unknown option ";
  str_tmp << option;
  throw EXCEPTION(str_tmp.str(),ERRDIV);
}


//
std::ostream& operator << (std::ostream& out, const Parser& parser) {
  out << "Usage : " << parser._binary << " [-";

  for ( auto& opt : parser._options ) {
    if ( opt._hasArg == 0 && opt._letter > 0 ) {
      out << opt._letter;
    }
  }

  out << "] ";

  for ( auto& opt : parser._options ) {
    if ( opt._hasArg == 0 && opt._letter < 0) {
      out << "[--" << opt._name << "] ";
    }
  }


  for ( auto& opt : parser._options ) {
    if ( opt._hasArg == 1 ) {
      out << "[-" << opt._letter << " argument] ";
    }
  }
  out << std::endl;
  out << std::endl;
  out << "Options are :" << std::endl;
  for ( auto& opt : parser._options ) {
    if ( opt._letter > 0 ) 
      out << "  -" << opt._letter << "  --" << opt._name << " : " 
        << std::endl << "\t" << opt._description << std::endl;
    else
      out << "      --" << opt._name << " : " 
        << std::endl << "\t" << opt._description << std::endl;
  }
  out << std::endl;
  return out;
}

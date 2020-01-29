/**
 * @file configparser.cpp
 *
 * @brief Implementation of the parser to read an input file
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


#include "io/configparser.hpp"
#include "base/exception.hpp"
#include "base/utils.hpp"
#include <fstream>
#include <sstream>


//
ConfigParser::ConfigParser(const std::string& filename) _NOEXCEPT :
  _caseSensitive(false),
  _isParsed(false),
  _filename(filename),
  _content(""),
  _contentOrig("")
{
}


//
ConfigParser::~ConfigParser(){
}

//
void ConfigParser::setFile(const std::string& filename) _NOEXCEPT {
  _filename = filename;
  _content = "";
  _contentOrig = "";
}

//
void ConfigParser::parse() {
  if ( _filename.compare("") == 0 ) return;
  if ( _isParsed ) return;
  std::ifstream inputFile(_filename,std::ios::in);
  if ( !inputFile ) {
    std::string str = "Error opening file ";
    str += _filename;
    throw EXCEPTION(str,ConfigParser::ERNAME);
  }

  unsigned int nbLines = 0;
  for ( std::string line ; std::getline(inputFile,line) ; ) {
    size_t pos_com = line.find_first_of("#");
    if ( pos_com != std::string::npos ) {
      line.resize(pos_com);
    }
    size_t pos_useless;
    while ( (pos_useless = line.find_first_of("=;")) != std::string::npos )
      line.replace(pos_useless,1," ");
    _contentOrig += " " + line + " ";
    utils::tolower(line);
    _content += " " + line + " ";
    ++nbLines;
  }
  std::clog << nbLines << " lines have been read from file " << _filename << std::endl;
  inputFile.close();
  _caseSensitive = true;
  size_t dirseparator = _filename.find_last_of("\\/");
  std::string dirname = (dirseparator == std::string::npos ? "" : _filename.substr(0,dirseparator+1));
  for(;;){
    try {
      ConfigParser cfinclude;
      const std::string newfile(this->getToken<std::string>("include"));
      size_t pos_useless = _content.find("include");
      if ( pos_useless != std::string::npos ) {
        _content.replace(pos_useless,7," ");
        _contentOrig.replace(pos_useless,7," ");
      }
      if ( newfile == _filename ) 
        throw EXCEPTION("Already included file " +_filename,ERRDIV);

      cfinclude.setFile(dirname+utils::trim(newfile,"\"'"));

      cfinclude.parse();
      _content += " " + cfinclude._content;
      _contentOrig += " " + cfinclude._content;
    }
    catch ( Exception &e ) {
      if ( e.getReturnValue() != ERFOUND ) {
        e.ADD("Ignoring file",ERRDIV);
        std::cerr << e.fullWhat() << std::endl;
      }
      else {
        break;
      }
    }
  }
  _caseSensitive = false;
  _isParsed = true;
}

//
void ConfigParser::setContent(const std::string& content) _NOEXCEPT {
  _content = content;
  _contentOrig = content;
  _isParsed = true;

  size_t pos_useless;
  while ( (pos_useless = _content.find_first_of("=;")) != std::string::npos ) {
    _content.replace(pos_useless,1," ");
    _contentOrig.replace(pos_useless,1," ");
  }
  utils::tolower(_content);
  _content += " ";
  _contentOrig += " ";
}

template<>
std::vector<std::string> ConfigParser::getToken(const std::string& token, const unsigned size, Characteristic dim) const {
  (void)(dim);
  const std::string &content = ( _caseSensitive ? _contentOrig : _content );
  size_t pos = _content.find(" "+token+" "); // Add space for safety (ntypat != typat)
  if ( pos == std::string::npos ) {
    std::string str_tmp = "Input token \"" + token + "\" could not be found";
    throw EXCEPTION(str_tmp,ConfigParser::ERFOUND);
  }

  std::istringstream str_data;
  str_data.str(std::move(content.substr(pos)));
  std::string readToken;
  str_data >> readToken;
  std::vector<std::string> rvector(size);
  for ( unsigned i = 0 ; i < size ; ++i ) {
    std::string value;
    std::string str = utils::readString(str_data);
    // Try to convert D exp format to E format
    /* For numbers only !!
    {
      size_t pos(str.find_first_of("Dd"));
      if (pos != std::string::npos) str[pos] = 'e';
    }
    */
    try {
      std::vector<std::string> multiple = utils::explode(str,'*');
      if ( multiple.size() == 0) { // No multiple
        throw EXCEPTION("",1);
      }
      else if ( multiple.size() == 1) { // No multiple
        std::istringstream conv(multiple[0]);
        conv >> value;
        if ( conv.fail() ) throw EXCEPTION("",1);
      }
      else if ( multiple.size() == 2 ) { // We have a multiplication
        unsigned factor;
        std::istringstream conv(multiple[0]);
        conv >> factor;
        if ( conv.fail() ) throw EXCEPTION("",2);
        conv.clear();
        conv.str(multiple[1]);
        conv >> value;
        if ( conv.fail() ) throw EXCEPTION("",1);
        for ( unsigned time = 1 ; time < factor && i < (size-1); ++time ) {
          // factor - 1 insertion since the last on is done at the end.
          rvector[i++]=value;
        }
      }
      else {
        throw EXCEPTION("",2);
      }
    }
    catch ( Exception& e ) {
      std::string str_err("Fail to read ");
      int err = ConfigParser::ERTYPE;
      if ( e.getReturnValue() == 1 ) {
        str_err +=  TypeName<std::string>::get() + " value for token \"" + token + "\"."; 
        if ( i > 0 ) {
          str_err += "\nCould only read " + utils::to_string(i) + " data instead of "
            + utils::to_string(size) + ".";  // long long type for icc to_string
          err |= ConfigParser::ERDIM;
        }
      }
      else if ( e.getReturnValue() == 2 ) {
        str_err +=  "expression for token \"" + token + "\"."; 
      }
      throw EXCEPTION(str_err,err);
    }
    rvector[i] = value;
    /*
       if ( str_data.eof() && i != (size-1)) {
       std::string str_err = "Could only read ";
       str_err += utils::to_string(i);
       str_err += "data for token " + token + " instead of " + utils::to_string(size) + ".";
       throw EXCEPTION(str_err,ConfigParser::ERDIM);
       }
       */
  }
  return rvector;
}


//
template<>
std::string ConfigParser::getToken(const std::string& token, Characteristic dim) const {
  (void)(dim);
  const std::string &content = ( _caseSensitive ? _contentOrig : _content );
  size_t pos = _content.find(" "+token+" ");
  if ( pos == std::string::npos ) {
    std::string str_tmp = "Input token \"" + token + "\" could not be found";
    throw EXCEPTION(str_tmp,ConfigParser::ERFOUND);
  }

  std::istringstream str_data;
  str_data.str(std::move(content.substr(pos)));
  std::string readToken;
  str_data >> readToken;
  std::string rvector = utils::readString(str_data);
  if ( str_data.fail() ) {
    std::string str_err("Fail to read ");
    str_err +=  TypeName<std::string>::get() + " value for token \"" + token + "\"."; 
    throw EXCEPTION(str_err,ConfigParser::ERTYPE);
  }
  /*
     if ( str_data.eof() ) {
     std::string str_err = "Could not read value for token \"";
     str_err += token + "\".";
     throw EXCEPTION(str_err,ConfigParser::ERDIM);
     }
     */
  return rvector;
}

bool ConfigParser::hasToken(const std::string& token) const {
  const std::string &content = ( _caseSensitive ? _contentOrig : _content );
  size_t pos = content.find(" "+token+" ");
  return  pos != std::string::npos;
}

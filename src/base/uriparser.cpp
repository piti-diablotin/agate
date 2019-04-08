/**
 * @file src/./uriparser.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2019 Jordan Bieder
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


#include "base/uriparser.hpp"
#include "base/utils.hpp"
#include <regex>
#include <iostream>

//
UriParser::UriParser() :
  _protocol(Undefined),
  _port(-1),
  _domain(),
  _user(),
  _password(),
  _path()
{
  ;
}

//
UriParser::~UriParser() {
  ;
}


void UriParser::parse(const std::string &uri) {
  std::regex rg("(http|https|sftp|ftp)://(([^/: ]*)(:([^/ :]*))?@)?([^/ :]*)(:([0-9]+))?(/.*)?");
  /** Result should look like this
 	 * 0 [sftp://user:passe@url.ext:22/path]
	 * 1 [sftp]
	 * 2 [user:passe@]
	 * 3 [user]
	 * 4 [:passe]
	 * 5 [passe]
	 * 6 [url.ext]
	 * 7 [:22]
	 * 8 [22]
	 * 9 [/path]
   */
  std::cmatch cm;
  std::regex_match(uri.c_str(),cm,rg);
  if ( cm.size() != 10 ) throw EXCEPTION("Unable to parse URI",ERRDIV);

  std::string proto = cm[1];
  if ( proto == "http" ) _protocol = Http;
  else if ( proto == "https" ) _protocol = Https;
  else if ( proto == "sftp" ) _protocol = Sftp;
  else if ( proto == "ftp" ) _protocol = Ftp;

  _user = cm[3];
  _password = cm[5];
  _domain = cm[6];
  if ( cm[8].length() > 0 ) _port = utils::stoi(cm[8]);
  _path = cm[9];


}

UriParser::Protocol UriParser::getProtocol() const {
  return _protocol;
}

//
int UriParser::getPort() const {
  return _port;
}

//
std::string UriParser::UriParser::getDomain() const {
  return _domain;
}

//
std::string UriParser::getUser() const {
  return _user;
}

//
std::string UriParser::getPassword() const {
  return _password;
}

//
std::string UriParser::getPath() const {
  return _path;
}

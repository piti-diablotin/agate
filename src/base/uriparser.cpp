/**
 * @file src/./uriparser.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2019 Jordan Bieder
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


#include "base/uriparser.hpp"
#include "base/utils.hpp"
#include "base/ssh.hpp"
#include <regex>
#include <iostream>
#include <fstream>
#ifdef HAVE_SSH
#include <libssh/libssh.h>
#endif

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


bool UriParser::parse(const std::string &uri) {
#if GCC_VERSION >= 40900 
  std::regex rg("(http|https|sftp|ftp|scp)://(([^/: ]*)(:([^/ :]*))?@)?([^/ :]*)(:([0-9]+))?(/.*)?");
#else 
  std::regex rg("(http|https|sftp|ftp|scp)://");
#endif

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
  if ( cm.size() == 0 ) return false;
  if ( cm.size() != 10 ) throw EXCEPTION("Unable to parse URI",ERRDIV);

#if GCC_VERSION < 40900 
  throw EXCEPTION("Detected URI but regex support does not allow to parse the URI",ERRDIV);
#endif

  /*
  for (unsigned i = 0 ; i < cm.size() ; ++i ) {
    std::clog << i << " [" << cm[i] << "]" << std::endl;
  }
  */

  std::string proto = cm[1];
  if ( proto == "http" ) _protocol = Http;
  else if ( proto == "https" ) _protocol = Https;
  else if ( proto == "sftp" ) _protocol = Sftp;
  else if ( proto == "ftp" ) _protocol = Ftp;
  else if ( proto == "scp" ) _protocol = Scp;

  _user = cm[3];
  _password = cm[5];
  _domain = cm[6];
  if ( cm[8].length() > 0 ) _port = utils::stoi(cm[8]);
  _path = cm[9];
  return true;
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

std::string UriParser::getFile() const {
  std::string file;
  if ( this->getProtocol() == UriParser::Sftp || this->getProtocol() == UriParser::Scp) {
    Ssh::Protocol proto = (this->getProtocol() == UriParser::Sftp ? Ssh::Sftp : Ssh::Scp);
    Ssh ssh;
    if ( !this->getUser().empty() ) ssh.setUsername(this->getUser());
    if ( !this->getPassword().empty() ) ssh.setPassword(this->getPassword());
    ssh.setHostname(this->getDomain());
    if ( this->getPort() != -1 ) ssh.setPort(this->getPort());
    ssh.connect();
    std::string message;
    bool isOk = ssh.verifyHost(message);
    if ( !isOk ) {
      std::clog << message;
      std::clog << "Are you sure you want to continue connecting (yes/no)? ";
      std::string reply;
      for (;;) {
        std::cin >> reply;
        if ( reply != "yes" && reply != "no" )
          std::clog << "Please type 'yes' or 'no': ";
        else break;
      }
      if ( reply == "yes" ) {
        ssh.validateHost();
      }
      else  {
        std::clog << "Abort" << std::endl;
        return nullptr;;
      }
    }
    try {
      ssh.authenticate();
    }
    catch (Exception &e) {
      int rc = e.getReturnValue();
#ifdef HAVE_SSH
      if ( rc == SSH_AUTH_DENIED ) {
        std::clog << "Password: ";
        std::cin >> message;
        ssh.setPassword(message);
        ssh.authenticate();
      }
      else {
        e.ADD("Wrong authentication",ERRDIV);
        throw e;
      }
#else
        throw e;
#endif
    }
    file = std::string("/tmp/")+*utils::explode(this->getPath(),'/').rbegin();
    std::ofstream tmpfile(file,std::ios::out);
    ssh.getFile(this->getPath(),tmpfile,proto);
    tmpfile.close();
  }
  else {
    throw EXCEPTION("Unmanaged protocole yet",ERRDIV);
  }
  return file;
}

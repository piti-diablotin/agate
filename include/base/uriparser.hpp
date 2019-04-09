/**
 * @file include/base/uriparser.hpp
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


#ifndef URIPARSER_HPP
#define URIPARSER_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <string>

/** 
 *
 */
class UriParser {

  public :
    enum Protocol { Undefined, Http, Https, Sftp, Ftp, Scp };

  private :
      Protocol    _protocol;
      int         _port;
      std::string _domain;
      std::string _user;
      std::string _password;
      std::string _path;

  protected :

  public :

    /**
     * Constructor.
     */
    UriParser();

    /**
     * Destructor.
     */
    virtual ~UriParser();

    bool parse(const std::string &uri);

    Protocol getProtocol() const;
    int getPort() const;
    std::string getDomain() const;
    std::string getUser() const;
    std::string getPassword() const;
    std::string getPath() const;
    std::string getFile() const;
};

#endif  // URIPARSER_HPP

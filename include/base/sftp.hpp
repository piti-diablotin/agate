/**
 * @file include/base/sftp.hpp
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


#ifndef SFTP_HPP
#define SFTP_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <string>

#ifdef HAVE_SSH
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#endif

/** 
 * Small class to list an retrieve file from remote SSH server
 */
class Sftp {

  private :
#ifdef HAVE_SSH
    ssh_session  _sshSession;
    sftp_session _sftpSession;
#endif
    std::string _hostname;
    std::string _user;
    std::string _password;
    int         _port;
    void getFileSFTP(const std::string &filename, std::ostream &destination);
    uint64_t sizeOfFileSFTP(const std::string &filename);
    void getFileSCP(const std::string &filename, std::ostream &destination);
    uint64_t sizeOfFileSCP(const std::string &filename);

  protected :

    int authenticateNone();
    int authenticatePubKey();
    int authenticatePassword();
    int authenticateInteractive();
    void createSftp();

  public :

    /**
     * Constructor.
     */
    Sftp();

    /**
     * Constructor.
     */
    Sftp(const std::string &host, const std::string &user, const std::string &password, int port=22);

    /**
     * Copy constructor
     */
    Sftp(const Sftp& sftp) = delete;

    /**
     * Move constructor
     */
    Sftp(Sftp&& sftp) = delete;

    /**
     * Copy opeartor
     */
    Sftp& operator=(const Sftp &sftp) = delete;

    /**
     * Destructor.
     */
    virtual ~Sftp();

    void setHostname(const std::string &host);
    void setUsername(const std::string &user);
    void setPassword(const std::string &password);
    void setPort(int port);

    void connect();
    void disconnect();
    bool verifyHost(std::string &message);
    void validateHost();
    void authenticate();
    uint64_t sizeOfFile(const std::string &filename);
    void getFile(const std::string &filename, std::ostream &destination);
};

#endif  // SFTP_HPP

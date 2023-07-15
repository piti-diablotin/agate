/**
 * @file include/base/ssh.hpp
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


#ifndef SSH_HPP
#define SSH_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <string>
#include <cstdint>

/** 
 * Small class to list an retrieve file from remote SSH server
 */
struct ssh_session_struct;
struct sftp_session_struct;

class Ssh {

  private :
#ifdef HAVE_SSH
    ssh_session_struct*  _sshSession;
    sftp_session_struct* _sftpSession;
#endif
    std::string _hostname;
    std::string _user;
    std::string _password;
    int         _port;
    void getFileSftp(const std::string &filename, std::ostream &destination);
    uint64_t sizeOfFileSftp(const std::string &filename);
    void getFileScp(const std::string &filename, std::ostream &destination);
    uint64_t sizeOfFileScp(const std::string &filename);

  protected :

    int authenticateNone();
    int authenticatePubKey();
    int authenticatePassword();
    int authenticateInteractive();
    void createSftp();

  public :

    enum Protocol { Sftp, Scp };

    /**
     * Constructor.
     */
    Ssh();

    /**
     * Constructor.
     */
    Ssh(const std::string &host, const std::string &user, const std::string &password, int port=22);

    /**
     * Copy constructor
     */
    Ssh(const Ssh& sftp) = delete;

    /**
     * Move constructor
     */
    Ssh(Ssh&& sftp) = delete;

    /**
     * Copy opeartor
     */
    Ssh& operator=(const Ssh &sftp) = delete;

    /**
     * Destructor.
     */
    virtual ~Ssh();

    void setHostname(const std::string &host);
    void setUsername(const std::string &user);
    void setPassword(const std::string &password);
    void setPort(int port);

    void connect();
    void disconnect();
    bool verifyHost(std::string &message);
    void validateHost();
    void authenticate();
    uint64_t sizeOfFile(const std::string &filename, Protocol proto = Scp);
    void getFile(const std::string &filename, std::ostream &destination, Protocol proto = Scp);
};

#endif  // SSH_HPP

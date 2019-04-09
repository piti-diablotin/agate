/**
 * @file src/base/ssh.cpp
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


#include "base/ssh.hpp"
#include "base/utils.hpp"
#include "base/exception.hpp"
#include <sstream>
#include <fcntl.h>
#include <chrono>
#include <vector>
#include <cmath>
#include <iomanip>

//
Ssh::Ssh() :
#ifdef HAVE_SSH
  _sshSession(ssh_new()),
  _sftpSession(nullptr),
#endif
  _hostname(),
  _user(),
  _password(),
  _port(22)
{
  ;
}

//
Ssh::Ssh(const std::string &host, const std::string &user, const std::string &password, int port) :
#ifdef HAVE_SSH
  _sshSession(ssh_new()),
  _sftpSession(nullptr),
#endif
  _hostname(host),
  _user(user),
  _password(password),
  _port(port)
{
  ;
}

//
Ssh::~Ssh() {
#ifdef HAVE_SSH
  this->disconnect();
#endif
}

//
void Ssh::setHostname(const std::string &host) {
  _hostname = host;
}

//
void Ssh::setUsername(const std::string &user) {
  _user = user;
}

//
void Ssh::setPassword(const std::string &password) {
  _password = password;
}

//
void Ssh::setPort(int port) {
  _port = port;
}

//
void Ssh::connect() {
#ifdef HAVE_SSH
  int error;

  if ( _sshSession == nullptr )
    throw EXCEPTION("Ssh session not allocated",ERRABT);

  ssh_options_set(_sshSession, SSH_OPTIONS_HOST, _hostname.c_str());
  ssh_options_set(_sshSession, SSH_OPTIONS_PORT, &_port);
  ssh_options_set(_sshSession, SSH_OPTIONS_USER, _user.c_str());
  ssh_options_parse_config(_sshSession,nullptr); //nullptr -> default is ~/.ssh/config

  error = ssh_connect(_sshSession);
  if ( error != SSH_OK )
    throw EXCEPTION("Error connecting to ssh server with error:\n"+std::string(ssh_get_error(_sshSession)),ERRABT);
#else
  throw EXCEPTION("SSH support is not activated",ERRDIV);
#endif
}

void Ssh::disconnect() {
#ifdef HAVE_SSH
  if ( _sftpSession != nullptr ) {
    sftp_free(_sftpSession);
    _sftpSession =nullptr;
  }
  if ( _sshSession != nullptr ) {
    ssh_free(_sshSession);
    _sshSession = nullptr;
  }
#else
  throw EXCEPTION("SSH support is not activated",ERRDIV);
#endif
}


bool Ssh::verifyHost(std::string &message) {
#ifdef HAVE_SSH
  //enum ssh_server_known_e state;
  int state;
  unsigned char *hash = nullptr;
  ssh_key srv_pubkey = nullptr;
  size_t hlen;
  char *hexa;
  int rc;
  std::ostringstream str;
  rc = ssh_get_server_publickey(_sshSession, &srv_pubkey);
  if (rc < 0) {
    throw EXCEPTION("Unable to retrieve server public key",ERRDIV);
  }
  rc = ssh_get_publickey_hash(srv_pubkey,
      SSH_PUBLICKEY_HASH_SHA1,
      &hash,
      &hlen);
  ssh_key_free(srv_pubkey);
  if (rc < 0) {
    throw EXCEPTION("Unable to retrieve server public key hash SHA1",ERRDIV);
  }
  //std::string hashStr((char*)(hash));
  //state = ssh_session_is_known_server(_sshSession);
  state = ssh_is_server_known(_sshSession);
  switch (state) {
    //case SSH_KNOWN_HOSTS_OK:
    case SSH_SERVER_KNOWN_OK:
      /* OK */
      break;
      //case SSH_KNOWN_HOSTS_CHANGED:
    case SSH_SERVER_KNOWN_CHANGED:
      str << "Host key for server changed: it is now:" << std::endl;
      hexa = ssh_get_hexa(hash, hlen);
      str << hexa << std::endl
        << "For security reasons, connection will be stopped" << std::endl;
      ssh_string_free_char(hexa);
      ssh_clean_pubkey_hash(&hash);
      message = str.str();
      return false;
      //case SSH_KNOWN_HOSTS_OTHER:
    case SSH_SERVER_FOUND_OTHER:
      str << "The host key for this server was not found but an other"
        << " type of key exists." << std::endl
        << "An attacker might change the default server key to"
        "confuse your client into thinking the key does not exist" << std::endl;
      ssh_clean_pubkey_hash(&hash);
      message = str.str();
      return false;
      //case SSH_KNOWN_HOSTS_NOT_FOUND:
    case SSH_SERVER_FILE_NOT_FOUND:
      str << "Could not find known host file." << std::endl
        << "If you accept the host key here, the file will be"
        "automatically created." << std::endl;
      /* FALL THROUGH to SSH_SERVER_NOT_KNOWN behavior */
      [[fallthrough]];
      //case SSH_SERVER_KNOWN_HOSTS_UNKNOWN:
    case SSH_SERVER_NOT_KNOWN:
      hexa = ssh_get_hexa(hash, hlen);
      str << "The server is unknown. Do you trust the host key?" << std::endl
        << hexa << std::endl;
      //str << utils::base64_encode(hashStr) << std::endl;
      ssh_string_free_char(hexa);
      ssh_clean_pubkey_hash(&hash);

      message = str.str();
      return false;
      //case SSH_KNOWN_HOSTS_ERROR:
    case SSH_SERVER_ERROR:
      ssh_clean_pubkey_hash(&hash);
      throw EXCEPTION("Error during host verification:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
      return false;
  }
  ssh_clean_pubkey_hash(&hash);
  return true;
#else
  throw EXCEPTION("SSH support is not activated",ERRDIV);
  (void) message;
#endif
}
void Ssh::validateHost() {
#ifdef HAVE_SSH
  int rc = ssh_write_knownhost(_sshSession);
  //int rc = ssh_session_update_known_hosti(_sshSession);
  if (rc < 0) 
    throw EXCEPTION("Error during host validation:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
#else
  throw EXCEPTION("SSH support is not activated",ERRDIV);
#endif
}

//
int Ssh::authenticateNone() {
#ifdef HAVE_SSH
  int rc = ssh_userauth_none(_sshSession, _user.c_str());
  return rc;
#else
  throw EXCEPTION("SSH support is not activated",ERRDIV);
  return 0;
#endif
}

//
int Ssh::authenticatePubKey() {
#ifdef HAVE_SSH
  int rc;
  rc = ssh_userauth_publickey_auto(_sshSession, _user.c_str(), _password.c_str());
  if (rc == SSH_AUTH_ERROR) {
    throw EXCEPTION("Authentication failed:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
  }
  return rc;
#else
  throw EXCEPTION("SSH support is not activated",ERRDIV);
  return 0;
#endif
}

//
int Ssh::authenticatePassword() {
#ifdef HAVE_SSH
  int rc;
  rc = ssh_userauth_password(_sshSession, _user.c_str(), _password.c_str());
  if (rc == SSH_AUTH_ERROR)
  {
    throw EXCEPTION("Authentication failed:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
  }
  return rc;
#else
  throw EXCEPTION("SSH support is not activated",ERRDIV);
  return 0;
#endif
}

//
int Ssh::authenticateInteractive() {
#ifdef HAVE_SSH
  return SSH_ERROR;
#else
  throw EXCEPTION("SSH support is not activated",ERRDIV);
  return 0;
#endif
}

//
void Ssh::authenticate() {
#ifdef HAVE_SSH
  int method = 0;
  int rc = 0;
  if ( (rc = this->authenticateNone()) == SSH_OK ) return;
  Exception e;
  method = ssh_userauth_list(_sshSession, _user.c_str());
  if (method & SSH_AUTH_METHOD_NONE) { 
    if ( (rc = this->authenticateNone()) == SSH_OK ) 
      return;
    e.ADD("Authentication None failed:\n"+std::string(ssh_get_error(_sshSession)),rc);
  }
  if (method & SSH_AUTH_METHOD_PUBLICKEY) { 
    if ( (rc = this->authenticatePubKey()) == SSH_OK ) 
      return;
    e.ADD("Authentication PubKey failed:\n"+std::string(ssh_get_error(_sshSession)),rc);
  }
  if (method & SSH_AUTH_METHOD_INTERACTIVE) { 
    if ( (rc = this->authenticateInteractive()) == SSH_OK) 
      return;
    e.ADD("Authentication Interactive failed:\n"+std::string(ssh_get_error(_sshSession)),rc);
  }
  if (method & SSH_AUTH_METHOD_PASSWORD) {
    if ( (rc = this->authenticatePassword()) == SSH_OK ) 
      return;
    e.ADD("Authentication Password failed:\n"+std::string(ssh_get_error(_sshSession)),rc);
  }
  throw e;
#else
  throw EXCEPTION("SSH support is not activated",ERRDIV);
#endif
}

void Ssh::createSftp() {
#ifdef HAVE_SSH
  int rc;
  if (_sftpSession != nullptr) return;
  _sftpSession = sftp_new(_sshSession);
  if ( _sftpSession == nullptr )
    throw EXCEPTION("Error allocating SFTP session:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);

  rc = sftp_init(_sftpSession);
  if (rc != SSH_OK) {
    sftp_free(_sftpSession);
    _sftpSession = nullptr;
    throw EXCEPTION("Error initializing SFTP session: "+utils::to_string(sftp_get_error(_sftpSession)),ERRDIV);
  }
#else
  throw EXCEPTION("SSH support is not activated",ERRDIV);
#endif
}

uint64_t Ssh::sizeOfFileSftp(const std::string &filename) {
#ifdef HAVE_SSH
  this->createSftp();
  int pos = filename.find_last_of("/\\");
  std::string dirname = filename.substr(0,pos);
  std::string filenamename = filename.substr(pos+1);

  sftp_dir dir;
  sftp_attributes attributes;
  //int rc;
  uint64_t size = 0;

  dir = sftp_opendir(_sftpSession, dirname.c_str());
  if ( !dir )
    throw EXCEPTION("Unable to open directory "+dirname,ERRDIV);

  while( (attributes = sftp_readdir(_sftpSession,dir)) != nullptr ) {
    //std::cout << "scanning " << attributes->name << std::endl;
    if ( strcmp(attributes->name, filenamename.c_str()) == 0 ) {
      size = attributes->size;
      sftp_attributes_free(attributes);
      break;
    }
    sftp_attributes_free(attributes);
  }

  if ( sftp_dir_eof(dir) && size == 0 ) {
    /*rc = */sftp_closedir(dir);
    throw EXCEPTION("File " +filename+ " does not exist on the server.",ERRDIV);
  }
  /*rc = */sftp_closedir(dir);
  return size;
#else
  (void) filename;
  throw EXCEPTION("SSH support is not activated",ERRDIV);
  return 0;
#endif
}

//
void Ssh::getFileSftp(const std::string &filename, std::ostream &destination) {
#ifdef HAVE_SSH
  this->createSftp();
  sftp_file file;
  char buffer[1024*1024];
  int nbytes/*, nwritten, rc*/;

  const uint64_t size = this->sizeOfFileSftp(filename);
  uint64_t advancement = 0;

  uint64_t previous = 0;
  uint64_t previousSize = 0;
  auto start = std::chrono::system_clock::now();
  auto printProgress = [&size,&previous,&previousSize,&start](uint64_t current) {
    int actual = (int)(100.*(double)current/size);
    if ( actual > previous ) {
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_seconds = end-start;
      const uint64_t total = current-previousSize;
      double speed = total/elapsed_seconds.count();
      std::string unit("B/s");
      if (speed > 1e9) {speed/=1e9;unit="G"+unit;}
      else if (speed > 1e6) {speed/=1e6;unit="M"+unit;}
      else if (speed > 1e3) {speed/=1e3;unit="k"+unit;}
      std::clog << actual << "% " << std::setprecision(2) << std::fixed << speed << unit << std::endl;
      previousSize = current;
      std::clog.flush();
      start = std::chrono::system_clock::now();
    }
    previous = actual;
  };

  file = sftp_open(_sftpSession, filename.c_str(), O_RDONLY, 0);

  if (file == nullptr) 
    throw EXCEPTION("Can't open file for reading:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);

  if ( !destination )
    throw EXCEPTION("Unable to use output stream",ERRDIV);

  //
  sftp_file_set_nonblocking(file);

  /**
   * Synchrone version
   */
  /*
     for (;;) {
     nbytes = sftp_read(file, buffer,sizeof(buffer));
     if (nbytes == 0) {
     break; // EOF
     } 
     else if (nbytes < 0) {
     sftp_close(file);
     throw EXCEPTION("Error while reading file "+filename+":\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
     }
     destination.write(buffer,nbytes);
     printProgress((advancement+=nbytes));
     if ( !destination ) {
     sftp_close(file);
     throw EXCEPTION("Error writing",ERRDIV);
     }
     }
     */

  /**
   * Asynchrone version
   */
  int async_request = sftp_async_read_begin(file, sizeof(buffer));
  if (async_request >= 0) {
    nbytes = sftp_async_read(file, buffer, sizeof(buffer),
        async_request);
  } else {
    nbytes = -1;
  }
  while (nbytes > 0 || nbytes == SSH_AGAIN) {
    if (nbytes > 0) {
      destination.write(buffer,nbytes);
      printProgress((advancement+=nbytes));
      async_request = sftp_async_read_begin(file, sizeof(buffer));
    } 
    if (async_request >= 0) {
      nbytes = sftp_async_read(file, buffer, sizeof(buffer),
          async_request);
    } else {
      nbytes = -1;
    }
  }

  printProgress((advancement+=nbytes));
  /*rc = */sftp_close(file);
#else
  (void) filename;
  (void) destination;
  throw EXCEPTION("SSH support is not activated",ERRDIV);
#endif
}

//
uint64_t Ssh::sizeOfFileScp(const std::string &filename) {
#ifdef HAVE_SSH
  int rc;
  ssh_scp scp;
  uint64_t size ;

  scp = ssh_scp_new(_sshSession, SSH_SCP_READ, filename.c_str());
  if (scp == nullptr)
    throw EXCEPTION("Error allocating scp session:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);

  rc = ssh_scp_init(scp);
  if (rc != SSH_OK) {
    ssh_scp_free(scp);
    throw EXCEPTION("Error initializing scp session:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
  }

  rc = ssh_scp_pull_request(scp);
  if (rc != SSH_SCP_REQUEST_NEWFILE)
    throw EXCEPTION("Error receiving information about file:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);

  size = ssh_scp_request_get_size(scp);

  ssh_scp_deny_request(scp,"Only quering size");

  ssh_scp_close(scp);
  ssh_scp_free(scp);

  return size;
#else
  (void) filename;
  throw EXCEPTION("SSH support is not activated",ERRDIV);
#endif
}
//
void Ssh::getFileScp(const std::string &filename, std::ostream &destination) {
#ifdef HAVE_SSH
  int rc;
  ssh_scp scp;
  uint64_t size = 0, bufferSize = 0;
  char *buffer = nullptr;
  uint64_t advancement = 0;

  uint64_t previous = 0;
  uint64_t previousSize = 0;
  auto start = std::chrono::system_clock::now();
  auto printProgress = [&size,&previous,&previousSize,&start](uint64_t current) {
    int actual = (int)(100.*(double)current/size);
    if ( actual > previous ) {
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_seconds = end-start;
      const uint64_t total = current-previousSize;
      double speed = total/elapsed_seconds.count();
      std::string unit("B/s");
      if (speed > 1e9) {speed/=1e9;unit="G"+unit;}
      else if (speed > 1e6) {speed/=1e6;unit="M"+unit;}
      else if (speed > 1e3) {speed/=1e3;unit="k"+unit;}
      std::clog << actual << "% " << std::setprecision(2) << std::fixed << speed << unit << std::endl;
      previousSize = current;
      std::clog.flush();
      start = std::chrono::system_clock::now();
    }
    previous = actual;
  };

  scp = ssh_scp_new(_sshSession, SSH_SCP_READ, filename.c_str());
  if (scp == nullptr)
    throw EXCEPTION("Error allocating scp session:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);

  rc = ssh_scp_init(scp);
  if (rc != SSH_OK) {
    ssh_scp_free(scp);
    throw EXCEPTION("Error initializing scp session:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
  }

  rc = ssh_scp_pull_request(scp);
  if (rc != SSH_SCP_REQUEST_NEWFILE)
    throw EXCEPTION("Error receiving information about file:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);

  size = ssh_scp_request_get_size(scp);
  bufferSize = size<1024*1024?size:1024*1024;

  try {
    buffer = new char[bufferSize];
  }
  catch (...) {
    throw EXCEPTION("Error allocating memory", ERRDIV);
  }

  ssh_scp_accept_request(scp);

  do {
    rc = ssh_scp_read(scp, buffer, bufferSize);
    if (rc == SSH_ERROR) {
      delete[] buffer;
      throw EXCEPTION("Error receiving file data:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
    }
    destination.write(buffer, rc);
    printProgress(advancement+=rc);
  } while ( (rc=ssh_scp_pull_request(scp)) != SSH_SCP_REQUEST_EOF);

  delete[] buffer;

  if (rc != SSH_SCP_REQUEST_EOF)
    throw EXCEPTION("Unexpected request:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);

  ssh_scp_close(scp);
  ssh_scp_free(scp);
#else
  (void) filename;
  (void) destination;
  throw EXCEPTION("SSH support is not activated",ERRDIV);
#endif
}

//
uint64_t Ssh::sizeOfFile(const std::string &filename, Protocol proto) {
  switch (proto) {
    case Sftp:
      return this->sizeOfFileSftp(filename);
      break;
    case Scp:
      return this->sizeOfFileScp(filename);
      break;
  }
  return 0;
}

//
void Ssh::getFile(const std::string &filename, std::ostream &destination, Protocol proto) {
  switch (proto) {
    case Sftp:
      this->getFileSftp(filename, destination);
      break;
    case Scp:
      this->getFileScp(filename, destination);
      break;
  }
}

/**
 * @file src/base/sftp.cpp
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


#include "base/sftp.hpp"
#include "base/utils.hpp"
#include "base/exception.hpp"
#include <sstream>

//
Sftp::Sftp() :
  _sshSession(ssh_new()),
  _sftpSession(nullptr),
  _hostname(),
  _user(),
  _password(),
  _port(22)
{
  ;
}

//
Sftp::Sftp(const std::string &host, const std::string &user, const std::string &password, int port) :
  _sshSession(ssh_new()),
  _hostname(host),
  _user(user),
  _password(password),
  _port(port)
{
  ;
}

//
Sftp::~Sftp() {
  if ( _sftpSession != nullptr )
    sftp_free(_sftpSession);
  this->disconnect();
  if ( _sshSession != nullptr )
    ssh_free(_sshSession);
}

//
void Sftp::setHostname(const std::string &host) {
  _hostname = host;
}

//
void Sftp::setUsername(const std::string &user) {
  _user = user;
}

//
void Sftp::setPassword(const std::string &password) {
  _password = password;
}

//
void Sftp::setPort(int port) {
  _port = port;
}

//
void Sftp::connect() {
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
}

void Sftp::disconnect() {
  ssh_disconnect(_sshSession);
}


bool Sftp::verifyHost(std::string &message) {
  //enum ssh_server_known_e state;
  int state;
  unsigned char *hash = nullptr;
  ssh_key srv_pubkey = nullptr;
  size_t hlen;
  char buf[10];
  char *hexa;
  char *p;
  int cmp;
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
}
void Sftp::validateHost() {
  int rc = ssh_write_knownhost(_sshSession);
  //int rc = ssh_session_update_known_hosti(_sshSession);
  if (rc < 0) 
    throw EXCEPTION("Error during host validation:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
}

//
bool Sftp::authenticateNone() {
  int rc = ssh_userauth_none(_sshSession, _user.c_str());
  return rc == SSH_AUTH_SUCCESS;
}

//
bool Sftp::authenticatePubKey() {
  int rc;
  rc = ssh_userauth_publickey_auto(_sshSession, _user.c_str(), _password.c_str());
  if (rc == SSH_AUTH_ERROR) {
    throw EXCEPTION("Authentication failed:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
  }
  return true;
}

//
bool Sftp::authenticatePassword() {
  int rc;
  rc = ssh_userauth_password(_sshSession, _user.c_str(), _password.c_str());
  if (rc == SSH_AUTH_ERROR)
  {
    throw EXCEPTION("Authentication failed:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
  }
  return true;
}

//
bool Sftp::authenticateInteractive() {
}

//
void Sftp::authenticate() {
  int method, rc;
  if ( this->authenticateNone() ) return;
  Exception e;
  method = ssh_userauth_list(_sshSession, _user.c_str());
  if (method & SSH_AUTH_METHOD_NONE) { 
    if ( this->authenticateNone() ) 
      return;
    e.ADD("Authentication None failed:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
  }
  if (method & SSH_AUTH_METHOD_PUBLICKEY) { 
    if ( this->authenticatePubKey() ) 
      return;
    e.ADD("Authentication PubKey failed:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
  }
  if (method & SSH_AUTH_METHOD_INTERACTIVE) { 
    if ( this->authenticateInteractive() ) 
      return;
    e.ADD("Authentication Interactive failed:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
  }
  if (method & SSH_AUTH_METHOD_PASSWORD) {
    if ( this->authenticatePassword() ) 
      return;
    e.ADD("Authentication Password failed:\n"+std::string(ssh_get_error(_sshSession)),ERRDIV);
  }
  e.ADD("Could not get authenticated",ERRDIV);
  throw e;
}

void Sftp::createSftp() {
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
}

int Sftp::sizeOfFile(std::string file) {
  this->createSftp();
  int pos = file.find_last_of("/\\");
  std::string dirname = file.substr(0,pos);
  std::string filename = file.substr(pos+1);

  sftp_dir dir;
  sftp_attributes attributes;
  int rc;
  uint64_t size = 0;

  dir = sftp_opendir(_sftpSession, dirname.c_str());
  if ( !dir )
    throw EXCEPTION("Unable to open directory "+dirname,ERRDIV);

  while( (attributes = sftp_readdir(_sftpSession,dir)) != nullptr ) {
    std::cout << "scanning " << attributes->name << std::endl;
    if ( strcmp(attributes->name, filename.c_str()) == 0 ) {
      size = attributes->size;
      break;
    }
    sftp_attributes_free(attributes);
  }

  if ( sftp_dir_eof(dir) && size == 0 ) {
    rc = sftp_closedir(dir);
    throw EXCEPTION("File " +file+ " does not exist on the server",ERRDIV);
  }
  rc = sftp_closedir(dir);
  return size;

}

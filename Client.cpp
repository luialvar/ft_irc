// Client.cpp
#include "Client.hpp"   // -> declaración de la clase Client

// constructors / destructor
Client::Client();
Client::Client(int fd, const std::string& ip);
Client::~Client();

// getters
int Client::getFd() const;
const std::string& Client::getIp() const;
const std::string& Client::getNickname() const;
const std::string& Client::getUsername() const;
const std::string& Client::getRealname() const;
const std::string& Client::getRecvBuffer() const;
bool Client::hasPassedPassword() const;
bool Client::isRegistered() const;

// setters
void Client::setFd(int fd);
void Client::setIp(const std::string& ip);
void Client::setNickname(const std::string& nickname);
void Client::setUsername(const std::string& username);
void Client::setRealname(const std::string& realname);
void Client::setPassedPassword(bool value);
void Client::setRegistered(bool value);

// buffer helpers
void Client::appendToBuffer(const std::string& data);
void Client::clearBuffer();
void Client::eraseFromBuffer(std::string::size_type count);
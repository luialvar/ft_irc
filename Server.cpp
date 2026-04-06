// Server.cpp
#include "Server.hpp"     // -> declaración de la clase Server

#include <iostream>       // -> std::cout, std::cerr
#include <stdexcept>      // -> std::runtime_error, std::exception
#include <sstream>        // -> std::stringstream
#include <cstring>        // -> memset y utilidades tipo C
#include <cstdlib>        // -> utilidades generales C

#include <unistd.h>       // -> close()
#include <fcntl.h>        // -> fcntl(), O_NONBLOCK
#include <csignal>        // -> señales

#include <sys/socket.h>   // -> socket(), setsockopt(), bind(), listen(), accept(), recv(), send()
#include <sys/types.h>    // -> tipos base de sockets
#include <netinet/in.h>   // -> sockaddr_in, htons(), INADDR_ANY
#include <arpa/inet.h>    // -> inet_ntoa(), inet_addr(), inet_ntop()

bool Server::_signal = false;

// canonical form
Server::Server(int port, const std::string& password);
Server::~Server();

// public
void Server::run();
void Server::signalHandler(int signum);

// socket / poll core
void Server::initServerSocket();
void Server::setupServerAddress();
void Server::addServerSocketToPoll();
void Server::acceptNewClient();
void Server::receiveNewData(int fd);
void Server::closeAllFds();
void Server::removeClient(int fd);

// client lookup
Client* Server::findClientByFd(int fd);
const Client* Server::findClientByFd(int fd) const;

// send / disconnect
void Server::sendMessage(int fd, const std::string& message);
void Server::disconnectClient(int fd, const std::string& reason);

// parsing / buffering
void Server::processClientBuffer(Client& client);
bool Server::extractOneMessage(Client& client, std::string& message);
void Server::handleMessage(Client& client, const std::string& message);

// registration
void Server::tryRegisterClient(Client& client);
bool Server::isClientFullyRegistered(const Client& client) const;
std::string Server::buildWelcomeMessage(const Client& client) const;
bool Server::isNicknameInUse(const std::string& nickname) const;

// command handlers
void Server::handlePass(Client& client, const std::vector<std::string>& tokens);
void Server::handleNick(Client& client, const std::vector<std::string>& tokens);
void Server::handleUser(Client& client, const std::vector<std::string>& tokens);
void Server::handlePing(Client& client, const std::vector<std::string>& tokens);
void Server::handlePong(Client& client, const std::vector<std::string>& tokens);
void Server::handleQuit(Client& client, const std::vector<std::string>& tokens);
void Server::handleJoin(Client& client, const std::vector<std::string>& tokens);
void Server::handlePart(Client& client, const std::vector<std::string>& tokens);
void Server::handlePrivmsg(Client& client, const std::vector<std::string>& tokens);
void Server::handleKick(Client& client, const std::vector<std::string>& tokens);
void Server::handleInvite(Client& client, const std::vector<std::string>& tokens);
void Server::handleTopic(Client& client, const std::vector<std::string>& tokens);
void Server::handleMode(Client& client, const std::vector<std::string>& tokens);

// utils
std::vector<std::string> Server::splitIrcMessage(const std::string& message) const;
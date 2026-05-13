// Server.cpp
#include "Server.hpp"     // -> declaración de la clase Server
#include "includes/ft_irc.hpp"
#include "includes/parser.hpp"
#include "ModeCommand.hpp"
#include "JoinCommand.hpp"
#include "InviteCommand.hpp"

#include <iostream>       // -> std::cout, std::cerr
#include <stdexcept>      // -> std::runtime_error, std::exception
#include <sstream>        // -> std::stringstream
#include <cstring>        // -> memset y utilidades tipo C
#include <cstdlib>        // -> utilidades generales C
#include <cerrno>

#include <unistd.h>       // -> close()
#include <fcntl.h>        // -> fcntl(), O_NONBLOCK
#include <csignal>        // -> señales

#include <sys/socket.h>   // -> socket(), setsockopt(), bind(), listen(), accept(), recv(), send()
#include <sys/types.h>    // -> tipos base de sockets
#include <netinet/in.h>   // -> sockaddr_in, htons(), INADDR_ANY
#include <arpa/inet.h>    // -> inet_ntoa(), inet_addr(), inet_ntop()

volatile sig_atomic_t Server::_signal = false;
// _signal is static because it belongs to the Server class, not one object.
// volatile tells the compiler it can change asynchronously.
// sig_atomic_t is the safe type to modify inside a signal handler.

Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _serverName("irc.local"), _serverSocketFd(-1)
{
	initCommandHandlers();
}

Server::~Server()
{
	closeAllFds();
}

void Server::run()
{
	initServerSocket(); //-> create the server socket

	std::cout  << "Server <" << _serverSocketFd << "> Connected"  << std::endl;
	std::cout << "Waiting to accept a connection...\n";

	while (Server::_signal == false) //-> run the server until the signal is received
	{
		int pollResult = poll(&_fds[0], _fds.size(), -1); //-> wait for an event
		//when it recieves something it goes down to the for
		if (pollResult == -1)
		{
			if (errno == EINTR)
				break;
			throw(std::runtime_error("poll() faild"));
		}
		for (size_t i = 0; i < _fds.size(); ) //-> check all file descriptors
		{
			int currentFd = _fds[i].fd;

			if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				if (currentFd == _serverSocketFd)
					throw(std::runtime_error("server socket poll error"));
				else
				{
					std::cout << "Client <" << currentFd << "> Disconnected" << std::endl;
					removeClient(currentFd);
					close(currentFd);
				}
			}
			else if (_fds[i].revents & POLLIN) //-> check if there is data to read
			{
				if (currentFd == _serverSocketFd)
					acceptNewClient(); //-> accept new client
				else
					receiveNewData(currentFd); //-> receive new data from a registered client
			}
			if (i < _fds.size() && _fds[i].fd == currentFd)
				++i;
		}
	}
	std::cout << std::endl << "Signal Received!" << std::endl;
	closeAllFds(); //-> close the file descriptors when the server stops
}

void Server::signalHandler(int signum)
{
	(void)signum;
	Server::_signal = true; //-> set the static boolean to true to stop the server
}

// public
// socket / poll core
void Server::initServerSocket()
{
	struct sockaddr_in add; //contains important information about the server address
	struct pollfd NewPoll; //used for monitoring file descriptors for I/O events
	//commonly employed with the poll() system call to perform multiplexed I/O

	std::memset(&add, 0, sizeof(add));
	add.sin_family = AF_INET; //-> set the address family to ipv4, so 16 bits directions
	add.sin_port = htons(this->_port); //-> convert the port to network byte order (big endian)
	add.sin_addr.s_addr = INADDR_ANY; //-> set the address to any local machine address
	//you could fix it to only one port

	_serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	 //-> create the server socket
	if(_serverSocketFd == -1) //-> check if the socket is created
		throw(std::runtime_error("faild to create socket"));

	int en = 1;
	if(setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
	//-> set the socket option (SO_REUSEADDR) to reuse the address
	//el servidor puede volver a hacer bind() al mismo puerto casi inmediatamente.
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option
	//(O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(_serverSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) //-> bind
	//the socket to the address
		throw(std::runtime_error("faild to bind socket"));
	if (listen(_serverSocketFd, SOMAXCONN) == -1) //-> listen for incoming connections
	// and making the socket a passive socket
		throw(std::runtime_error("listen() faild"));

	NewPoll.fd = _serverSocketFd; //-> add the server socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	//so events specifies the events to monitor and revents indicates events that occurred
	NewPoll.revents = 0; //-> set the revents to 0
	_fds.push_back(NewPoll); //-> add the server socket to the pollfd
}

void Server::acceptNewClient()
{
	Client cli;
	struct sockaddr_in cliadd;
	struct pollfd newPoll;
	socklen_t len = sizeof(cliadd);

	int clientFd = accept(_serverSocketFd, (struct sockaddr *)&cliadd, &len);
	if (clientFd == -1)
	{
		std::cerr << "accept() failed" << std::endl;
		return;
	}
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl() failed" << std::endl;
		close(clientFd);
		return;
	}
	newPoll.fd = clientFd;
	newPoll.events = POLLIN;
	newPoll.revents = 0;

	cli.setFd(clientFd);
	cli.setIp(inet_ntoa(cliadd.sin_addr));

	_clients.push_back(cli);
	_fds.push_back(newPoll);

	std::cout << "Client <" << clientFd << "> Connected" << std::endl;
}

void Server::receiveNewData(int fd)
{
	char buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));

	ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes > 0)
	{
		buffer[bytes] = '\0';
		std::cout << "Client <" << fd << "> Data: " << buffer;
		Client *client = findClientByFd(fd);
		if (client == 0)
		{
			std::cerr << "Client <" << fd << "> not found" << std::endl;
			removeClient(fd);
			close(fd);
			return;
		}
		client->appendToBuffer(buffer);
		processClientBuffer(*client);
	}
	else if (bytes == 0)
	{
		std::cout << "Client <" << fd << "> Disconnected" << std::endl;
		removeClient(fd);
		close(fd);
	}
	else
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		{
			return;
		}
		std::cerr << "recv() failed for client <" << fd << ">" << std::endl;
		removeClient(fd);
		close(fd);
	}
}

void Server::closeAllFds()
{
	for (std::vector<Client>::size_type i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i].getFd() != -1)
			close(_clients[i].getFd());
	}

	if (_serverSocketFd != -1)
	{
		close(_serverSocketFd);
		_serverSocketFd = -1;
	}
	_fds.clear();
	_clients.clear();
}

void Server::removeClient(int fd)
{
	for (std::vector<pollfd>::size_type i = 0; i < _fds.size(); ++i)
	//-> remove the client from the pollfd
	{
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			break;
		}
	}

	for (std::vector<Client>::size_type i = 0; i < _clients.size(); ++i)
	//-> remove the client from the vector of clients
	{
		if (_clients[i].getFd() == fd)
		{
			_clients.erase(_clients.begin() + i);
			break;
		}
	}
}

Client* Server::findClientByFd(int fd)
{
	for (std::vector<Client>::size_type i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i].getFd() == fd)
			return &_clients[i];
	}
	return 0;
}

const Client* Server::findClientByFd(int fd) const
{
	for (std::vector<Client>::size_type i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i].getFd() == fd)
			return &_clients[i];
	}
	return 0;
}

void Server::initCommandHandlers()
{
	_commandHandlers["PASS"] = &Server::handlePass;
	_commandHandlers["NICK"] = &Server::handleNick;
	_commandHandlers["USER"] = &Server::handleUser;
	_commandHandlers["QUIT"] = &Server::handleQuit;
	_commandHandlers["PING"] = &Server::handlePing;
	_commandHandlers["PONG"] = &Server::handlePong;
	_commandHandlers["JOIN"] = &Server::handleJoin;
	_commandHandlers["PART"] = &Server::handlePart;
	_commandHandlers["PRIVMSG"] = &Server::handlePrivmsg;
	_commandHandlers["KICK"] = &Server::handleKick;
	_commandHandlers["INVITE"] = &Server::handleInvite;
	_commandHandlers["TOPIC"] = &Server::handleTopic;
	_commandHandlers["MODE"] = &Server::handleMode;
}

void Server::add_newChannel(const Channel _channel)
{
	_channels.insert({_channel.getName(), _channel});
}

void Server::processClientBuffer(Client& client)
{
	std::string message;

	while (extractOneMessage(client, message))
	{
		std::cout << "Client <" << client.getFd() << "> Message: "
			<< "'" << message << "'" << std::endl;

		CommandParts parts;
		parseMessage(message, parts);
		executeCommand(client, parts);
	}
}


bool Server::extractOneMessage(Client& client, std::string& message)
{
	std::string::size_type endPos = client.getRecvBuffer().find("\r\n");

	if (endPos == std::string::npos)
		return false;
	message = client.getRecvBuffer().substr(0, endPos);
	client.eraseFromBuffer(endPos + 2);
	return true;
}

void Server::executeCommand(Client& client, const CommandParts& parts)
{
	if (parts.command.empty())
		return;

	std::map<std::string, CommandHandler>::iterator it = _commandHandlers.find(parts.command);

	if (it == _commandHandlers.end())
	{
		// ERR_UNKNOWNCOMMAND (421)
		std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
		sendReply(client, "421 " + nick + " " + parts.command + " :Unknown command");
		return;
	}

	if (!client.isRegistered() && !(parts.command == "PASS" || parts.command == "NICK" || parts.command == "USER" || parts.command == "QUIT"))
	{
		// ERR_NOTREGISTERED (451)
		std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
		sendReply(client, "451 " + nick + " :You have not registered");
		return;
	}

	// Llamar a la función manejadora a través del puntero
	CommandHandler handler = it->second;
	(this->*handler)(client, parts.args);
}

Channel* Server::findChannel(const std::string &name)
{
	std::map<std::string, Channel>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return &(it->second);
	return NULL;
}

Client* Server::findClientByNickname(const std::string &nickname)
{
	size_t i = 0;
	while (i < _clients.size())
	{
		if (_clients[i].getNickname() == nickname)
			return &_clients[i];
		i++;
	}
	return NULL;
}


void Server::sendMessage(int fd, const std::string& message)
{
	std::string full_message = message + "\r\n";
	if (send(fd, full_message.c_str(), full_message.length(), 0) < 0)
	{
		std::cerr << "send() failed for client <" << fd << ">" << std::endl;
	}
}

void Server::sendReply(const Client& client, const std::string& message)
{
	// Formato estándar de respuesta: :<nombre_servidor> <mensaje>
	sendMessage(client.getFd(), ":" + _serverName + " " + message);
}

void Server::handlePing(Client& client, const std::vector<std::string>& tokens)
{
	if (tokens.empty())
	{
		// ERR_NOORIGIN (409)
		sendReply(client, "409 " + client.getNickname() + " :No origin specified");
		return;
	}
	// Responder con PONG. El formato es "PONG <servidor> :<token>"
	sendMessage(client.getFd(), "PONG " + _serverName + " :" + tokens[0]);
}
void Server::handlePong(Client& client, const std::vector<std::string>& tokens)
{
	(void) tokens;
	std::cout<<"cliente: " << client.getUsername() <<std::endl;
}
void Server::handleQuit(Client& client, const std::vector<std::string>& tokens)
{
	(void) tokens;
	std::cout<<"cliente: " << client.getUsername() <<std::endl;
}
void Server::handleJoin(Client& client, const std::vector<std::string>& tokens)
{
	JoinCommand cmd(*this, client, tokens);
	cmd.execute();
}
void Server::handlePart(Client& client, const std::vector<std::string>& tokens)
{
	(void) tokens;
	std::cout<<"cliente: " << client.getUsername() <<std::endl;
}
void Server::handlePrivmsg(Client& client, const std::vector<std::string>& tokens)
{
	(void) tokens;
	std::cout<<"cliente: " << client.getUsername() <<std::endl;
}
void Server::handleKick(Client& client, const std::vector<std::string>& tokens)
{
	(void) tokens;
	std::cout<<"cliente: " << client.getUsername() <<std::endl;
}
void Server::handleInvite(Client& client, const std::vector<std::string>& tokens)
{
	InviteCommand inv(*this, client, tokens);
	inv.execute();
}
void Server::handleTopic(Client& client, const std::vector<std::string>& tokens)
{
	(void) tokens;
	std::cout<<"cliente: " << client.getUsername() <<std::endl;
}
void Server::handleMode(Client& client, const std::vector<std::string>& tokens)
{
	if (tokens.empty())
	{
		sendReply(client, "461 " + client.getNickname() + "MODE :Not enough parameters");
		return;
	}
	if (tokens[0][0] == '#')
	{
		ModeCommand cmd(*this, client, tokens);
		cmd.execute();
	}
}

static std::string serverName()
{
    return "ircserv";
}

static std::string toUpperCommand(const std::string& command)
{
    std::string upper;

    for (std::string::size_type i = 0; i < command.size(); ++i)
    {
        if (command[i] >= 'a' && command[i] <= 'z')
            upper += command[i] - 32;
        else
            upper += command[i];
    }
    return upper;
}

static std::string getReplyTarget(const Client& client)
{
    if (client.getNickname().empty())
        return "*";
    return client.getNickname();
}

static bool isNicknameChar(char c)
{
    if (c >= 'a' && c <= 'z')
        return true;
    if (c >= 'A' && c <= 'Z')
        return true;
    if (c >= '0' && c <= '9')
        return true;
    if (c == '[' || c == ']' || c == '{' || c == '}')
        return true;
    if (c == '\\' || c == '|')
        return true;
    return false;
}

static bool isValidNickname(const std::string& nickname)
{
    if (nickname.empty())
        return false;
    if (nickname[0] == '#' || nickname[0] == ':')
        return false;
    for (std::string::size_type i = 0; i < nickname.size(); ++i)
    {
        if (!isNicknameChar(nickname[i]))
            return false;
    }
    return true;
}

void Server::handlePass(Client& client, const std::vector<std::string>& tokens)
{
    if (client.isRegistered())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 462 "
            + getReplyTarget(client) + " :You may not reregister");
        return;
    }
    if (tokens.empty() || tokens[0].empty())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 461 "
            + getReplyTarget(client) + " PASS :Not enough parameters");
        return;
    }
    if (tokens[0] != _password)
    {
        client.setPassedPassword(false);
        sendMessage(client.getFd(), ":" + serverName() + " 464 "
            + getReplyTarget(client) + " :Password incorrect");
        return;
    }
    client.setPassedPassword(true);
    tryRegisterClient(client);
}

void Server::handleNick(Client& client, const std::vector<std::string>& tokens)
{
    std::string oldNickname = client.getNickname();

    if (tokens.empty() || tokens[0].empty())
    {
        sendMessage(client.getFd(), ":" + serverName()
            + " 431 * :No nickname given");
        return;
    }
    if (!client.hasPassedPassword())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 451 "
            + getReplyTarget(client) + " :You have not registered");
        return;
    }
    if (!isValidNickname(tokens[0]))
    {
        sendMessage(client.getFd(), ":" + serverName() + " 432 "
            + getReplyTarget(client) + " " + tokens[0]
            + " :Erroneous nickname");
        return;
    }
    if (toUpperCommand(tokens[0]) != toUpperCommand(oldNickname)
        && isNicknameInUse(tokens[0]))
    {
        sendMessage(client.getFd(), ":" + serverName() + " 433 "
            + getReplyTarget(client) + " " + tokens[0]
            + " :Nickname is already in use");
        return;
    }
    client.setNickname(tokens[0]);
    if (client.isRegistered())
    {
        sendMessage(client.getFd(), ":" + oldNickname + "!"
            + client.getUsername() + "@" + client.getIp()
            + " NICK " + tokens[0]);
    }
    else
        tryRegisterClient(client);
}

void Server::handleUser(Client& client, const std::vector<std::string>& tokens)
{
    if (client.isRegistered())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 462 "
            + getReplyTarget(client) + " :You may not reregister");
        return;
    }
    if (!client.hasPassedPassword())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 451 "
            + getReplyTarget(client) + " :You have not registered");
        return;
    }
    if (tokens.size() < 4 || tokens[0].empty())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 461 "
            + getReplyTarget(client) + " USER :Not enough parameters");
        return;
    }
    client.setUsername(tokens[0]);
    client.setRealname(tokens[3]);
    tryRegisterClient(client);
}

void Server::tryRegisterClient(Client& client)
{
    if (client.isRegistered())
        return;
    if (!isClientFullyRegistered(client))
        return;
    client.setRegistered(true);
    sendMessage(client.getFd(), buildWelcomeMessage(client));
}

bool Server::isClientFullyRegistered(const Client& client) const
{
    if (!client.hasPassedPassword())
        return false;
    if (client.getNickname().empty())
        return false;
    if (client.getUsername().empty())
        return false;
    return true;
}

std::string Server::buildWelcomeMessage(const Client& client) const
{
    return ":" + serverName() + " 001 " + client.getNickname()
        + " :Welcome to the Internet Relay Network "
        + client.getNickname() + "!" + client.getUsername()
        + "@" + client.getIp();
}

bool Server::isNicknameInUse(const std::string& nickname) const
{
    std::string wanted = toUpperCommand(nickname);

    for (std::vector<Client>::size_type i = 0; i < _clients.size(); ++i)
    {
        if (toUpperCommand(_clients[i].getNickname()) == wanted)
            return true;
    }
    return false;
}

// Server.cpp
#include "Server.hpp"     // -> declaración de la clase Server

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
	: _port(port), _password(password), _serverSocketFd(-1)
{
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

void Server::processClientBuffer(Client& client)
{
	std::string message;

	while (extractOneMessage(client, message))
	{
		std::cout << "Client <" << client.getFd() << "> Message: "
			<< message << std::endl;
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

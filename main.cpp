// main.cpp
#include "Server.hpp"   // -> clase Server
#include <iostream>     // -> std::cout, std::cerr
#include <sstream>      // -> std::stringstream
#include <string>       // -> std::string
#include <csignal>      // -> std::signal, SIGINT, SIGQUIT

static bool isDigits(const std::string& str)
{
	if (str.empty())
		return false;

	for (std::string::size_type i = 0; i < str.size(); ++i)
	{
		if (str[i] < '0' || str[i] > '9')
			return false;
	}
	return true;
}

static bool parsePort(const std::string& str, int& port)
{
	std::stringstream ss;

	if (!isDigits(str))
		return false;

	ss << str;
	ss >> port;

	if (ss.fail() || !ss.eof())
		return false;
	//maybe this could be better: if (port < 1024 || port > 49151)
	if (port < 1024 || port > 65535)
		return false;
	return true;
}

int main(int argc, char **argv)
{
	int port;
	std::string password;

	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	if (!parsePort(argv[1], port))
	{
		std::cerr << "Error: invalid port" << std::endl;
		return 1;
	}

	password = argv[2];
	if (password.empty())
	{
		std::cerr << "Error: password cannot be empty" << std::endl;
		return 1;
	}

	try
	{
			//(ctrl + c) or (ctrl + \)
			std::signal(SIGINT, Server::signalHandler);
			std::signal(SIGQUIT, Server::signalHandler);
			std::signal(SIGPIPE, SIG_IGN);
			//esto arregla que si intentamos escribir en un socket 
			//cuya conexión ya está cerrada, no termines todo el servidor

			Server server(port, password);
		server.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

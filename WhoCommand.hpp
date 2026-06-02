#ifndef WHOCOMMAND_HPP
#define	WHOCOMMAND_HPP

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

class WhoCommand
{
	private:
		Server&	_server;
		Client&	_client;
		const std::vector<std::string>&	_args;
		Channel	*_channel;

		bool parse();
		void sendInfo();



	public:
		WhoCommand(Server &server, Client &client, const std::vector<std::string> &args);
		void execute();
		~WhoCommand();
};

#endif
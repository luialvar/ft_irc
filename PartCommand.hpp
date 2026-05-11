#ifndef PARTCOMMAND_HPP
#define	PARTCOMMAND_HPP

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

class PartCommand
{
	private:
		Server&	_server;
		Client&	_client;
		const std::vector<std::string>&	_args;
		Channel	*_channel;
		std::vector<std::string> _channels;

		bool parse(std::string _channel_it);

	public:
		PartCommand(Server &server, Client &client, const std::vector<std::string> &args);
		void	execute();
		~PartCommand();
};

#endif
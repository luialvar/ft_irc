#ifndef JOINCOMMAND_HPP
#define	JOINCOMMAND_HPP
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"


class JoinCommand
{
	private:
		Server&	_server;
		Client&	_client;
		const std::vector<std::string>&	_args;
		Channel	*_channel;
		std::vector<std::string> _channels;
		std::vector<std::string> _keys;

		bool	parse(std::string _channel_it);
		bool	checkModesAndConditions(std::string _key_it);
		void	createAndJoin();

	public:
		JoinCommand(Server &server, Client &client, const std::vector<std::string> &args);
		void	execute();
		~JoinCommand();
};

#endif
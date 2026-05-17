#ifndef PRIVMSGCOMMAND_HPP
#define	PRIVMSGCOMMAND_HPP

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

class PrivmsgCommand
{
	private:
		Server&	_server;
		Client&	_client;
		const std::vector<std::string>&	_args;
		Channel	*_channel;
		std::vector<std::string> _targets;

		bool parse(std::string _channel_it);
        bool msgChannel(Channel &_targetChannel);
        bool msgUser(Client &_targetUser);

	public:
		PrivmsgCommand(Server &server, Client &client, const std::vector<std::string> &args);
		void execute();
		~PrivmsgCommand();
};

#endif
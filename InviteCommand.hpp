#ifndef INVITECOMMAND_HPP
#define	INVITECOMMAND_HPP

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

class InviteCommand
{
	private:
		Server&	_server;
		Client&	_client;
		Client*	_target;
		const std::vector<std::string>&	_args;
		Channel	*_channel;

		bool parseAndChecks();

	public:
		InviteCommand(Server &server, Client &client, const std::vector<std::string> &args);
		void	execute();
		~InviteCommand();
};

#endif
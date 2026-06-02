#ifndef KICKCOMMAND_HPP
#define	KICKCOMMAND_HPP
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"


class KickCommand
{
	private:
		Server&	_server;
		Client&	_client;
		const std::vector<std::string>&	_args;
		Channel	*_channel;
		std::vector<std::string> _kickClients;
        std::string _reason;

		bool	parse();
        void    checkForKick(std::string _clientToKick);

	public:
		KickCommand(Server &server, Client &client, const std::vector<std::string> &args);
		void	execute();
		~KickCommand();
};

#endif
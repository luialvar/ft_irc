#ifndef TOPICCOMMAND_HPP
#define	TOPICCOMMAND_HPP

#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

class TopicCommand
{
	private:
		Server&	_server;
		Client&	_client;
		const std::vector<std::string>&	_args;
		Channel	*_channel;

		bool parse();
		void checkOrChangeTopic();



	public:
		TopicCommand(Server &server, Client &client, const std::vector<std::string> &args);
		void execute();
		~TopicCommand();
};

#endif
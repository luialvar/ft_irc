#include "Server.hpp"
#include "includes/parser.hpp"
#include "ModeCommand.hpp"
#include "JoinCommand.hpp"
#include "InviteCommand.hpp"
#include "PartCommand.hpp"
#include "PrivmsgCommand.hpp"
#include "TopicCommand.hpp"
#include "KickCommand.hpp"
#include "WhoCommand.hpp"
#include "QuitCommand.hpp"

#include <iostream>
#include <unistd.h>

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
	_commandHandlers["WHO"] = &Server::handleWho;
	_commandHandlers["CAP"] = &Server::handleCap;
}

void Server::add_newChannel(const Channel _channel)
{
	_channels.insert(std::make_pair(_channel.getName(), _channel));
}

void Server::remove_Channel(const Channel _channel)
{
	_channels.erase(_channel.getName());
}

void Server::processClientBuffer(Client& client)
{
	std::string message;

	while (extractOneMessage(client, message))
	{
		std::cout << "Client <" << client.getFd() << "> Message: "
			<< "'" << message << "'" << std::endl << std::endl;

		CommandParts parts;
		//luialvar
		int clientFd = client.getFd();
		//luialvar
		parseMessage(message, parts);
		executeCommand(client, parts);
		//luialvar
		if (findClientByFd(clientFd) == 0)
			return;
		//luialvar
	}
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
	for (std::list<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->getNickname() == nickname)
			return &(*it);
	}
	return NULL;
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
	QuitCommand quit(*this, client, tokens);
	quit.execute();
}
void Server::handleJoin(Client& client, const std::vector<std::string>& tokens)
{
	JoinCommand cmd(*this, client, tokens);
	cmd.execute();
}
void Server::handlePart(Client& client, const std::vector<std::string>& tokens)
{
	PartCommand part(*this, client, tokens);
	part.execute();
}
void Server::handlePrivmsg(Client& client, const std::vector<std::string>& tokens)
{
	PrivmsgCommand msg(*this, client, tokens);
	msg.execute();
}
void Server::handleKick(Client& client, const std::vector<std::string>& tokens)
{
	KickCommand kick(*this, client, tokens);
	kick.execute();
}
void Server::handleInvite(Client& client, const std::vector<std::string>& tokens)
{
	InviteCommand inv(*this, client, tokens);
	inv.execute();
}
void Server::handleTopic(Client& client, const std::vector<std::string>& tokens)
{
	TopicCommand topic(*this, client, tokens);
	topic.execute();
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

void Server::handleWho(Client& client, const std::vector<std::string>& tokens)
{
	WhoCommand who(*this, client, tokens);
	who.execute();
}

void Server::handleCap(Client& client, const std::vector<std::string>& tokens)
{
	if (tokens.empty())
		return;
	if (tokens[0] == "LS")
		sendReply(client, " CAP * LS :");
	else if(tokens[0] == "END")
		return;
}

//Saca al cliente de todos los canales en los que esté y manda un mensaje
// si la función la llama QUIT o ejecuta un PART si lo llama JOIN
void Server::smokeGrenade(Client& _client, const std::string& _command, const std::string& _reason)
{
	std::map<std::string, Channel>::iterator it;
	std::set<Client*> _toNotify;
	std::vector<Client*> _clientsInChannel;
	std::vector<std::string> _channelsJoined;

	//Guarda todos los canales en los que está unido el usuario
	for (it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->second.hasClient(&_client))
			_channelsJoined.push_back(it->second.getName());
	}

	//Si llama un comando JOIN se encarga de hacer un PART por cada canal al que esté unido el usuario
	if (_command == "JOIN")
	{
		for (size_t i = 0; i < _channelsJoined.size(); i++)
		{
			std::vector<std::string> tokens;
			tokens.push_back(_channelsJoined[i]);

			handlePart(_client, tokens);
		}
		return;
	}

	//Prepara el mensaje de notificación a los clientes que comparten canal con el usuario
	std::string str = ":" + _client.getNickname() +
						"!" + _client.getUsername() +
						 "@" + getServerName() +
						  " QUIT :" + _reason;

	if(_command == "QUIT")
	{	
		Channel* _ch;
		for (size_t i = 0; i < _channelsJoined.size(); i++)
		{
			//Guarda el canal y sus clientes sobre el que va a trabajar esta iteración
			_ch = findChannel(_channelsJoined[i]);
			if (_ch == NULL)
				continue;
			_clientsInChannel = _ch->getClients();

			//Elimina al cliente del canal actual
			_ch->removeClient(&_client);

			//Elimina el canal/nombra otro operador si es necesario
			if (_ch->getClientCount() == 0)
				remove_Channel(*_ch);
			else if (_ch->isOperator(&_client))
			{
				_ch->removeOperator(&_client);
				_ch->addOperator(_ch->getClients().front());
				for(int i = 0; i < (int)_ch->getClientCount(); i++)
					sendReply(*_ch->getClients()[i], "MODE " + _ch->getName() + " +o " + _ch->getClients().front()->getNickname());
			}

			//Notifica a los clientes que comparte canal con el usuario que ha usado QUIT y 
			//los añade a la lista de notificados para no duplicar mensajes
			for(int j = 0; j < (int)_clientsInChannel.size(); j++)
			{
				if (_clientsInChannel[j] != &_client && _toNotify.insert(_clientsInChannel[j]).second)
					sendMessage(_clientsInChannel[j]->getFd(), str);
			}
		}
		int _clientFd = _client.getFd();
		removeClient(_clientFd);
		close(_clientFd);
	}
}

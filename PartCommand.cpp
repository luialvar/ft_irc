#include "PartCommand.hpp"
#include "utils.hpp"
#include <sstream>
#include <sys/socket.h>

static std::string formatError(int code, const std::string& nick, const std::string& arg1, const std::string& arg2) {

	std::stringstream ss;
    ss << code << " " << nick << " ";

	(void) arg2;
    switch (code) {
        case 403: // ERR_NOSUCHCHANNEL
            ss << arg1 << " :No such channel";
            break;
		case 442: // ERR_NOTONCHANNEL
			ss << arg1 << " :You're not on that channel";
			break;
        case 461: // ERR_NEEDMOREPARAMS
            ss << arg1 << " :Not enough parameters";
            break;
        default:
            ss << ":Unknown error";
            break;
    }
    return ss.str();
}

PartCommand::PartCommand(Server &server, Client &client, const std::vector<std::string> &args): _server(server), _client(client), _args(args), _channel(NULL)
{}

PartCommand::~PartCommand(){}

void	PartCommand::execute()
{
	if (_args.empty())
	{
		//llamada a funcion error ERR_NEEDMOREPARAMS
		_server.sendReply(_client, formatError(461, _client.getNickname(), "PART", ""));
		return;
	}
	_channels = split(_args[0], ',');
	for(int i = 0; i < (int)_channels.size(); i++)
	{
		if (!parse(_channels[i]))
			continue;
			
		//Borrar el cliente del canal
		_channel->removeClient(&_client);
		
		//Comando para HexChat y mensaje para todos los clientes del canal
		std::string str = ":" + _client.getNickname() + "!" + _client.getUsername() + "@" + _server.getServerName() + " PART :" + _channel->getName() + "\r\n";
		send(_client.getFd(), str.c_str(), str.length(), 0);
		for(int j = 0; j < (int)_channel->getClientCount(); j++)
		{
			int fd = _channel->getClients()[j]->getFd();
			send(fd, str.c_str(), str.length(), 0);
		}

		//Comprobar si queda alguien en el canal por si hay que eliminar dicho canal o nombrar otro operador en el caso que este lo fuese
		if (_channel->getClientCount() == 0)
			_server.remove_Channel(*_channel);
		else if (_channel->isOperator(&_client))
		{
			_channel->removeOperator(&_client);
			_channel->addOperator(_channel->getClients().front());
		}
	}
}

bool	PartCommand::parse(std::string _channel_it)
{
	_channel = _server.findChannel(_channel_it);
	if (_channel == NULL)
	{
		//llamada a funcion error ERR_NOSUCHCHANNEL
		_server.sendReply(_client, formatError(403, _client.getNickname(), _channel_it, ""));
		return false;
	}

	if (!(_channel->hasClient(&_client)))
	{
		//llamada a funcion error ERR_NOTONCHANNEL
		_server.sendReply(_client, formatError(442, _client.getNickname(), _channel_it, _channel->getName()));
		return false;
	}
	return true;
}

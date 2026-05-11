#include "PartCommand.hpp"
#include "utils.cpp"
#include <sstream>

static std::string formatError(int code, const std::string& nick, const std::string& arg1, const std::string& arg2) {
    
	std::stringstream ss;
    ss << code << " " << nick << " ";

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
		return;;
	}
	_channels = split(_args[0], ',');
	for(int i = 0; i < _channels.size(); i++)
	{
		if (!parse(_channels[i]))
			return;
		_channel->removeClient(&_client);
		_server.sendReply(_client, _client.getUsername() + " is leaving the channel " + _channels[i]);
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
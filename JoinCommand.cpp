#include "JoinCommand.hpp"
#include "utils.cpp"
#include <sstream>

static std::string formatError(int code, const std::string& nick, const std::string& arg1, const std::string& arg2) {

	std::stringstream ss;
    ss << code << " " << nick << " ";

	(void)arg2;
    switch (code) {
        case 403: // ERR_NOSUCHCHANNEL
            ss << arg1 << " :No such channel";
            break;
	    case 407: // ERR_BADCHANMASK
            ss << arg1 << " :Bad Channel Mask";
            break;
        case 443: // ERR_USERONCHANNEL
            ss << arg1 << " :is already on channel";
            break;
        case 461: // ERR_NEEDMOREPARAMS
            ss << arg1 << " :Not enough parameters";
            break;
	    case 471: // ERR_CHANNELISFULL
    		ss << arg1 << " :Cannot join channel (+l)";
            break;
		case 473: // ERR_INVITEONLYCHAN
    		ss << arg1 << " :Cannot join channel (+i)";
            break;
	    case 475: // ERR_BADCHANNELKEY
    		ss << arg1 << " :Cannot join channel (+k)";
            break;
        case 482: // ERR_CHANOPRIVSNEEDED
            ss << arg1 << " :You're not channel operator";
            break;
        default:
            ss << ":Unknown error";
            break;
    }
    return ss.str();
}

JoinCommand::JoinCommand(Server &server, Client &client, const std::vector<std::string> &args): _server(server), _client(client), _args(args), _channel(NULL)
{}

JoinCommand::~JoinCommand(){}

void JoinCommand::execute()
{
	if (_args.empty())
	{
		//llamada a funcion error ERR_NEEDMOREPARAMS
		_server.sendReply(_client, formatError(461, _client.getNickname(), "JOIN", ""));
		return;
	}
	//Añadido los vectors para los casos en los que pueden llegar varios canales/keys
	_channels = split(_args[0], ',');
	_keys = split(_args[1], ',');
	std::string	_aux_key = "";
	for(int i = 0; i < (int)_channels.size(); i++)
	{
		if (!parse(_channels[i]))
			return;
		if (!_keys.empty())
			_aux_key = _keys[i];
		if ((!_channels.empty()) && !checkModesAndConditions(_aux_key))
			return;
		_channel->addClient(&_client);
		_server.sendReply(_client, _client.getNickname() + " is joining the channel " + _channel->getName());
	}
}

bool	JoinCommand::parse(std::string _channel_it)
{
	if (_args[0][0] != '#')
	{
		//llamada a funcion error ERR_BADCHANMASK
		_server.sendReply(_client, formatError(407, _client.getNickname(),_channel_it, ""));
		return false;
	}

	_channel = _server.findChannel(_channel_it);
	if (_channel == NULL)
	{
		//llamada a funcion error ERR_NOSUCHCHANNEL
		_server.sendReply(_client, formatError(403, _client.getNickname(), _channel_it, ""));
		createAndJoin(_channel_it);
		return false;
	}

	if (_channel->hasClient(&_client))
	{
		_server.sendReply(_client, formatError(443, _client.getNickname(), _channel_it, _channel->getName()));
		return false;
	}
	return true;
}

bool	JoinCommand::checkModesAndConditions(std::string _key_it)
{

	if (_channel->isModeSet('l') && (_channel->getUserLimit() == _channel->getClientCount()))
	{
		//llamada a funcion error ERR_CHANNELISFULL
		_server.sendReply(_client, formatError(471, _client.getNickname(), _channel->getName(), ""));
		return false;
	}

	if (_channel->isModeSet('k') && _key_it != _channel->getKey())
	{
		//llamada a funcion error ERR_BADCHANNELKEY
		_server.sendReply(_client, formatError(475, _client.getNickname(), _channel->getName(), ""));
		return false;
	}

	if (_channel->isModeSet('i') && !(_channel->isInvited(&_client)))
	{
		//llamada a funcion error ERR_INVITEONLYCHAN
		_server.sendReply(_client, formatError(473, _client.getNickname(),_channel->getName(), ""));
		return false;
	}
	return true;
}

void	JoinCommand::createAndJoin(std::string _channelName)
{
	Channel _channel(_channelName);
	_channel.addClient(&_client);
	_channel.addOperator(&_client);
	_server.add_newChannel(_channel);
	_server.sendReply(_client, _client.getNickname() + " is joining the channel " + _channel.getName());
}

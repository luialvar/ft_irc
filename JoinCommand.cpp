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

	_channels = split(_args[0], ',');
	if (_args.size() > 1 && !_args[1].empty())
		_keys = split(_args[1], ',');

	for(size_t i = 0; i < _channels.size(); i++)
	{
		const std::string& channelName = _channels[i];
		std::string key = (i < _keys.size()) ? _keys[i] : "";

		if (channelName.empty() || channelName[0] != '#')
		{
			_server.sendReply(_client, formatError(407, _client.getNickname(), channelName, ""));
			continue;
		}

		_channel = _server.findChannel(channelName);

		if (_channel == NULL)
		{
			Channel newChannel(channelName);
			_server.add_newChannel(newChannel);
			_channel = _server.findChannel(channelName);

			if (!_channel) continue;

			if (!key.empty())
			{
				_channel->setKey(key);
				_channel->setMode('k');
			}
			_channel->addClient(&_client);
			_channel->addOperator(&_client);
		}
		else
		{
			if (_channel->hasClient(&_client))
			{
				_server.sendReply(_client, formatError(443, _client.getNickname(), channelName, _client.getNickname()));
				continue;
			}
			if (!checkModesAndConditions(key))
				continue;
			_channel->addClient(&_client);
		}

		std::string joinMsg = ":" + _client.getNickname() + "!" + _client.getUsername() + "@" + _client.getIp() + " JOIN :" + _channel->getName();
		_channel->broadcastMessage(joinMsg, _server, NULL);

		if (!_channel->getTopic().empty())
			_server.sendReply(_client, "332 " + _client.getNickname() + " " + _channel->getName() + " :" + _channel->getTopic());
		else
			_server.sendReply(_client, "331 " + _client.getNickname() + " " + _channel->getName() + " :No topic is set");

		_server.sendReply(_client, "353 " + _client.getNickname() + " = " + _channel->getName() + " :" + _channel->getUserListString());
		_server.sendReply(_client, "366 " + _client.getNickname() + " " + _channel->getName() + " :End of /NAMES list.");
	}
}

bool	JoinCommand::checkModesAndConditions(std::string _key_it)
{
	if (_channel->isModeSet('l') && _channel->getUserLimit() > 0 && _channel->getClientCount() >= _channel->getUserLimit())
	{
		//llamada a funcion error ERR_CHANNELISFULL
		_server.sendReply(_client, formatError(471, _client.getNickname(), _channel->getName(), ""));
		return false;
	}
	if (_channel->isModeSet('k') && !_channel->checkKey(_key_it))
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

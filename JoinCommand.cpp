#include "JoinCommand.hpp"
#include "utils.hpp"
#include <sstream>
#include <sys/socket.h>

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
	if (_args.size() == 2)
		_keys = splitKeys(_args[1]);
	std::string	_aux_key = "";

	for(int i = 0; i < (int)_channels.size(); i++)
	{
		if (_channels[i] == "0")
		{
			_server.smokeGrenade(_client, "JOIN", "");
			return;
		}
	}

	for(int i = 0; i < (int)_channels.size(); i++)
	{
		if (!parse(_channels[i]))
			continue;
		if (!_keys.empty() && i < (int)_keys.size())
			_aux_key = _keys[i];
		else
			_aux_key = "";
		if ((!_channels.empty()) && !checkModesAndConditions(_aux_key))
			continue;
		
		_channel->addClient(&_client);

		std::string str = 
			":" +
			_client.getNickname() +
			"!" +
			_client.getUsername() +
			"@" +
			_server.getServerName() +
			" JOIN :" +
			_channel->getName() +
			"\r\n";

		//Recorrer todos los clientes del canal y mandarles el mensaje
			for(int i = 0; i < (int)_channel->getClientCount(); i++)
			{
				int fd = _channel->getClients()[i]->getFd();
				//luialvar
				_server.sendMessage(fd, str);
				//luialvar
			}

		sendReplies();
	}
}

bool	JoinCommand::parse(std::string _channel_it)
{
	if (_channel_it[0] != '#')
	{
		//llamada a funcion error ERR_BADCHANMASK
		_server.sendReply(_client, formatError(407, _client.getNickname(),_channel_it, ""));
		return false;
	}

	_channel = _server.findChannel(_channel_it);
	if (_channel == NULL)
	{
		//llamada a funcion error ERR_NOSUCHCHANNEL
		//_server.sendReply(_client, formatError(403, _client.getNickname(), _channel_it, ""));
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
	Channel _newChannel(_channelName);
	_newChannel.addClient(&_client);
	_newChannel.addOperator(&_client);
	_newChannel.setTopic("");
	_server.add_newChannel(_newChannel);
	_channel = _server.findChannel(_channelName);
	std::string str = ":" + _client.getNickname() + "!" + _client.getUsername() + "@" + _server.getServerName() + " JOIN :" + _channel->getName() + "\r\n";
	//luialvar
	_server.sendMessage(_client.getFd(), str);
	//luialvar
	sendReplies();
}

void JoinCommand::sendReplies()
{
	std::string names;
	std::vector<Client*> _channelClients = _channel->getClients();

	for (size_t i = 0; i < _channelClients.size(); ++i)
	{
		if (i > 0)
			names += " ";
		if (_channel->isOperator(_channelClients[i]))
			names += "@";
		names += _channelClients[i]->getNickname();
	}
	std::string str =
		"353 " +
		_client.getNickname() +
		" = " +
		_channel->getName() +
		" :" +
		names;
	_server.sendReply(_client, str);

	str =
		"366 " +
		_client.getNickname() +
		" " +
		_channel->getName() +
		" :End of /NAMES list";
	_server.sendReply(_client, str);
}

#include "InviteCommand.hpp"
#include "utils.hpp"
#include <sstream>

static std::string formatError(int code, const std::string& nick, const std::string& arg1, const std::string& arg2) {

	std::stringstream ss;
    ss << code << " " << nick << " ";

	(void)arg2;
    switch (code) {
        case 401: // ERR_NOSUCHCHNICK
            ss << arg1 << " :No such nick";
            break;
        case 403: // ERR_NOSUCHCHANNEL
            ss << arg1 << " :No such channel";
            break;
        case 442: // ERR_NOTONCHANNEL
            ss << arg1 << " :You're not on that channel";
            break;
        case 443: // ERR_USERONCHANNEL
            ss << arg1 << " :is already on channel" << arg2;
            break;
        case 461: // ERR_NEEDMOREPARAMS
            ss << arg1 << " :Not enough parameters";
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

InviteCommand::InviteCommand(Server &server, Client &client, const std::vector<std::string> &args): _server(server), _client(client), _args(args), _channel(NULL)
{}

InviteCommand::~InviteCommand(){}

void InviteCommand::execute()
{
	if (!parseAndChecks())
		return;
    _channel->addInvite(_target);
    //_server.sendReply(_client, ("341 " + _client.getNickname() + " " + _target->getNickname() + " " + _channel->getName()));
    //_server.sendMessage(_target->getFd(), "You have been invited to the channel " + _channel->getName() + " by " + _client.getNickname());
	std::string str =
		":" +
		_client.getNickname() +
		"!" +
		_client.getUsername() +
		"@" +
		_server.getServerName() +
		" INVITE " +
		_target->getNickname() +
		" " +
		_channel->getName() +
		"\r\n";
	_server.sendMessage(_target->getFd(), str);
}

bool InviteCommand::parseAndChecks()
{
    if (_args.empty() || _args.size() != 2)
    {
        //llamada a funcion error ERR_NEEDMOREPARAMS
		_server.sendReply(_client, formatError(461, _client.getNickname(), "INVITE", ""));
		return false;
    }

    _channel = _server.findChannel(_args[1]);
	if (_channel == NULL)
	{
		//llamada a funcion error ERR_NOSUCHCHANNEL
		_server.sendReply(_client, formatError(403, _client.getNickname(), _args[1], ""));
		return false;
	}

    _target = _server.findClientByNickname(_args[0]);
    if (_target == NULL)
    {
        //llamada a funcion error ERR_NOSUCHCHNICK
		_server.sendReply(_client, formatError(401, _client.getNickname(), _args[1], ""));
		return false;
    }

    //Comprobar si quien usa invite está en el canal
    if (!_channel->hasClient(&_client))
	{
        //llamada a funcion error ERR_NOTONCHANNEL
		_server.sendReply(_client, formatError(443, _client.getNickname(), _args[1], ""));
		return false;
	}

    //Comprobar si el canal está en modo solo invitar y el cliente es operador
    if (_channel->isModeSet('i') && !_channel->isOperator(&_client))
    {
        //Llamada a función error ERR_CHANOPRIVSNEEDED
        _server.sendReply(_client, formatError(482, _client.getNickname(), _args[1], ""));
        return false;
    }

    //Comprobar si el usuario al que se quiere invitar ya está en el canal
    if (_channel->hasClient(_target))
    {
        //llamada a funcion error ERR_USERONCHANNEL
        _server.sendReply(_client, formatError(443, _client.getNickname(), _args[0], _args[1]));
        return false;
    }
	return true;
}

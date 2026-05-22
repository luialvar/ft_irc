#include "WhoCommand.hpp"
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
        case 482: // ERR_CHANOPRIVISNEEDED
            ss << arg1 << " :You are not channel operator";
            break;
        default:
            ss << ":Unknown error";
            break;
    }
    return ss.str();
}

WhoCommand::WhoCommand(Server &server, Client &client, const std::vector<std::string> &args): _server(server), _client(client), _args(args), _channel(NULL)
{}

WhoCommand::~WhoCommand(){}

void WhoCommand::execute()
{
	if (!parse())
		return;
	sendInfo();
}

bool WhoCommand::parse()
{
	if (_args.empty())
	{
		//llamada a funcion error ERR_NEEDMOREPARAMS
		_server.sendReply(_client, formatError(461, _client.getNickname(), "TOPIC", ""));
		return false;
	}

    _channel = _server.findChannel(_args[0]);
	if (_channel == NULL)
	{
		//llamada a funcion error ERR_NOSUCHCHANNEL
		_server.sendReply(_client, formatError(403, _client.getNickname(), _args[0], ""));
		return false;
	}

	if (!_channel->hasClient(&_client))
	{
		//llamada a función error ERR_NOTONCHANNEL
		_server.sendReply(_client, formatError(442, _client.getNickname(), _args[0], ""));
		return false;
	}
}

void WhoCommand::sendInfo()
{

}
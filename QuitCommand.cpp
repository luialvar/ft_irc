#include "QuitCommand.hpp"
#include "utils.hpp"
#include <sstream>
#include <sys/socket.h>

static std::string formatError(int code, const std::string& nick, const std::string& arg1, const std::string& arg2) {

	std::stringstream ss;
    ss << code << " " << nick << " ";

	(void) arg2;
    switch (code) {
        case 461: // ERR_NEEDMOREPARAMS
            ss << arg1 << " :Not enough parameters";
            break;
        default:
            ss << ":Unknown error";
            break;
    }
    return ss.str();
}

QuitCommand::QuitCommand(Server &server, Client &client, const std::vector<std::string> &args): _server(server), _client(client), _args(args), _channel(NULL)
{}

QuitCommand::~QuitCommand(){}

void	QuitCommand::execute()
{
	if (_args.empty())
	{
		//llamada a funcion error ERR_NEEDMOREPARAMS
		_server.sendReply(_client, formatError(461, _client.getNickname(), "QUIT", ""));
		return;
	}
	std::string reason = "";

    if (!_args.empty())
        reason = _args[0];

    _server.smokeGrenade(_client, "QUIT", reason);
}
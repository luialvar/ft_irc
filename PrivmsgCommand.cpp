#include "PrivmsgCommand.hpp"
#include "utils.hpp"
#include <sstream>
#include <sys/socket.h>

static std::string formatError(int code, const std::string& nick, const std::string& arg1, const std::string& arg2) {

	std::stringstream ss;
    ss << code << " " << nick << " ";

	(void) arg2;
    switch (code) {
        case 401: // ERR_NOSUCHCNICK
            ss << arg1 << " :No such nick";
            break;
        case 403: // ERR_NOSUCHCHANNEL
            ss << arg1 << " :No such channel";
            break;
        case 411: // ERR_NORECIPENT
            ss << " :No recipent given " << arg1;
            break;
        case 412: // ERR_NOTEXTTOSEND
            ss << " :No text to send";
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

PrivmsgCommand::PrivmsgCommand(Server &server, Client &client, const std::vector<std::string> &args): _server(server), _client(client), _args(args), _channel(NULL)
{}

PrivmsgCommand::~PrivmsgCommand(){}

void	PrivmsgCommand::execute()
{
	if (_args.empty())
	{
		//llamada a funcion error ERR_NEEDMOREPARAMS
		_server.sendReply(_client, formatError(461, _client.getNickname(), "PRIVMSG", ""));
		return;
	}
    else if (_args.size() == 1 && _args[0][0] == ':')
    {
        //llamada a funcion error ERR_NORECIPENT
		_server.sendReply(_client, formatError(411, _client.getNickname(), "PRIVMSG", ""));
		return;
    }
    _targets = split(_args[0], ',');
	for(int i = 0; i < (int)_targets.size(); i++)
	{
        //Comprobar si el target es un canal y si existe
        if (_targets[i][0] == '#')
        {
            if (!_server.findChannel(_targets[i]))
            {
                //llamada a funcion error ERR_NOSUCHCHANNEL
                _server.sendReply(_client, formatError(403, _client.getNickname(), _targets[i], ""));
                continue;
            }
            //Intenta mandar mensaje al canal objetivo
            if (msgChannel(*_server.findChannel(_targets[i])))
                continue;
            else
                continue;
        }

        //Comprueba si el objetivo es un usuario que existe y le intenta mandar el mensaje
        if (_server.findClientByNickname(_targets[i]))
        {
            if (msgUser(*_server.findClientByNickname(_targets[i])))
            continue;
        }
        else
        {
            //llamada a funcion error ERR_NOSUCHCNICK
            _server.sendReply(_client, formatError(401, _client.getNickname(), _targets[i], ""));
            continue;
        }
    }
}

bool PrivmsgCommand::msgChannel(Channel &_targetChannel)
{
    if (_args.size() == 1)
    {
        //llamada a funcion error ERR_NOTEXTTOSEND
        _server.sendReply(_client, formatError(412, _client.getNickname(), "PRIVMSG", ""));
        return false;
    }
    //Comprobar que el cliente está en el canal al que quiere mandar el mensaje
    if (_targetChannel.hasClient(&_client))
    {
        //Preparar el mensaje
        std::string str =
            ":" +
            _client.getNickname() +
            "!" +
            _client.getUsername() +
            "@" +
            _server.getServerName() +
            " PRIVMSG " +
                _targetChannel.getName() +
                " :" +
                _args[1] +
                "\r\n";

        //Recorrer todos los clientes del canal y mandarles el mensaje
		for(int i = 0; i < (int)_targetChannel.getClientCount(); i++)
		{
            //Excluir al cliente que inició el comando ya que a él ya le sale el mensaje en pantalla
            if (_targetChannel.getClients()[i] != &_client)
            {
                int fd = _targetChannel.getClients()[i]->getFd();
                send(fd, str.c_str(), str.length(), 0);
            }
		}
        return true;
    }
    else
    {
        //llamada a funcion error ERR_NOTONCHANNEL
        _server.sendReply(_client, formatError(442, _client.getNickname(), _channel->getName(), ""));
		return false;
    }
}

bool PrivmsgCommand::msgUser(Client &_targetUser)
{
    if (_args.size() == 1)
    {
        //llamada a funcion error ERR_NOTEXTTOSEND
        _server.sendReply(_client, formatError(412, _client.getNickname(), "PRIVMSG", ""));
        return false;
    }

    //Preparar el mensaje
    std::string str =
        ":" +
        _client.getNickname() +
        "!" +
        _client.getUsername() +
        "@" +
        _server.getServerName() +
        " PRIVMSG " +
            _targetUser.getNickname() +
            " :" +
            _args[1] +
            "\r\n";

    send(_targetUser.getFd(), str.c_str(), str.length(), 0);
    return true;
}

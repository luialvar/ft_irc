#include "TopicCommand.hpp"
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

TopicCommand::TopicCommand(Server &server, Client &client, const std::vector<std::string> &args): _server(server), _client(client), _args(args), _channel(NULL)
{}

TopicCommand::~TopicCommand(){}

void TopicCommand::execute()
{
	if (!parse())
		return;
	checkOrChangeTopic();
}

bool TopicCommand::parse()
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

	if (_channel->isModeSet('t') && !_channel->isOperator(&_client) && _args.size() == 2)
	{
		//llamada a función error ERR_CHANOPRIVISNEEDED
		_server.sendReply(_client, formatError(482, _client.getNickname(), _channel->getName(), ""));
		return false;
	}

	return true;
}

void TopicCommand::checkOrChangeTopic()
{
	//Si es solo consulta del TOPIC
	if (_args.size() == 1)
	{
		//Si está vacío devolvemos el RPL_NOTOPIC
		if (_channel->getTopic().empty())
		{
			_server.sendReply(_client, " 331 " + _client.getNickname() + " " + _channel->getName() + " :No topic is set");
			return;
		}
		//Si no, devolvemos el RPL_TOPIC y el RPL_TOPICWHOTIME
		else
		{
			_server.sendReply(_client, " 332 " + _client.getNickname() + " " + _channel->getName() + " :" + _channel->getTopic());
			_server.sendReply(_client, " 333 " + _client.getNickname() + " " + _channel->getName() + " " + _channel->getTopicSetter() + " " + _channel->getTopicTime());
			return;
		}
	}
	//Si modifica el TOPIC
	else
	{
		std::string str;

		//Si el comando se usa para borrar el topic actual
		if (_args[1].size() == 1 && _args[1][0] == ':')
		{
			_channel->setTopic("");
			_channel->setTopicSetter(_client.getNickname());
			_channel->setTopicTime();

			str =
				":" +
				_client.getNickname() +
				"!" +
				_client.getUsername() +
				"@" +
				_server.getServerName() +
				" TOPIC " +
				_channel->getName() +
				" :" +
				"\r\n";

		}
		//Si el comando se usa para cambiar el topic
		else
		{
			_channel->setTopic(_args[1]);
			_channel->setTopicSetter(_client.getNickname());
			_channel->setTopicTime();

			str =
				":" +
				_client.getNickname() +
				"!" +
				_client.getUsername() +
				"@" +
				_server.getServerName() +
				" TOPIC " +
				_channel->getName() +
				" :" +
				_args[1] +
				"\r\n";
		}

		//Recorrer todos los clientes del canal y mandarles el mensaje
			for(int i = 0; i < (int)_channel->getClientCount(); i++)
			{
				int fd = _channel->getClients()[i]->getFd();
				//luialvar
				_server.sendMessage(fd, str);
				//luialvar
			}
		return;
			
	}
}

#include "KickCommand.hpp"
#include "utils.hpp"
#include <sstream>
#include <sys/socket.h>

static std::string formatError(int code, const std::string& nick, const std::string& arg1, const std::string& arg2) {

	std::stringstream ss;
    ss << code << " " << nick << " ";

	(void)arg2;
    switch (code) {
        case 401: // ERR_NOSUCHNICK
            ss << arg1 << " :No such nick";
            break;
        case 403: // ERR_NOSUCHCHANNEL
            ss << arg1 << " :No such channel";
            break;
        case 441: // ERR_USERNOTINCHANNEL
            ss << arg1 << " " << arg2 << " :They aren't on that channel";
            break;
        case 442: // ERR_NOTONCHANNEL
            ss << arg1 << " :You're not on that channel";
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

KickCommand::KickCommand(Server &server, Client &client, const std::vector<std::string> &args): _server(server), _client(client), _args(args), _channel(NULL)
{}

KickCommand::~KickCommand(){}

void KickCommand::execute()
{
    for(int i = 0; i < (int)_args.size(); i++)
        std::cout << "------------------------------------ARG[" << i << "] = " << _args[i] << "\n";
    if (!parse())
    return;
    
    
    _kickClients = split(_args[2], ',');

    for(int i = 0; i < (int)_kickClients.size(); i++)
        checkForKick(_kickClients[i]);
}

bool    KickCommand::parse()
{
    if (_args.empty() || _args.size() < 2)
    {
        _server.sendReply(_client, formatError(461, _client.getNickname(), "KICK", ""));
        return false;
    }
    
    _channel = _server.findChannel(_args[0]);
    if (_channel == NULL)
    {
        _server.sendReply(_client, formatError(403, _client.getNickname(), _args[1], ""));
        return false;
    }
    
    if (!_channel->hasClient(&_client))
    {
        _server.sendReply(_client, formatError(442, _client.getNickname(), _args[1], ""));
        return false;
    }
    
    if (!_channel->isOperator(&_client))
    {
        _server.sendReply(_client, formatError(482, _client.getNickname(), _channel->getName(), ""));
        return false;
    }
    
    if (_args.size() == 4)
    _reason = _args[3];
    else
    _reason = "";
    
    return true;
}

void    KickCommand::checkForKick(std::string _clientToKick)
{
    Client *_target = _server.findClientByNickname(_clientToKick);
    
    if (_target == NULL)
    {
        std::cout << "\nAQUI LLEGAAAAAAAAAAAAA! y el nick =: " + _clientToKick + "\n";
        _server.sendReply(_client, formatError(401, _client.getNickname(), _clientToKick, ""));
        return;
    }
    
    if (!_channel->hasClient(_target))
    {
        _server.sendReply(_client, formatError(441, _client.getNickname(), _target->getNickname(), _channel->getName()));
        return;
    }
    
    std::string _str;
    _str =
        ":" +
        _client.getNickname() +
        "!" +
        _client.getUsername() +
        "@" +
        _server.getServerName() +
        " KICK " +
        _channel->getName() +
        " " +
        _clientToKick +
        " :" +
        _reason +
        "\r\n";

    //Recorrer todos los clientes del canal y mandarles el mensaje
    for(int i = 0; i < (int)_channel->getClientCount(); i++)
    {
        int fd = _channel->getClients()[i]->getFd();
        send(fd, _str.c_str(), _str.length(), 0);
    }

    _channel->removeClient(_target);
}
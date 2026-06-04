#include "Server.hpp"

static std::string serverName()
{
    return "ircserv";
}

std::string	Server::getServerName()
{
	return serverName();
}

static std::string toUpperCommand(const std::string& command)
{
    std::string upper;

    for (std::string::size_type i = 0; i < command.size(); ++i)
    {
        if (command[i] >= 'a' && command[i] <= 'z')
            upper += command[i] - 32;
        else
            upper += command[i];
    }
    return upper;
}

static std::string getReplyTarget(const Client& client)
{
    if (client.getNickname().empty())
        return "*";
    return client.getNickname();
}

static bool isNicknameChar(char c)
{
    if (c >= 'a' && c <= 'z')
        return true;
    if (c >= 'A' && c <= 'Z')
        return true;
    if (c >= '0' && c <= '9')
        return true;
    if (c == '[' || c == ']' || c == '{' || c == '}')
        return true;
    if (c == '\\' || c == '|')
        return true;
    return false;
}

static bool isValidNickname(const std::string& nickname)
{
    if (nickname.empty())
        return false;
    if (nickname[0] == '#' || nickname[0] == ':')
        return false;
    for (std::string::size_type i = 0; i < nickname.size(); ++i)
    {
        if (!isNicknameChar(nickname[i]))
            return false;
    }
    return true;
}

void Server::handlePass(Client& client, const std::vector<std::string>& tokens)
{
    if (client.isRegistered())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 462 "
            + getReplyTarget(client) + " :You may not reregister");
        return;
    }
    if (tokens.empty() || tokens[0].empty())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 461 "
            + getReplyTarget(client) + " PASS :Not enough parameters");
        return;
    }
    if (tokens[0] != _password)
    {
        client.setPassedPassword(false);
        sendMessage(client.getFd(), ":" + serverName() + " 464 "
            + getReplyTarget(client) + " :Password incorrect");
        return;
    }
    client.setPassedPassword(true);
    tryRegisterClient(client);
}

void Server::handleNick(Client& client, const std::vector<std::string>& tokens)
{
    std::string oldNickname = client.getNickname();

    if (tokens.empty() || tokens[0].empty())
    {
        sendMessage(client.getFd(), ":" + serverName()
            + " 431 * :No nickname given");
        return;
    }
    if (!client.hasPassedPassword())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 451 "
            + getReplyTarget(client) + " :You have not registered");
        return;
    }
    if (!isValidNickname(tokens[0]))
    {
        sendMessage(client.getFd(), ":" + serverName() + " 432 "
            + getReplyTarget(client) + " " + tokens[0]
            + " :Erroneous nickname");
        return;
    }
    if (toUpperCommand(tokens[0]) != toUpperCommand(oldNickname)
        && isNicknameInUse(tokens[0]))
    {
        sendMessage(client.getFd(), ":" + serverName() + " 433 "
            + getReplyTarget(client) + " " + tokens[0]
            + " :NicknamisNicknameInUsee is already in use");
        return;
    }
    client.setNickname(tokens[0]);
    if (client.isRegistered())
    {
        sendMessage(client.getFd(), ":" + oldNickname + "!"
            + client.getUsername() + "@" + client.getIp()
            + " NICK " + tokens[0]);
    }
    else
        tryRegisterClient(client);
}

void Server::handleUser(Client& client, const std::vector<std::string>& tokens)
{
    if (client.isRegistered())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 462 "
            + getReplyTarget(client) + " :You may not reregister");
        return;
    }
    if (!client.hasPassedPassword())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 451 "
            + getReplyTarget(client) + " :You have not registered");
        return;
    }
    if (tokens.size() < 4 || tokens[0].empty())
    {
        sendMessage(client.getFd(), ":" + serverName() + " 461 "
            + getReplyTarget(client) + " USER :Not enough parameters");
        return;
    }
    client.setUsername(tokens[0]);
    client.setRealname(tokens[3]);
    tryRegisterClient(client);
}

void Server::tryRegisterClient(Client& client)
{
    if (client.isRegistered())
        return;
    if (!isClientFullyRegistered(client))
        return;
    client.setRegistered(true);
    sendMessage(client.getFd(), buildWelcomeMessage(client));
}

bool Server::isClientFullyRegistered(const Client& client) const
{
    if (!client.hasPassedPassword())
        return false;
    if (client.getNickname().empty())
        return false;
    if (client.getUsername().empty())
        return false;
    return true;
}

std::string Server::buildWelcomeMessage(const Client& client) const
{
    return ":" + serverName() + " 001 " + client.getNickname()
        + " :Welcome to the Internet Relay Network "
        + client.getNickname() + "!" + client.getUsername()
        + "@" + client.getIp();
}

bool Server::isNicknameInUse(const std::string& nickname) const
{
    std::string wanted = toUpperCommand(nickname);

    for (std::list<Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (toUpperCommand(it->getNickname()) == wanted)
            return true;
    }
    return false;
}

#include "Channel.hpp"

Channel::Channel(const std::string& name) : _name(name), _topic(""), _userLimit(0)
{
}

Channel::~Channel()
{
}

const std::string& Channel::getName() const
{
	return _name;
}

const std::string &Channel::getTopic() const
{
	return _topic;
}

void Channel::setTopic(const std::string &topic)
{
	this->_topic = topic;
}

void Channel::addClient(Client *client)
{
	this->_clients.push_back(client);
}

void Channel::removeClient(Client *client)
{
	std::vector<Client*>::iterator it = std::find(_clients.begin(), _clients.end(), client);
	if (it != _clients.end())
		_clients.erase(it);
}

bool Channel::hasClient(Client *client) const
{
	return std::find(_clients.begin(), _clients.end(), client) != _clients.end();
}

size_t Channel::getClientCount() const
{
	return _clients.size();
}

const std::vector<Client*> &Channel::getClients() const
{
	return _clients;
}

void Channel::setMode(char mode)
{
	_modes.insert(mode);
}

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

void Channel::unsetMode(char mode)
{
	_modes.erase(mode);
}

bool Channel::isModeSet(char mode) const
{
	return std::find(_modes.begin(), _modes.end(), mode) != _modes.end();
}

bool Channel::isOperator(Client *client) const
{
	return std::find(_operators.begin(), _operators.end(), client) != _operators.end();
}

void Channel::addOperator(Client *client)
{
	if (!isOperator(client))
		_operators.push_back(client);
}

void Channel::removeOperator(Client *client)
{
	std::vector<Client*>::iterator it = std::find(_operators.begin(), _operators.end(), client);
	if (it != _operators.end())
		_operators.erase(it);
}

void Channel::setKey(const std::string &key)
{
	_key = key;
}

void Channel::removeKey()
{
	_key.clear();
}

bool Channel::checkKey(const std::string &key) const
{
	return _key == key;
}

const std::string &Channel::getKey() const
{
	return _key;
}

void Channel::setUserLimit(size_t limit)
{
	_userLimit = limit;
}

void Channel::removeUserLimit()
{
	_userLimit = 0;
}

size_t Channel::getUserLimit() const
{
	return _userLimit;
}

void Channel::addInvite(Client *client)
{
	_invitedClients.insert(client);
}

void Channel::removeInvite(Client *client)
{
	_invitedClients.erase(client);
}

bool Channel::isInvited(Client *client) const
{
	return std::find(_invitedClients.begin(), _invitedClients.end(), client) != _invitedClients.end();
}


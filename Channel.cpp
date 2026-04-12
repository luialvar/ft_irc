#include "Channel.hpp"

Channel::Channel(const std::string& name) : _name(name)
{
	// Constructor
}

Channel::~Channel()
{
	// Destructor
}

const std::string& Channel::getName() const
{
	return _name;
}

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

class Client; // Forward-declaration para evitar includes circulares

class Channel
{
private:
	std::string			_name;
	std::string			_topic;
	std::vector<Client*> _clients;
	// Futuras adiciones: modos del canal, lista de operadores, etc.

public:
	Channel(const std::string& name);
	~Channel();

	const std::string& getName() const;
};

#endif

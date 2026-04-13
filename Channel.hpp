#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <set> // Para almacenar modos y clientes invitados de forma eficiente
#include <algorithm> // Para std::find en algunas implementaciones

class Client; // Forward-declaration para evitar includes circulares

class Channel
{
private:
	std::string			_name;
	std::string			_topic;
	std::vector<Client*> _clients;
	std::set<char>		_modes; // Almacena los caracteres de los modos activos (ej: 'i', 't', 'k', 'o', 'l')
	std::string			_key; // Contraseña para el modo +k
	size_t				_userLimit; // Límite de usuarios para el modo +l (0 significa sin límite)
	std::vector<Client*> _operators; // Clientes con el modo +o (operadores del canal)
	std::set<Client*>	_invitedClients; // Clientes invitados a un canal +i (invite-only)
	std::vector<std::string> _bannedMasks; // Máscaras de baneo para el modo +b

public:
	Channel(const std::string& name);
	~Channel();

	const std::string& getName() const;
	const std::string& getTopic() const;
	void setTopic(const std::string &topic);

	// Gestión de clientes en el canal
	void addClient(Client *client);
	void removeClient(Client *client);
	bool hasClient(Client *client) const;
	size_t getClientCount() const;
	const std::vector<Client*>& getClients() const;

	// Gestión de modos generales
	void setMode(char mode);
	void unsetMode(char mode);
	bool isModeSet(char mode) const;
	std::string getModeString() const; // Devuelve una cadena como "+itk"

	// Gestión de operadores (+o)
	bool isOperator(Client* client) const;
	void addOperator(Client* client);
	void removeOperator(Client* client);

	// Gestión de la clave (+k)
	void setKey(const std::string& key);
	void removeKey();
	bool checkKey(const std::string& key) const;
	const std::string& getKey() const; // Para RPL_CHANNELMODEIS

	// Gestión del límite de usuarios (+l)
	void setUserLimit(size_t limit);
	void removeUserLimit();
	size_t getUserLimit() const; // Para RPL_CHANNELMODEIS

	// Gestión de invitaciones (+i)
	void addInvite(Client* client);
	void removeInvite(Client* client);
	bool isInvited(Client* client) const;

	// Gestión de baneos (+b)
	void addBan(const std::string& mask);
	void removeBan(const std::string& mask);
	bool isBanned(Client* client) const; // Requiere el nick/host del cliente para comparar con la máscara

	// Utilidad para enviar mensajes a todos los miembros del canal
	void broadcastMessage(const std::string& message, Client* sender = NULL);
};

#endif

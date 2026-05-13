// Server.hpp
#ifndef SERVER_HPP
#define SERVER_HPP
#include <map>
#include "Client.hpp"   // -> clase Client
#include "Channel.hpp"  // -> clase Channel
#include <string>       // -> std::string
#include <list>         // -> std::list
#include <vector>       // -> std::vector
#include <poll.h>       // -> struct pollfd, poll()
#include <csignal>
#include "./includes/ft_irc.hpp"

class Server
{
public:
	// Definición de un tipo para un puntero a una función miembro manejadora de comandos
	typedef void (Server::*CommandHandler)(Client&, const std::vector<std::string>&);
		Server(int port, const std::string& password); // [x]
		~Server(); // [x]

		void run(); // [x]

		static void signalHandler(int signum); // [x]
		void sendMessage(int fd, const std::string& message);
		// Nuevos métodos auxiliares para el comando MODE
		Channel* findChannel(const std::string &name);
		Client* findClientByNickname(const std::string &nickname);
		void sendReply(const Client& client, const std::string& message);
		void add_newChannel(const Channel _channel);

private:
	int						_port;
	std::string				_password;
	std::string				_serverName;
	int						_serverSocketFd;
	std::list<Client>		_clients;
	std::vector<pollfd>		_fds;

	std::map<std::string, Channel> _channels;
	std::map<std::string, CommandHandler> _commandHandlers;

	static volatile sig_atomic_t _signal;


		void initServerSocket(); // [x]
		void acceptNewClient(); // [x]
		void receiveNewData(int fd); // [x]
		void closeAllFds(); // [x]
		void removeClient(int fd); // [x]

		Client* findClientByFd(int fd); // [x]
		const Client* findClientByFd(int fd) const; // [x]
		void initCommandHandlers();


	void disconnectClient(int fd, const std::string& reason);

		void processClientBuffer(Client& client); // [x]
		bool extractOneMessage(Client& client, std::string& message); // [x]
	void handleMessage(Client& client, const std::string& message);

	void tryRegisterClient(Client& client);
	bool isClientFullyRegistered(const Client& client) const;

	void handlePass(Client& client, const std::vector<std::string>& tokens);
	void handleNick(Client& client, const std::vector<std::string>& tokens);
	void handleUser(Client& client, const std::vector<std::string>& tokens);
	void handlePing(Client& client, const std::vector<std::string>& tokens);
	void handlePong(Client& client, const std::vector<std::string>& tokens);
	void handleQuit(Client& client, const std::vector<std::string>& tokens);
	void handleJoin(Client& client, const std::vector<std::string>& tokens);
	void handlePart(Client& client, const std::vector<std::string>& tokens);
	void handlePrivmsg(Client& client, const std::vector<std::string>& tokens);
	void handleKick(Client& client, const std::vector<std::string>& tokens);
	void handleInvite(Client& client, const std::vector<std::string>& tokens);
	void handleTopic(Client& client, const std::vector<std::string>& tokens);
	void handleMode(Client& client, const std::vector<std::string>& tokens);

	std::vector<std::string> splitIrcMessage(const std::string& message) const;
	std::string buildWelcomeMessage(const Client& client) const;
	bool isNicknameInUse(const std::string& nickname) const;
	void executeCommand(Client& client, const CommandParts &parts);


};

#endif

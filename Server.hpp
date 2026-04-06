// Server.hpp
#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"   // -> clase Client
#include <string>       // -> std::string
#include <vector>       // -> std::vector
#include <poll.h>       // -> struct pollfd, poll()

class Server
{
private:
	int						_port;
	std::string				_password;
	int						_serverSocketFd;
	std::vector<Client>		_clients;
	std::vector<pollfd>		_fds;

	static bool				_signal;

public:
	Server(int port, const std::string& password);
	~Server();

	void run();

	static void signalHandler(int signum);

private:
	void initServerSocket();
	void setupServerAddress();
	void addServerSocketToPoll();
	void acceptNewClient();
	void receiveNewData(int fd);
	void closeAllFds();
	void removeClient(int fd);

	Client* findClientByFd(int fd);
	const Client* findClientByFd(int fd) const;

	void sendMessage(int fd, const std::string& message);
	void disconnectClient(int fd, const std::string& reason);

	void processClientBuffer(Client& client);
	bool extractOneMessage(Client& client, std::string& message);
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
};

#endif
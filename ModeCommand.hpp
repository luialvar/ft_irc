#ifndef MODECOMMAND_HPP
#define MODECOMMAND_HPP
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

class ModeCommand
{
private:
	Server&	_server;
	Client&	_client;
	const std::vector<std::string>&	_args;
	Channel	*_targetChannel;
	bool	_addingMode;
	std::string	_modeChanges;
	std::vector<std::string>	_modeParams;
	std::string _modeSuccesful;
	std::string _paramsSuccesful;

public:
	ModeCommand(Server &server, Client &client, const std::vector<std::string> &args);
	void	execute();
	~ModeCommand();
private:
	bool	_parse();
	void	_handleModeRequest();
	bool	_checkPermissions();
	void	_applyChanges();
	void	_broadcastChanges();
};

#endif

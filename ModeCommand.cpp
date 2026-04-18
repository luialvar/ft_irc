#include "ModeCommand.hpp"

ModeCommand::ModeCommand(Server &server, Client &client, const std::vector<std::string> &args):_server(server), _client(client), _args(args), _targetChannel(NULL), _addingMode(true)
{}

void ModeCommand::execute()
{
	if (!_parse())
		return;
	if (_modeChanges.empty())
		return;
	if (!_checkPermissions())
		return;
	_applyChanges();
	_broadcastChanges();
}

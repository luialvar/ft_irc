#include "ModeCommand.hpp"
#include <sstream>

const std::string MODES = "itkol";

ModeCommand::ModeCommand(Server &server, Client &client, const std::vector<std::string> &args)
:_server(server), _client(client), _args(args), _targetChannel(NULL), _addingMode(true), _modeSuccesful(""), _paramsSuccesful("")
{}

ModeCommand::~ModeCommand(){}

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

bool	isValidChar(char c)
{
	return MODES.find(c) != std::string::npos;
}

bool	modeNeedsParam(char c, char sign)
{
	return (c == 'o' || (c == 'k' && sign == '+') || (c == 'l' && sign == '+'));
}

bool	ModeCommand::_parse()
{
	char lastSignChar = 0;
	char currentSing = _addingMode ? '+' : '-';

	if (_args.empty())
	{
		//llamada a funcion error ERR_NEEDMOREPARAMS
		return false;
	}

	_targetChannel = _server.findChannel(_args[0]);
	if (_targetChannel == NULL)
	{
		//llamada a funcion error ERR_NOSUCHCHANNEL
		return false;
	}

	if (_args.size() == 1)
	{
		_handleModeRequest();
		return false;
	}
	std::string modes = _args[1];
	std::string::iterator it_modes = modes.begin();
	size_t arg_idx = 2;
	while (it_modes != modes.end())
	{
		char c = *it_modes;
		if (c == '+' || c == '-')
			currentSing = c;
		else
		{
			if (isValidChar(c))
			{
				std::string currentParam = "";
				bool paramError = false;
				if (modeNeedsParam(c, currentSing))
				{
					if (arg_idx < _args.size())
					{
						currentParam = _args[arg_idx];
						arg_idx++;
					}
					else
						paramError = true;
				}
				if (!paramError)
				{
					if (currentSing != lastSignChar)
					{
						_modeChanges += currentSing;
						lastSignChar = currentSing;
					}
					_modeChanges += c;
					if (!currentParam.empty())
						_modeParams.push_back(currentParam);
				}
			}
			else
			{
				//llamar a la funcion error ERROR 472
			}
		}
		it_modes++;
	}
	return true;
}

bool	ModeCommand::_checkPermissions()
{
	if (_targetChannel->isOperator(&_client))
		return true;
	else
	{
		//llamar a la funcion error ERROR ERR_CHANOPRIVSNEEDED
		return false;
	}
}

void	ModeCommand::_applyChanges()
{
	std::string::iterator	it_modes = _modeChanges.begin();
	size_t	idx_param = 0;
	Client *client;
	char currentSing = _addingMode ? '+' : '-';

	while (it_modes != _modeChanges.end())
	{
		char lastSign = 0;
		char c = (*it_modes);
		switch (c)
		{
		case 'i':
			if (_addingMode)
				_targetChannel->setMode(c);
			else
				_targetChannel->unsetMode(c);
			if (currentSing != lastSign)
			{
				_modeChanges +=currentSing;
				lastSign = currentSing;
			}
			_modeChanges += c;
			break;
		case 't':
			if (_addingMode)
				_targetChannel->setMode(c);
			else
				_targetChannel->unsetMode(c);
			if (currentSing != lastSign)
				{
					_modeChanges +=currentSing;
					lastSign = currentSing;
				}
				_modeChanges += c;
			break;
		case 'k':
			if (_addingMode)
			{
				std::string key = _modeParams[idx_param];
				if (_targetChannel->getKey().empty())
				{
					_targetChannel->setKey(key);
					_targetChannel->setMode(c);
					if (currentSing != lastSign)
					{
						_modeChanges +=currentSing;
						lastSign = currentSing;
					}
					_modeChanges += c;
					_paramsSuccesful += " " + key;
				}
				idx_param++;
			}
			else
			{
				_targetChannel->removeKey();
				_targetChannel->unsetMode(c);
				if (currentSing != lastSign)
				{
					_modeChanges +=currentSing;
					lastSign = currentSing;
				}
				_modeChanges += c;
			}
			break;
		case 'o':
			client = _server.findClientByNickname(_modeParams[idx_param]);
			if (client)
			{
				if (_targetChannel->hasClient(client))
				{
					if (_addingMode)
						_targetChannel->addOperator(client);
					else
						_targetChannel->removeOperator(client);
					if (currentSing != lastSign)
					{
						_modeChanges +=currentSing;
						lastSign = currentSing;
					}
					_modeChanges += c;
					_paramsSuccesful += " " + _modeParams[idx_param];
				}
				else
				{
					//error ERR_USERNOTINCHANNEL
				}
			}
			else
			{
				//error ERR_NOSUCHNICK
			}
			idx_param++;
			break;
		case 'l':
			if (_addingMode)
			{
				std::string limit = _modeParams[idx_param];
				std::stringstream ss(limit);
				size_t l;
				if ((ss >> l) && ss.eof() && l > 0)
				{
					_targetChannel->setUserLimit(l);
					_targetChannel->setMode(c);
					if (currentSing != lastSign)
					{
						_modeChanges +=currentSing;
						lastSign = currentSing;
					}
					_modeChanges += c;
					_paramsSuccesful += " " + l;
				}
				idx_param++;
			}
			else
			{
				_targetChannel->removeUserLimit();
				_targetChannel->unsetMode(c);
				if (currentSing != lastSign)
				{
					_modeChanges +=currentSing;
					lastSign = currentSing;
				}
				_modeChanges += c;
			}
			break;
		case '+':
			_addingMode = true;
			currentSing = c;
			break;
		case '-':
			_addingMode = false;
			currentSing = c;
			break;
		default:
			//llamar a la funcion error ERROR 472
			break;
		}
		it_modes++;
	}
}

void ModeCommand::_broadcastChanges()
{
	if (!_modeSuccesful.empty())
	{
		std::string prefix = ":" + _client.getNickname() + "!" + _client.getUsername() + "@" + _client.getIp();
		std::string message = prefix + " MODE " + _targetChannel->getName() + " " + _modeSuccesful + _paramsSuccesful;
		_targetChannel->broadcastMessage(message, _server, NULL);
	}
}

void ModeCommand::_handleModeRequest()
{
	std::string nick;
	std::string nameChannel;
	std::string modeAndParams;
	std::string message;

	nick = _client.getNickname();
	nameChannel = _targetChannel->getName();
	modeAndParams = _targetChannel->getModeString();
	message = "324 " + nick + " " + nameChannel + " " + modeAndParams;
	_server.sendReply(_client, message);
}

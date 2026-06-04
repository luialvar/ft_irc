#include "ModeCommand.hpp"
#include <sstream>
#include <iostream>

const std::string MODES = "itkol";

static std::string formatError(int code, const std::string& nick, const std::string& arg1, const std::string& arg2) {
    std::stringstream ss;

    ss << code << " " << nick << " ";

    switch (code) {
		case 401://error ERR_NOSUCHNICK
    		ss << arg1 << " :No such nick/channel";
    		break;
        case 403: // ERR_NOSUCHCHANNEL
            ss << arg1 << " :No such channel";
            break;
        case 441: // ERR_USERNOTINCHANNEL
            ss << arg1 << " " << arg2 << " :They aren't on that channel";
            break;
        case 461: // ERR_NEEDMOREPARAMS
            ss << arg1 << " :Not enough parameters";
            break;
        case 472: // ERR_UNKNOWNMODE
            ss << arg1 << " :is unknown mode char to me for " << arg2;
            break;
        case 482: // ERR_CHANOPRIVSNEEDED
            ss << arg1 << " :You're not channel operator";
            break;
        default:
            ss << ":Unknown error";
            break;
    }
    return ss.str();
}

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

static bool	isValidChar(char c)
{
	return MODES.find(c) != std::string::npos;
}

static bool	modeNeedsParam(char c, char sign)
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
		_server.sendReply(_client, formatError(461, _client.getNickname(), "MODE", ""));
		return false;
	}

	_targetChannel = _server.findChannel(_args[0]);
	if (_targetChannel == NULL)
	{
		std::cout <<" :No such nick/channel";
		//llamada a funcion error ERR_NOSUCHCHANNEL
		_server.sendReply(_client, formatError(403, _client.getNickname(), _args[0], ""));
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
				else
					_server.sendReply(_client, formatError(461, _client.getNickname(), std::string(1, c), _args[0]));
			}
			else
			{
				//llamar a la funcion error ERROR 472
				_server.sendReply(_client, formatError(472, _client.getNickname(), std::string(1, c), _args[0]));
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
		_server.sendReply(_client, formatError(482, _client.getNickname(), _args[0], ""));
		return false;
	}
}

static	void insertSign(char &currentSing, char &lastSign, std::string &succes)
{
	if (currentSing != lastSign)
	{
		succes +=currentSing;
		lastSign = currentSing;
	}
}

static	void modeWithOutParams(bool add, Channel *cha, char c)
{
	if (add)
		cha->setMode(c);
	else
		cha->unsetMode(c);
}

void	ModeCommand::_applyChanges()
{
	std::string::iterator	it_modes = _modeChanges.begin();
	size_t	idx_param = 0;
	Client *client;
	char currentSing = _addingMode ? '+' : '-';
	char lastSign = 0;

	while (it_modes != _modeChanges.end())
	{
		char c = (*it_modes);
		switch (c)
		{
		case 'i':
			modeWithOutParams(_addingMode, _targetChannel, c);
			insertSign(currentSing, lastSign, _modeSuccesful);
			_modeSuccesful += c;
			break;
		case 't':
			modeWithOutParams(_addingMode, _targetChannel, c);
			insertSign(currentSing, lastSign, _modeSuccesful);
			_modeSuccesful += c;
			break;
		case 'k':
			if (_addingMode)
			{
				std::string key = _modeParams[idx_param];
				if (_targetChannel->getKey().empty())
				{
					_targetChannel->setKey(key);
					_targetChannel->setMode(c);
					insertSign(currentSing, lastSign, _modeSuccesful);
					_modeSuccesful += c;
					_paramsSuccesful += " " + key;
				}
				idx_param++;
			}
			else
			{
				_targetChannel->removeKey();
				_targetChannel->unsetMode(c);
				insertSign(currentSing, lastSign, _modeSuccesful);
				_modeSuccesful += c;
			}
			break;
		case 'o':
			client = _server.findClientByNickname(_modeParams[idx_param]);
			if (!client)
				_server.sendReply(_client, formatError(401, _client.getNickname(), _modeParams[idx_param], "")); // ERR_NOSUCHNICK
			else if (!_targetChannel->hasClient(client))
				_server.sendReply(_client, formatError(441, _client.getNickname(), _modeParams[idx_param], _args[0])); // ERR_USERNOTINCHANNEL
			else
			{
				if (_addingMode)
					_targetChannel->addOperator(client);
				else
					_targetChannel->removeOperator(client);
				insertSign(currentSing, lastSign, _modeSuccesful);
				_modeSuccesful += c;
				_paramsSuccesful += " " + _modeParams[idx_param];
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
					insertSign(currentSing, lastSign, _modeSuccesful);
					_modeSuccesful += c;
					_paramsSuccesful += " ";
					_paramsSuccesful += ss.str();

				}
				idx_param++;
			}
			else
			{
				_targetChannel->removeUserLimit();
				_targetChannel->unsetMode(c);
				insertSign(currentSing, lastSign, _modeSuccesful);
				_modeSuccesful += c;
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
			_server.sendReply(_client, formatError(472, _client.getNickname(), std::string(1, c), _args[0]));
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



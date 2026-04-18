#include "ModeCommand.hpp"

const std::string MODES = "itkol";

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

bool	isValidChar(char c)
{
	return MODES.find(c);
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
	if (_server.findChannel(_args[0]) == NULL)
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

#include "./includes/ft_irc.hpp"
#include <string>
#include <sstream>

void	trim(std::string &line)
{
	const std::string whitespace = " \t\n\r\f\v";

	size_t start = line.find_first_not_of(whitespace);

	if (std::string::npos == start)
	{
		line.clear();
		return;
	}

	size_t end = line.find_last_not_of(whitespace);

	line = line.substr(start, end - start + 1);
}

void	find_prefix(std::string &line, CommandParts &out)
{
	if (!line.empty() && line[0] == ':')
	{
		size_t start = line.find_first_not_of(" ");
		size_t prefix_end = line.find(" ");
		if (prefix_end != std::string::npos)
		{
			out.prefix = line.substr(1, prefix_end - 1);
			line = line.substr(prefix_end + 1);
		}
	}
}

void	parseMessage(std::string message, CommandParts &out)
{
	size_t pos_large_param = 0;

	trim(message);
	if (message.empty())
		return;

	find_prefix(message, out);
	pos_large_param = message.find(" :");
	if (pos_large_param != std::string::npos)
	{
		std::string trail = message.substr(pos_large_param + 2);
		message = message.substr(0, pos_large_param);
	}
	size_t start = message.find_first_not_of(" ");
	size_t command_end = message.find(" ");
	if (start != std::string::npos)
	{
		out.command = message.substr(start, command_end - start);
		message = message.substr(command_end + 1);
	}

}

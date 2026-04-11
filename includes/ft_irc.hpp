#ifndef FT_IRC_HPP
#define FT_IRC_HPP

#include <string>
#include <vector>

struct CommandParts
{
	std::string prefix;
	std::string command;
	std::vector<std::string> args;
};


#endif

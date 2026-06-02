#ifndef FT_IRC_HPP
#define FT_IRC_HPP

#include <string>
#include <vector>

struct CommandParts
{
	std::string prefix;
	std::string command;
	std::vector<std::string> args;

	// Método para limpiar la estructura entre pruebas
	void clear() {
		prefix.clear();
		command.clear();
		args.clear();
	}
};

#endif

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "includes/ft_irc.hpp"
#include "includes/parser.hpp"

// --- Función auxiliar para imprimir los resultados ---

void print_parts(const std::string& test_case, const CommandParts& parts)
{
	std::cout << "--- Test Case: [" << test_case << "] ---" << std::endl;
	std::cout << "Prefix:  '" << parts.prefix << "'" << std::endl;
	std::cout << "Command: '" << parts.command << "'" << std::endl;
	std::cout << "Args:    ";
	if (parts.args.empty())
	{
		std::cout << "(empty)";
	}
	else
	{
		for (size_t i = 0; i < parts.args.size(); ++i)
		{
			std::cout << "[" << i << "]='" << parts.args[i] << "' ";
		}
	}
	std::cout << std::endl << std::endl;
}

// --- Main para ejecutar las pruebas ---

int main()
{
	std::vector<std::string> tests;
	tests.push_back("NICK mi_nick");
	tests.push_back("USER user hostname server :Real Name");
	tests.push_back(":server.name NOTICE * :*** Looking up your hostname...");
	tests.push_back(":nick!user@host PRIVMSG #channel :Hello world!");
	tests.push_back("QUIT :Gone to lunch");
	tests.push_back("  PING :12345  ");
	tests.push_back("JOIN #channel1,#channel2 key1,key2");
	tests.push_back("PART #channel");
	tests.push_back(":server.name"); // Caso borde: solo prefijo
	tests.push_back("AWAY"); // Caso borde: solo comando
	tests.push_back(""); // Caso borde: vacío
	tests.push_back("   "); // Caso borde: solo espacios
	tests.push_back(":prefix   CMD   arg1   :trailing part  "); // Espacios extra

	CommandParts parts;

	for (size_t i = 0; i < tests.size(); ++i)
	{
		parts.clear();
		parseMessage(tests[i], parts);
		print_parts(tests[i], parts);
	}

	return 0;
}

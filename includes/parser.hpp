#ifndef PARSER_HPP
#define PARSER_HPP

#include "ft_irc.hpp"
#include <string>

void parseMessage(std::string message, CommandParts &out);

#endif

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

struct Message
{
	std::string					prefix;
	std::string					command;
	std::vector<std::string>	params;
};

std::string	nextToken(const std::string &str, size_t &pos);
Message		parseMessage(const std::string &str);

#endif
#include "Utils.hpp"

#include <cctype>

std::string ircLower(const std::string &value)
{
	std::string result(value);

	for (std::string::size_type i = 0; i < result.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(result[i]);
		if (c >= 'A' && c <= 'Z')
			result[i] = static_cast<char>(c - 'A' + 'a');
		else if (c == '[')
			result[i] = '{';
		else if (c == ']')
			result[i] = '}';
		else if (c == '\\')
			result[i] = '|';
		else if (c == '^')
			result[i] = '~';
	}
	return (result);
}

std::string toUpperAscii(const std::string &value)
{
	std::string result(value);

	for (std::string::size_type i = 0; i < result.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(result[i]);
		if (c >= 'a' && c <= 'z')
			result[i] = static_cast<char>(c - 'a' + 'A');
	}
	return (result);
}

bool parsePort(const std::string &value, unsigned short &port)
{
	unsigned long result = 0;

	if (value.empty())
		return (false);
	for (std::string::size_type i = 0; i < value.size(); ++i)
	{
		if (!std::isdigit(static_cast<unsigned char>(value[i])))
			return (false);
		result = result * 10 + static_cast<unsigned long>(value[i] - '0');
		if (result > 65535UL)
			return (false);
	}
	if (result == 0)
		return (false);
	port = static_cast<unsigned short>(result);
	return (true);
}

bool validChannelName(const std::string &name)
{
	if (name.size() < 2 || name.size() > 50 || (name[0] != '#' && name[0] != '&'))
		return (false);
	for (std::string::size_type i = 1; i < name.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(name[i]);
		if (c == 0 || c == 7 || c == '\r' || c == '\n' || c == ' ' || c == ',')
			return (false);
	}
	return (true);
}

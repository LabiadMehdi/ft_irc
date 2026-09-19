#include "Message.hpp"
#include <cctype>

static std::string toUpper(const std::string &s)
{
	std::string out = s;
	for (size_t i = 0; i < out.size(); i++)
		out[i] = std::toupper(static_cast<unsigned char>(out[i]));
	return out;
}

std::string	nextToken(const std::string &str, size_t &pos)
{
	std::string token;

	while (pos < str.size() && str[pos] == ' ')
		pos++;
	if (pos >= str.size())
	{
		token = "";
		return token;
	}
	size_t sp = str.find(' ', pos);
	if (sp == std::string::npos)
	{
		token = str.substr(pos);
		pos = str.size();
		return token;
	}
	token = str.substr(pos, sp - pos);
	pos = sp;
	while (pos < str.size() && str[pos] == ' ')
		pos++;
	return token;	
}

struct Message	parseMessage(const std::string &str)
{
	struct Message msg;
	size_t pos = 0;

	if (pos < str.size() && str[0] == ':')
		msg.prefix = nextToken(str, ++pos);

	msg.command = toUpper(nextToken(str, pos));

	while (pos < str.size())
	{
		if (str[pos] == ':')
		{
			msg.params.push_back(str.substr(pos + 1));
			break ;
		}
		msg.params.push_back(nextToken(str, pos));
	}
	return msg;
}

#include "IrcMessage.hpp"
#include "Utils.hpp"

bool parseIrcMessage(const std::string &line, IrcMessage &message)
{
	std::string::size_type pos = 0;
	std::string::size_type end;

	message.command.clear();
	message.params.clear();
	while (pos < line.size() && line[pos] == ' ')
		++pos;
	if (pos < line.size() && line[pos] == ':')
	{
		end = line.find(' ', pos);
		if (end == std::string::npos)
			return (false);
		pos = end + 1;
		while (pos < line.size() && line[pos] == ' ')
			++pos;
	}
	end = line.find(' ', pos);
	if (end == std::string::npos)
		end = line.size();
	if (end == pos)
		return (false);
	message.command = toUpperAscii(line.substr(pos, end - pos));
	pos = end;
	while (pos < line.size())
	{
		while (pos < line.size() && line[pos] == ' ')
			++pos;
		if (pos >= line.size())
			break ;
		if (line[pos] == ':')
		{
			message.params.push_back(line.substr(pos + 1));
			break ;
		}
		end = line.find(' ', pos);
		if (end == std::string::npos)
			end = line.size();
		message.params.push_back(line.substr(pos, end - pos));
		pos = end;
	}
	return (!message.command.empty());
}

#ifndef IRCMESSAGE_HPP
# define IRCMESSAGE_HPP

# include <string>
# include <vector>

struct IrcMessage
{
	std::string					command;
	std::vector<std::string>	params;
};

bool parseIrcMessage(const std::string &line, IrcMessage &message);

#endif

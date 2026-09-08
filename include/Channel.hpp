#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <ctime>
# include <set>
# include <string>

class Channel
{
public:
	std::string	name;

	std::set<int>	members;
	std::set<int>	operators;

	Channel();
	Channel(const std::string &channelName);
	~Channel();

	bool hasMember(int fd) const;
	bool isOperator(int fd) const;
};

#endif

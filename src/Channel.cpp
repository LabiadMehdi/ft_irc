#include "Channel.hpp"

Channel::Channel()
{
}

Channel::Channel(const std::string &channelName)
	: name(channelName)
{
}

Channel::~Channel()
{
}

bool Channel::hasMember(int fd) const
{
	return (members.find(fd) != members.end());
}

bool Channel::isOperator(int fd) const
{
	return (operators.find(fd) != operators.end());
}

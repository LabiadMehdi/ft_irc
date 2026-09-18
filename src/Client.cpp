#include "Client.hpp"

Client::Client(int fd, const std::string &host) : _fd(fd), _hostname(host), _registered(false), _pwd_accepted(false)
{
}

int	Client::getFd() const
{
	return _fd;
}

std::string &Client::getInBuf()
{
    return _in_buf;
}
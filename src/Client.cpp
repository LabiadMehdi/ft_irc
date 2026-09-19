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

std::string Client::getNick() const
{
    return _nick;
}

bool	Client::isRegistered() const
{
	return _registered;
}

bool	Client::isPwdAccepted() const
{
	return _pwd_accepted;
}

void	Client::setPwdAccepted(bool b)
{
	_pwd_accepted = b;
}

void	Client::setNick(const std::string nick)
{
	_nick = nick;
}

void	Client::setUsername(const std::string username)
{
	_username = username;
}

void	Client::setRegistered(bool b)
{
	_registered = b;
}

std::string		Client::getUsername() const
{
	return _username;
}
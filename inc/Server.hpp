#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <poll.h>

class Client;
class Channel;

class Server
{
	private:
		int								_port;
		std::string						_password;
		std::map<int, Client*>			_clients;
		std::map<std::string, Channel*>	_channels;
		std::vector<struct pollfd>		_pollfds;
		int								_listening_fd;
};

#endif
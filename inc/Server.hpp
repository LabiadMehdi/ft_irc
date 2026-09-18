#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <poll.h>
#include "Message.hpp"

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
		std::vector<int>				_toRemove;
		int								_listening_fd;
		
		Server(const Server &other);
		Server	&operator=(const Server &other);
		
		void	acceptClient();
		void	readFromClient(int fd);
		void	removeClient(int fd);
		void	handleMessage(Client *c, const Message &msg);
		void	markForRemoval(int fd);
		void	cleanupClients();
	public:
		Server(int port, const std::string &password);
		~Server();

		void	setup();
		void	run();
};

#endif
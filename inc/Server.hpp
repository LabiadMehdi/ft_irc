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
		void	removeFromAllChannels(Client *client);
		void	removeClient(int fd);
		void	handleMessage(Client *client, const Message &msg);
		void	markForRemoval(int fd);
		void	cleanupClients();
		void	sendTo(Client *client, const std::string &msg);
		void 	sendNumeric(Client *client, int code, const std::string &text);
		void	sendNumeric(Client *client, int code, const std::string &params, const std::string &text);
		Client *findByNick(const std::string &nick);
		void	checkRegistration(Client *client);

		void	handlePass(Client *client, const Message &msg);
		void	handleNick(Client *client, const Message &msg);
		void	handleUser(Client *client, const Message &msg);
		Channel *findChannel(const std::string &name);
		void 	broadcast(Channel *chan, const std::string &msg, Client *except);
		void 	handleJoin(Client *client, const Message &msg);
		void	handlePrivmsg(Client *client, const Message &msg);
	public:
		Server(int port, const std::string &password);
		~Server();

		void	setup();
		void	run();
};

#endif
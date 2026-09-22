#include "Server.hpp"
#include <sys/socket.h>
#include <stdexcept>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include "Client.hpp"
#include "Reader.hpp"
#include "Channel.hpp"

Server::Server(int port, const std::string &password) : _port(port), _password(password), _listening_fd(-1)
{
}

void	Server::setup()
{
	_listening_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listening_fd == -1)
		throw std::runtime_error("socket() failed");
	int opt = 1;
	if (setsockopt(_listening_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error(std::string("setsockopt() failed") + std::strerror(errno));

	struct	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_listening_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		throw std::runtime_error(std::string("bind() failed") + std::strerror(errno));
	if (listen(_listening_fd, SOMAXCONN) == -1)
		throw std::runtime_error(std::string("listen() failed") + std::strerror(errno));
	if (fcntl(_listening_fd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error(std::string("fcntl() failed") + std::strerror(errno));

	struct pollfd pfd;
	pfd.fd = _listening_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
}

Server::~Server()
{
	std::map<std::string, Channel*>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
	std::map<int, Client*>::iterator cit;
	for (cit = _clients.begin(); cit != _clients.end(); ++cit)
	{
		close(cit->first);
		delete cit->second;
	}
	if (_listening_fd != -1)
		close(_listening_fd);
}

void Server::markForRemoval(int fd)
{
    for (size_t i = 0; i < _toRemove.size(); i++)
        if (_toRemove[i] == fd)
            return ;
    _toRemove.push_back(fd);
}

void Server::removeFromAllChannels(Client *client)
{
    std::map<std::string, Channel*>::iterator it = _channels.begin();
    while (it != _channels.end())
    {
        Channel *chan = it->second;
        chan->removeMember(client);
        chan->removeOperator(client);
		if (chan->isEmpty())
		{
			delete chan;
			_channels.erase(it++);
		}
		else
        	++it;
    }
}

void Server::removeClient(int fd)
{
	for (size_t i = 0; i < _pollfds.size(); i++)
	{
		if (_pollfds[i].fd == fd)
		{
			_pollfds.erase(_pollfds.begin() + i);
			break;
		}
	}

	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		removeFromAllChannels(it->second);
		delete it->second;
		_clients.erase(it);
	}

	close(fd);

	std::cerr << "client disconnected on fd " << fd << std::endl;
}

void Server::cleanupClients()
{
	for (size_t i = 0; i < _toRemove.size(); i++)
		removeClient(_toRemove[i]);
	_toRemove.clear();
}

static const size_t MAX_INPUT_LINE = 512;

void	Server::readFromClient(int fd)
{
	char buf[512];
	ssize_t n = recv(fd, buf, sizeof(buf), 0);

	if (n <= 0)
	{
		broadcastQuit(_clients[fd], "Connection closed");
		markForRemoval(fd);
		return;
	}
	std::string chunk(buf, n);
	Client *client = _clients[fd];
	std::vector<Message> msgs = feed(client->getInBuf(), chunk);
	for (size_t i = 0; i < msgs.size(); i++)
		handleMessage(client, msgs[i]);
	if (client->getInBuf().size() > MAX_INPUT_LINE)
	{
		broadcastQuit(client, "Excess Flood");
		markForRemoval(fd);
	}
}

void	Server::run()
{
	while (!g_stop)
	{
		if (poll(&_pollfds[0], _pollfds.size(), -1) == -1)
		{
			if (g_stop)
				break;
			throw std::runtime_error("pollfd() failed");
		}
		if (_pollfds[0].revents & (POLLIN | POLLHUP | POLLERR))
			acceptClient();
		for (size_t i = 1; i < _pollfds.size(); i++)
		{
			if (_pollfds[i].revents & POLLIN)
			{
				readFromClient(_pollfds[i].fd);
			}
		}
		cleanupClients();
	}
}

void	Server::acceptClient()
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	int fd = accept(_listening_fd, (struct sockaddr *)&addr, &len);
	if (fd == -1)
	{
		std::cerr << "accept() failed" << std::endl;
		return ;
	}
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl() failed" << std::endl;
		close(fd);
		return ;
	}

	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);

	std::string host = inet_ntoa(addr.sin_addr);
	Client *client = new Client(fd, host);
	_clients[fd] = client;

	std::cerr << "client connected on fd " << fd << std::endl;
}

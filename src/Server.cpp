#include "Server.hpp"
#include <sys/socket.h>
#include <stdexcept>
#include <netinet/in.h>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>

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

	struct pollfd pfd;
	pfd.fd = _listening_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
}

Server::~Server()
{
	if (_listening_fd != -1)
		close(_listening_fd);
}

void	Server::run()
{
	while (true)
	{
		if (poll(&_pollfds[0], _pollfds.size(), -1) == -1)
			throw std::runtime_error("pollfd() failed");
		
		if (_pollfds[0].revents & POLLIN)
			acceptClient();
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
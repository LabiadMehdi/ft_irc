#include "Server.hpp"

Server::Server(unsigned short port, const std::string &password)
	: _listener(-1), _port(port), _password(password), _name("ircserv.local")
{
	_pollfds.reserve(MAX_CLIENTS + 1);
	setupSocket();
}

Server::~Server()
{
	for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	_clients.clear();
	if (_listener >= 0)
		close(_listener);
}

void Server::run()
{

}

void Server::cmdNick(Client &client, const IrcMessage &message)
{

}

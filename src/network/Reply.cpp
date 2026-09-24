#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Utils.hpp"
#include <sys/socket.h>
#include <iomanip>
#include <sstream>

void Server::sendNumeric(Client *client, int code, const std::string &text)
{
	sendNumeric(client, code, "", text);
}

void Server::sendTo(Client *client, const std::string &msg)
{
	std::string line = msg + "\r\n";
	send(client->getFd(), line.c_str(), line.size(), 0);
}

void Server::sendNumeric(Client *client, int code, const std::string &params, const std::string &text)
{
	std::string str = ":ircserv ";
	std::ostringstream oss;
	oss << std::setw(3) << std::setfill('0') << code;
	std::string codeStr = oss.str();
	str += codeStr;
	str += ' ';
	str += (client->getNick().empty() ? "*" : client->getNick());
	if (!params.empty())
	{
		str += " ";
		str += params;
	}
	str += " :";
	str += text;
	sendTo(client, str);
}

void 	Server::broadcast(Channel *chan, const std::string &msg, Client *except)
{
	std::set<Client*>::const_iterator it;
	for (it = chan->getMembers().begin(); it != chan->getMembers().end(); ++it)
	{
		if (*it != except)
			sendTo(*it, msg);
	}
}

void Server::broadcastQuit(Client *client, const std::string &reason)
{
    std::string line = ":" + client->getPrefix() + " QUIT :" + reason;
    std::map<std::string, Channel*>::iterator it;
    for (it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (it->second->isMember(client))
            broadcast(it->second, line, client);
    }
}

Client *Server::findByNick(const std::string &nick)
{
	std::map<int, Client *>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (toUpper(it->second->getNick()) == toUpper(nick))
			return it->second;
	}
	return NULL;
}

Channel *Server::findChannel(const std::string &name)
{
	std::map<std::string, Channel*>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (toUpper(it->first) == toUpper(name))
			return it->second;
	}
	return NULL;
}

bool Server::requireRegistered(Client *client)
{
	if (client->isRegistered())
		return true;
	sendNumeric(client, 451, "You have not registered");
	return false;
}

Channel *Server::requireChannel(Client *client, const std::string &name)
{
	Channel *chan = findChannel(name);
	if (!chan)
		sendNumeric(client, 403, name, "No such channel");
	return chan;
}

bool Server::requireMember(Client *client, Channel *chan, const std::string &name)
{
	if (chan->isMember(client))
		return true;
	sendNumeric(client, 442, name, "You're not on that channel");
	return false;
}

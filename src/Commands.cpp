#include "Server.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include <cctype>
#include "Channel.hpp"

void Server::checkRegistration(Client *client)
{
	if (client->isRegistered())
		return ;
	if (!client->isPwdAccepted())
		return ;
	if (client->getNick().empty())
		return ;
	if (client->getUsername().empty())
		return ;
	client->setRegistered(true);
	sendNumeric(client, 1,"Welcome to the ft_irc Network " + client->getNick());
}

void Server::handlePass(Client *client, const Message &msg)
{
	if (client->isRegistered())
	{
		sendNumeric(client, 462,"You may not register");
		return ;
	}
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 461,"PASS :Not enough parameters");
		return ;
	}
	if (msg.params[0] != _password)
	{
		sendNumeric(client, 464,"Password incorrect");
		return ;
	}
	client->setPwdAccepted(true);
	checkRegistration(client);
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

static bool isValidNick(const std::string &nick)
{
	if (nick.size() < 1 || nick.size() > 9)
		return false;
	std::string goodChar = "[]\\_^{|}`";
	if (!std::isalpha(static_cast<unsigned char>(nick[0])) && goodChar.find(nick[0]) == std::string::npos)
   		return false;
	for (size_t i = 1; i < nick.size(); i++)
	{
		if (!std::isalnum(static_cast<unsigned char>(nick[i])) && nick[i] != '-' && goodChar.find(nick[i]) == std::string::npos)
   			return false;
	}
	return true;
}

void Server::handleNick(Client *client, const Message &msg)
{
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 431,"No nickname given");
		return ;
	}
	if (!isValidNick(msg.params[0]))
	{
		sendNumeric(client, 432, msg.params[0], "Erroneous nickname");
		return ;
	}
	Client	*existing = findByNick(msg.params[0]);
	if (existing && existing != client)
	{
		sendNumeric(client, 433, msg.params[0], "Nickname is already in use");
		return ;
	}
	client->setNick(msg.params[0]);
	checkRegistration(client);
}

void Server::handleUser(Client *client, const Message &msg)
{
	if (client->isRegistered())
	{
		sendNumeric(client, 462,"You may not register");
		return ;
	}
	if (msg.params.size() < 4)
	{
		sendNumeric(client, 461,"USER :Not enough parameters");
		return ;
	}
	client->setUsername(msg.params[0]);
	checkRegistration(client);
}

static bool	isValidChannelName(const std::string &name)
{
	if (name.empty())
		return false;
	if (name[0] != '#')
		return false;
	if (name.find(' ') != std::string::npos || name.find(',') != std::string::npos || name.find(':') != std::string::npos )
		return false;
	return true;
}

Channel *Server::findChannel(const std::string &name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it  == _channels.end())
		return NULL;
	return it->second;
}

void Server::handleJoin(Client *client, const Message &msg)
{
	if (!client->isRegistered())
	{
		sendNumeric(client, 451, "You have not registered");
		return ;
	}
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 461, "JOIN", "Not enough parameters");
		return ;
	}
	if (!isValidChannelName(msg.params[0]))
	{
		sendNumeric(client, 403, msg.params[0], "No such channel");
		return ;
	}
	std::string name = msg.params[0];
	Channel *chan = findChannel(name);
	if (!chan)
	{
		chan = new Channel(name);
		_channels[name] = chan;
		chan->addOperator(client);
	}
	else
	{
		if (chan->isMember(client))
			return ;
		if (chan->isInviteOnly() && !chan->isInvited(client->getNick()))
		{
			sendNumeric(client, 473, name, "Cannot join channel (+i)");
			return ;
		}
		if (chan->hasKey())
		{
			std::string given;
			if (msg.params.size() > 1)
				given = msg.params[1];
			else
				given = "";
			if (given != chan->getKey())
			{
				sendNumeric(client, 475, name, "Cannot join channel (+k)");
				return ;
			}
		}
		if (chan->hasLimit() && chan->getMembers().size() >= (size_t)chan->getLimit())
		{
			sendNumeric(client, 471, name, "Cannot join channel (+l)");
			return ;
		}
	}
	chan->addMember(client);
	broadcast(chan, ":" + client->getPrefix() + " JOIN " + name, NULL);
}

void Server::handlePrivmsg(Client *client, const Message &msg)
{
	if (!client->isRegistered())
	{
		sendNumeric(client, 451, "You have not registered");
		return ;
	}
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 411, "No recipient given (PRIVMSG)");
		return ;
	}
	if (msg.params.size() < 2)
	{
		sendNumeric(client, 412, "No text to send");
		return ;
	}
	std::string target = msg.params[0];
	std::string text = msg.params[1];
	std::string line = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + text;
	if (target[0] == '#')
	{
		Channel *chan = findChannel(target);
		if (!chan)
		{
			sendNumeric(client, 403, msg.params[0], "No such channel");
			return ;
		}
		if (!chan->isMember(client))
		{
			sendNumeric(client, 404, target, "Cannot send to channel");
			return ;
		}
		broadcast(chan, ":" + client->getPrefix() + " PRIVMSG " + msg.params[0] + " :" + text, client);
	}
	else
	{
		Client *dest = findByNick(target);
		if (!dest)
		{
			sendNumeric(client, 401, msg.params[0], "No such nickname");
			return ;
		}
		sendTo(dest, line);
	}
}

void Server::handlePart(Client *client, const Message &msg)
{
	if (!client->isRegistered())
	{
		sendNumeric(client, 451, "You have not registered");
		return ;
	}
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 461,"PART", "Not enough parameters");
		return ;
	}
	std::string name = msg.params[0];
	Channel *chan = findChannel(name);
	if (!chan)
	{
		sendNumeric(client, 403, name, "No such channel");
		return ;
	}
	if (!chan->isMember(client))
	{
		sendNumeric(client, 442, name, "You're not on that channel");
		return ;
	}
	std::string reason = (msg.params.size() > 1) ? msg.params[1] : client->getNick();
	broadcast(chan, ":" + client->getPrefix() + " PART " + name + " :" + reason, NULL);
	chan->removeMember(client);
	chan->removeOperator(client);
	if (chan->isEmpty())
	{
		_channels.erase(name);
		delete chan;
	}
}

void Server::handleQuit(Client *client, const Message &msg)
{
	std::string reason = (msg.params.size() > 0) ? msg.params[0] : "Client quit";
	std::string line = ":" + client->getPrefix() + " QUIT :" + reason;
	std::map<std::string, Channel*>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->second->isMember(client))
			broadcast(it->second, line, client);
	}
	markForRemoval(client->getFd());
}

void Server::handleMessage(Client *client, const Message &msg)
{
	if (msg.command == "PASS")
        handlePass(client, msg);
    else if (msg.command == "NICK")
        handleNick(client, msg);
    else if (msg.command == "USER")
        handleUser(client, msg);
	else if (msg.command == "JOIN")
		handleJoin(client, msg);
	else if (msg.command == "PRIVMSG")
		handlePrivmsg(client, msg);
	else if (msg.command == "PART")
		handlePart(client, msg);
	else if (msg.command == "QUIT")
		handleQuit(client, msg);
    else
        sendTo(client,  "unknown command: " + msg.command);
}
#include "Server.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include <cctype>
#include "Channel.hpp"
#include <iostream>
#include <sstream>

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
	std::string newNick = msg.params[0];

	if (client->isRegistered())
	{
		std::string line = ":" + client->getPrefix() + " NICK :" + newNick;

		std::set<Client*> recipients;
		recipients.insert(client);
		std::map<std::string, Channel*>::iterator it;
		for (it = _channels.begin(); it != _channels.end(); ++it)
		{
			if (it->second->isMember(client))
			{
				const std::set<Client*> &members = it->second->getMembers();
				recipients.insert(members.begin(), members.end());
			}
		}

		std::set<Client*>::iterator r;
		for (r = recipients.begin(); r != recipients.end(); ++r)
			sendTo(*r, line);
	}

	client->setNick(newNick);
	checkRegistration(client);
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
	if (!chan->getTopic().empty())
    	sendNumeric(client, 332, name, chan->getTopic());
	sendNumeric(client, 353, "= " + name, chan->getNamesList());
	sendNumeric(client, 366, name, "End of /NAMES list");
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
	broadcastQuit(client, reason);
	markForRemoval(client->getFd());
}

void Server::handlePing(Client *client, const Message &msg)
{
	std::string token = (msg.params.size() > 0) ? msg.params[0] : "";
	sendTo(client, ":ircserv PONG ircserv :" + token);
}

void Server::handlePong(Client *client, const Message &msg)
{
	(void)client;
	(void)msg;
}

void Server::handleKick(Client *client, const Message &msg)
{
	if (!client->isRegistered())
	{
		sendNumeric(client, 451, "You have not registered");
		return ;
	}
	if (msg.params.size() < 2)
	{
		sendNumeric(client, 461,"KICK", "Not enough parameters");
		return ;
	}
	std::string name = msg.params[0];
	Channel *chan = findChannel(name);
	if (!chan)
	{
    	sendNumeric(client, 403, name, "No such channel");
    	return;
	}
	if (!chan->isMember(client))
	{
		sendNumeric(client, 442, name, "You're not on that channel");
		return ;
	}
	if (!chan->isOperator(client))
	{
		sendNumeric(client, 482, name, "You're not channel operator");
		return ;
	}
	Client *toKick = findByNick(msg.params[1]);
	if (!toKick || !chan->isMember(toKick))
	{
		sendNumeric(client, 441, msg.params[1] + " " + name, "They aren't on that channel");
    	return;
	}
	std::string reason = (msg.params.size() > 2) ? msg.params[2] : client->getNick();
	std::string line = ":" + client->getPrefix() + " KICK " + name + " " + msg.params[1] + " :" + reason;
	broadcast(chan, line, NULL);
	chan->removeMember(toKick);
	chan->removeOperator(toKick);
	if (chan->isEmpty())
	{
		_channels.erase(name);
		delete chan;
	}
}

void Server::handleInvite(Client *client, const Message &msg)
{
	if (!client->isRegistered())
	{
		sendNumeric(client, 451, "You have not registered");
		return ;
	}
	if (msg.params.size() < 2)
	{
		sendNumeric(client, 461,"INVITE", "Not enough parameters");
		return ;
	}
	Client *target = findByNick(msg.params[0]);
	if (!target)
	{
		sendNumeric(client, 401, msg.params[0], "No such nick/channel");
		return ;
	}
	Channel *chan = findChannel(msg.params[1]);
	std::string name = msg.params[1];
	if (!chan)
	{
    	sendNumeric(client, 403, name, "No such channel");
    	return;
	}
	if (!chan->isMember(client))
	{
		sendNumeric(client, 442, name, "You're not on that channel");
		return ;
	}
	if (chan->isInviteOnly())
	{
		if (!chan->isOperator(client))
		{
			sendNumeric(client, 482, name, "You're not an operator");
			return ;
		}
	}
	if (chan->isMember(target))
	{
		sendNumeric(client, 443, msg.params[0] + " " + name, "is already in channel");
		return ;
	}
	chan->addInvite(msg.params[0]);
	sendNumeric(client, 341, msg.params[0] + " " + name, "");
	sendTo(target, ":" + client->getPrefix() + " INVITE " + msg.params[0] + " :" + name);
}

void Server::handleTopic(Client *client, const Message &msg)
{
	if (!client->isRegistered())
	{
		sendNumeric(client, 451, "You have not registered");
		return ;
	}
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 461,"TOPIC", "Not enough parameters");
		return ;
	}
	Channel *chan = findChannel(msg.params[0]);
	std::string name = msg.params[0];
	if (!chan)
	{
		sendNumeric(client, 403, name, "No such channel");
    	return;
	}
	if (!chan->isMember(client))
	{
		sendNumeric(client, 442, name, "You're not on that channel");
		return ;
	}
	if (msg.params.size() < 2)
	{
		if (chan->getTopic().empty())
			sendNumeric(client, 331, name, "No topic is set");
		else
			sendNumeric(client, 332, name, chan->getTopic());
		return ;
	}
	if (chan->isTopicRestricted())
	{
		if (!chan->isOperator(client))
		{
			sendNumeric(client, 482, name, "You're not an operator");
			return ;
		}
	}
	chan->setTopic(msg.params[1]);
	broadcast(chan, ":" + client->getPrefix() + " TOPIC " + name + " :" + msg.params[1], NULL);
}

void fillApplied(char &appliedSign, std::string &applied, const char c, const char sign)
{
	if (appliedSign != sign)
	{
		applied += sign;
		appliedSign = sign;
	}
	applied += c;
}

void Server::handleMode(Client *client, const Message &msg)
{
	if (!client->isRegistered())
	{
		sendNumeric(client, 451, "You have not registered");
		return ;
	}
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 461,"MODE", "Not enough parameters");
		return ;
	}
	std::string name = msg.params[0];
	Channel *chan = findChannel(name);
	if (!chan)
	{
		sendNumeric(client, 403, name, "No such channel");
		return ;
	}
	if (msg.params.size() == 1)
	{
		sendNumeric(client, 324, name + " " + chan->getModeString(), "");
		return ;
	}
	if (!chan->isMember(client))
	{
		sendNumeric(client, 442, name, "You're not on that channel");
		return ;
	}
	if (!chan->isOperator(client))
	{
		sendNumeric(client, 482, name, "You're not channel operator");
		return ;
	}
	
	std::string modes = msg.params[1];
	char sign = '+';
	size_t paramIndex = 2;
	std::string applied;
	std::string appliedArgs;
	char appliedSign = 0;
	for (size_t i = 0; i < modes.size(); i++)
	{
		char c = modes[i];
		if (c == '+' || c == '-')
		{
			sign = c;
			continue ;
		}
		switch (c)
		{
			case 'i':
				chan->setInviteOnly(sign == '+');
				fillApplied(appliedSign, applied, c, sign);
				break ;
			case 't':
				chan->setTopicRestricted(sign == '+');
				fillApplied(appliedSign, applied, c, sign);
				break ;
			case 'k':
			{
				if (sign == '+')
				{
					if (paramIndex >= msg.params.size())
						break;
					std::string key = msg.params[paramIndex++];
					chan->setKey(key);
					fillApplied(appliedSign, applied, c, sign);
					appliedArgs += " " + key;
				}
				else
				{
					chan->setKey("");
					fillApplied(appliedSign, applied, c, sign);
				}
				break;
			}
			case 'l':
			{
				if (sign == '+')
				{
					if (paramIndex >= msg.params.size())
						break;
					std::string arg = msg.params[paramIndex++];
					int limit;
					std::stringstream ss(arg);
					ss >> limit;
					if (ss.fail() || !ss.eof() || limit <= 0)
						break;
					chan->setLimit(limit);
					fillApplied(appliedSign, applied, c, sign);
					appliedArgs += " " + arg;
				}
				else
				{
					chan->clearLimit();
					fillApplied(appliedSign, applied, c, sign);
				}
				break;
			}
			case 'o':
			{
				if (paramIndex >= msg.params.size())
					break;
				std::string nick = msg.params[paramIndex++];
				Client *target = findByNick(nick);
				if (!target || !chan->isMember(target))
				{
					sendNumeric(client, 441, nick + " " + name, "They aren't on that channel");
					break;
				}
				if (sign == '+')
					chan->addOperator(target);
				else
					chan->removeOperator(target);
				fillApplied(appliedSign, applied, c, sign);
				appliedArgs += " " + nick;
				break;
			}
			default:
				sendNumeric(client, 472, std::string(1, c), "is unknown mode char to me");
				break ;
		}
	}
	if (!applied.empty())
		broadcast(chan, ":" + client->getPrefix() + " MODE " + name + " " + applied + appliedArgs, NULL);
}

void Server::handleMessage(Client *client, const Message &msg)
{
	if (msg.command.empty())
		return ;
	else if (msg.command == "PASS")
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
	else if (msg.command == "PING")
		handlePing(client, msg);
	else if (msg.command == "PONG")
		handlePong(client, msg);
	else if (msg.command == "KICK")
		handleKick(client, msg);
	else if (msg.command == "INVITE")
		handleInvite(client, msg);
	else if (msg.command == "TOPIC")
		handleTopic(client, msg);
	else if (msg.command == "MODE")
		handleMode(client, msg);
    else
        sendNumeric(client,  421, msg.command, "Unknown command");
}
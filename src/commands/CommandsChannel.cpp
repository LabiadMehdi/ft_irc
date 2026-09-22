#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

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

void Server::handleJoin(Client *client, const Message &msg)
{
	if (!requireRegistered(client))
		return ;
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

void Server::handlePart(Client *client, const Message &msg)
{
	if (!requireRegistered(client))
		return ;
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 461,"PART", "Not enough parameters");
		return ;
	}
	std::string name = msg.params[0];
	Channel *chan = requireChannel(client, name);
	if (!chan)
		return ;
	if (!requireMember(client, chan, name))
		return ;
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

void Server::handleKick(Client *client, const Message &msg)
{
	if (!requireRegistered(client))
		return ;
	if (msg.params.size() < 2)
	{
		sendNumeric(client, 461,"KICK", "Not enough parameters");
		return ;
	}
	std::string name = msg.params[0];
	Channel *chan = requireChannel(client, name);
	if (!chan)
		return ;
	if (!requireMember(client, chan, name))
		return ;
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
	if (!requireRegistered(client))
		return ;
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
	std::string name = msg.params[1];
	Channel *chan = requireChannel(client, name);
	if (!chan)
		return ;
	if (!requireMember(client, chan, name))
		return ;
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
	if (!requireRegistered(client))
		return ;
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 461,"TOPIC", "Not enough parameters");
		return ;
	}
	std::string name = msg.params[0];
	Channel *chan = requireChannel(client, name);
	if (!chan)
		return ;
	if (!requireMember(client, chan, name))
		return ;
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

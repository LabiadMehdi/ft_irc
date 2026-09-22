#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

void Server::handlePrivmsg(Client *client, const Message &msg)
{
	if (!requireRegistered(client))
		return ;
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

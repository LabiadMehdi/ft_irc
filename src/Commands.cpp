#include "Server.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include <cctype>

void Server::handlePass(Client *client, const Message &msg)
{
	if (client->isRegistered())
	{
		sendNumeric(client, 462, msg.params[0], "You may not register");
		return ;
	}
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 461, msg.params[0], "PASS :Not enough parameters");
		return ;
	}
	if (msg.params[0] != _password)
	{
		sendNumeric(client, 464, msg.params[0], "Password incorrect");
		return ;
	}
	client->setPwdAccepted(true);
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
		sendNumeric(client, 431, msg.params[0], "No nickname given");
		return ;
	}
	if (!isValidNick(msg.params[0]))
	{
		sendNumeric(client, 432, msg.params[0], msg.params[0] + " :Erroneous nickname");
		return ;
	}
	Client	*existing = findByNick(msg.params[0]);
	if (existing && existing != client)
	{
		sendNumeric(client, 433, msg.params[0], msg.params[0] + " :Nickname is already in use");
		return ;
	}
	client->setNick(msg.params[0]);
}

void Server::handleUser(Client *client, const Message &msg)
{
    (void)client;
    (void)msg;    
}

void Server::handleMessage(Client *client, const Message &msg)
{
	if (msg.command == "PASS")
        handlePass(client, msg);
    else if (msg.command == "NICK")
        handleNick(client, msg);
    else if (msg.command == "USER")
        handleUser(client, msg);
    else
        sendTo(client,  "unknown command: " + msg.command);
}
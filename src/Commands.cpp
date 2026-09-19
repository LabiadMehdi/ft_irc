#include "Server.hpp"
#include "Client.hpp"

void Server::handlePass(Client *client, const Message &msg)
{
    (void)client;
    (void)msg;
}

void Server::handleNick(Client *client, const Message &msg)
{
	(void)msg;
	sendNumeric(client, 464, "Password incorrect");
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
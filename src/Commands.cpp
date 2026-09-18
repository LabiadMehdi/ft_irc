#include "Server.hpp"
#include "Client.hpp"
#include <iostream>

void Server::handleMessage(Client *client, const Message &msg)
{
    std::cerr << "fd " << client->getFd()
              << " command [" << msg.command << "]" << std::endl;
}
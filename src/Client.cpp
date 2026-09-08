#include "Client.hpp"

Client::Client(int socketFd, const std::string &clientHost)
	: fd(socketFd), host(clientHost), passwordAccepted(false)
{
}

Client::~Client()
{
}

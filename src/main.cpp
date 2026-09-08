#include "Server.hpp"
#include "Utils.hpp"

#include <exception>
#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	unsigned short port;
	if (!parsePort(argv[1], port))
	{
		std::cerr << "Error: port must be a number between 1 and 65535" << std::endl;
		return (1);
	}
	try
	{
		Server server(port, argv[2]);
		server.run();
	}
	catch (const std::exception &error)
	{
		std::cerr << "Error: " << error.what() << std::endl;
		return (1);
	}
	return (0);
}

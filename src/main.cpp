#include "Server.hpp"
#include <sstream>
#include <iostream>
#include <csignal>

bool	parsePort(const char *str, int &port)
{
	std::stringstream ss(str);
	port = 0;
	ss >> port;
	if (ss.fail())
		return false;
	if (!ss.eof())
		return false;
	if (port < 1024 || port > 65535)
		return false;
	return true;
}

volatile sig_atomic_t g_stop = 0;

static void handleSignal(int sig)
{
	(void)sig;
	g_stop = 1;
}

int main(int ac, char **av)
{
	if (ac == 3)
	{
		int port;
		if (!parsePort(av[1], port))
		{
			std::cerr << "invalid port" << std::endl;
			return 1;
		}
		try
		{
			Server server(port, av[2]);
			server.setup();
			signal(SIGINT, handleSignal);
			signal(SIGQUIT, handleSignal);
			signal(SIGPIPE, SIG_IGN);
			server.run();
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error " << e.what() << std::endl;
			return 1;
		}
		return 0;
	}
	std::cerr << "Usage error" << std::endl;
	return 1;
}

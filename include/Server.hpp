#ifndef SERVER_HPP
# define SERVER_HPP

# include "Channel.hpp"
# include "Client.hpp"
# include "IrcMessage.hpp"

class Server
{
public:
	Server(unsigned short port, const std::string &password);
	~Server();

	void run();

private:
	int				_listener;
	unsigned short		_port;
	std::string			_password;
	std::string			_name;

	void cmdNick(Client &client, const IrcMessage &message);

	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);
};

#endif

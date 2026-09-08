#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>

class Client
{
public:
	int			fd;
	std::string	host;
	std::string	nickname;
	std::string	username;
	std::string	realname;
	bool		passwordAccepted;

	Client(int socketFd, const std::string &clientHost);
	~Client();

private:
	Client();
	Client(const Client &other);
	Client &operator=(const Client &other);
};

#endif

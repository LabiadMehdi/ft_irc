#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
	private:
		int			_fd;
		std::string	_in_buf;
		std::string	_out_buf;
		std::string	_nick;
		std::string	_username;
		std::string	_hostname;
		bool		_registered;
		bool		_pwd_accepted;
	public:
		Client(int fd, const std::string &host);
		int					getFd() const;
		std::string 		&getInBuf();
		const std::string	&getNick() const;
		const std::string	&getUsername() const;
		std::string 		getPrefix() const;
		bool				isRegistered() const;
		bool				isPwdAccepted() const;
		void				setPwdAccepted(bool b);
		void				setRegistered(bool b);
		void				setNick(const std::string &nick);
		void				setUsername(const std::string username);
};

#endif
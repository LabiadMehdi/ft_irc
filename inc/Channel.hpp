#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <vector>

class Client;

class Channel
{
	private:
		std::string				_name;
		std::string				_topic;
		std::set<Client*>		_members;
		std::set<Client*>		_operators;
		std::set<std::string>	_invited;
		bool					_inv_only;
		bool					_topic_restricted;
		std::string				_key;
		int						_user_limit;
		bool					_has_limit;
};

#endif
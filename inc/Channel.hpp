#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

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
	public:
		Channel(const std::string &name);
		
		const std::string &getName() const;
		const std::string &getTopic() const;
		void setTopic(const std::string &topic);
		bool isTopicRestricted() const;

		void addMember(Client *c);
		void removeMember(Client *c);
		bool isMember(Client *c) const;
		bool isEmpty() const;

		void addOperator(Client *c);
		void removeOperator(Client *c);
		bool isOperator(Client *c) const;
		void addInvite(const std::string &nick);

		bool isInviteOnly() const;
		bool isInvited(const std::string &name) const;
		bool hasKey() const;
		bool hasLimit() const;
		const std::string &getKey() const;
		int getLimit() const;

		const std::set<Client*> &getMembers() const;
};

#endif
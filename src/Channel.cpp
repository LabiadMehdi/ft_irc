#include "Channel.hpp"

Channel::Channel(const std::string &name) : _name(name), _inv_only(false), _topic_restricted(false), _user_limit(0), _has_limit(false)
{	
}

const std::string &Channel::getName() const
{
	return _name;
}

const std::string &Channel::getTopic() const
{
	return _topic;
}

void Channel::setTopic(const std::string &topic)
{
	_topic = topic;
}

void Channel::addMember(Client *c)
{
	_members.insert(c);
}

void Channel::removeMember(Client *c)
{
	_members.erase(c);
}

bool Channel::isMember(Client *c) const
{
	return _members.count(c) > 0;
}

bool Channel::isEmpty() const
{
	return _members.empty();
}

void Channel::addOperator(Client *c)
{
	_operators.insert(c);
}

void Channel::removeOperator(Client *c)
{
	_operators.erase(c);
}

bool Channel::isOperator(Client *c) const
{
	return _operators.count(c) > 0;
}

const std::set<Client*> &Channel::getMembers() const
{
	return _members;
}

bool Channel::isInviteOnly() const
{
	return _inv_only;
}

bool Channel::isInvited(const std::string &name) const
{
	return _invited.count(name) > 0;
}

bool Channel::hasKey() const
{
	return !_key.empty();
}

bool Channel::hasLimit() const
{
	return _has_limit;
}

const std::string &Channel::getKey() const
{
	return _key;
}

int Channel::getLimit() const
{
	return _user_limit;
}

void Channel::addInvite(const std::string &nick)
{
	_invited.insert(nick);
}

bool Channel::isTopicRestricted() const
{
	return _topic_restricted;
}

void Channel::setInviteOnly(bool b)
{
	_inv_only = b;
}

void Channel::setTopicRestricted(bool b)
{
	_topic_restricted = b;
}

void Channel::setKey(const std::string &key)
{
	_key = key;
}
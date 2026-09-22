#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <sstream>

static void fillApplied(char &appliedSign, std::string &applied, const char c, const char sign)
{
	if (appliedSign != sign)
	{
		applied += sign;
		appliedSign = sign;
	}
	applied += c;
}

void Server::handleMode(Client *client, const Message &msg)
{
	if (!requireRegistered(client))
		return ;
	if (msg.params.size() < 1)
	{
		sendNumeric(client, 461,"MODE", "Not enough parameters");
		return ;
	}
	std::string name = msg.params[0];
	Channel *chan = requireChannel(client, name);
	if (!chan)
		return ;
	if (msg.params.size() == 1)
	{
		sendNumeric(client, 324, name + " " + chan->getModeString(), "");
		return ;
	}
	if (!requireMember(client, chan, name))
		return ;
	if (!chan->isOperator(client))
	{
		sendNumeric(client, 482, name, "You're not channel operator");
		return ;
	}

	std::string modes = msg.params[1];
	char sign = '+';
	size_t paramIndex = 2;
	std::string applied;
	std::string appliedArgs;
	char appliedSign = 0;
	for (size_t i = 0; i < modes.size(); i++)
	{
		char c = modes[i];
		if (c == '+' || c == '-')
		{
			sign = c;
			continue ;
		}
		switch (c)
		{
			case 'i':
				chan->setInviteOnly(sign == '+');
				fillApplied(appliedSign, applied, c, sign);
				break ;
			case 't':
				chan->setTopicRestricted(sign == '+');
				fillApplied(appliedSign, applied, c, sign);
				break ;
			case 'k':
			{
				if (sign == '+')
				{
					if (paramIndex >= msg.params.size())
						break;
					std::string key = msg.params[paramIndex++];
					chan->setKey(key);
					fillApplied(appliedSign, applied, c, sign);
					appliedArgs += " " + key;
				}
				else
				{
					chan->setKey("");
					fillApplied(appliedSign, applied, c, sign);
				}
				break;
			}
			case 'l':
			{
				if (sign == '+')
				{
					if (paramIndex >= msg.params.size())
						break;
					std::string arg = msg.params[paramIndex++];
					int limit;
					std::stringstream ss(arg);
					ss >> limit;
					if (ss.fail() || !ss.eof() || limit <= 0)
						break;
					chan->setLimit(limit);
					fillApplied(appliedSign, applied, c, sign);
					appliedArgs += " " + arg;
				}
				else
				{
					chan->clearLimit();
					fillApplied(appliedSign, applied, c, sign);
				}
				break;
			}
			case 'o':
			{
				if (paramIndex >= msg.params.size())
					break;
				std::string nick = msg.params[paramIndex++];
				Client *target = findByNick(nick);
				if (!target || !chan->isMember(target))
				{
					sendNumeric(client, 441, nick + " " + name, "They aren't on that channel");
					break;
				}
				if (sign == '+')
					chan->addOperator(target);
				else
					chan->removeOperator(target);
				fillApplied(appliedSign, applied, c, sign);
				appliedArgs += " " + nick;
				break;
			}
			default:
				sendNumeric(client, 472, std::string(1, c), "is unknown mode char to me");
				break ;
		}
	}
	if (!applied.empty())
		broadcast(chan, ":" + client->getPrefix() + " MODE " + name + " " + applied + appliedArgs, NULL);
}

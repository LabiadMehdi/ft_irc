#include "Reader.hpp"

std::vector<Message>	feed(std::string &buffer, const std::string &chunk)
{
	std::vector<Message> out;
	buffer += chunk;
	while (true)
	{
		size_t nl = buffer.find('\n');
		if (nl == std::string::npos)
			break ;
		size_t end = nl;
		if (end > 0 && buffer[end - 1] == '\r')
			end--;
		std::string line = buffer.substr(0, end);
		buffer.erase(0, nl + 1);
		out.push_back(parseMessage(line));
	}
	return out;
}
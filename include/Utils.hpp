#ifndef UTILS_HPP
# define UTILS_HPP

# include <cstddef>
# include <sstream>
# include <string>
# include <vector>

std::string ircLower(const std::string &value);
std::string toUpperAscii(const std::string &value);
bool parsePort(const std::string &value, unsigned short &port);
bool validChannelName(const std::string &name);

#endif

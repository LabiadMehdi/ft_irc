#include "Utils.hpp"
#include <cctype>

std::string toUpper(const std::string &s)
{
	std::string out = s;
	for (size_t i = 0; i < out.size(); i++)
		out[i] = std::toupper(static_cast<unsigned char>(out[i]));
	return out;
}
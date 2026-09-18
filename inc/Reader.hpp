#ifndef READER_HPP
#define READER_HPP

#include <string>
#include <iostream>
#include "Message.hpp"

std::vector<Message>	feed(std::string &buffer, const std::string &chunk);

#endif
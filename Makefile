NAME     = ircserv
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinc

SRCDIR   = src
OBJDIR   = obj

SRCS     = main.cpp Utils.cpp \
           network/Server.cpp network/Reply.cpp network/Reader.cpp \
           model/Client.cpp model/Channel.cpp model/Message.cpp \
           commands/Commands.cpp commands/CommandsRegistration.cpp commands/CommandsChannel.cpp commands/CommandsMode.cpp
OBJS     = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))
HEADERS  = $(wildcard inc/*.hpp)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
NAME     = ircserv
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinc

SRCDIR   = src
OBJDIR   = obj

SRCS     = main.cpp Server.cpp Client.cpp Channel.cpp Commands.cpp \
           Message.cpp Reader.cpp
OBJS     = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))
HEADERS  = $(wildcard inc/*.hpp)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
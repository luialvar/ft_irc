NAME := ircserv
TEST_NAME := test_parser

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98

# Server sources
SRCS := main.cpp Server.cpp Client.cpp Channel.cpp parser.cpp ModeCommand.cpp JoinCommand.cpp
OBJS := $(SRCS:.cpp=.o)

# Test sources
TEST_SRCS := test_parser.cpp parser.cpp
TEST_OBJS := $(TEST_SRCS:.cpp=.o)

HEADERS := Server.hpp Client.hpp Channel.hpp includes/ft_irc.hpp includes/parser.hpp

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

test: $(TEST_NAME)

$(TEST_NAME): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) -o $(TEST_NAME)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TEST_OBJS)

fclean: clean
	rm -f $(NAME) $(TEST_NAME)

re: fclean all

.PHONY: all test clean fclean re

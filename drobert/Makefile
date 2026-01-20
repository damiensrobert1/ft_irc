NAME := ircserv 
CXX := c++
#CXXFLAGS := -Wall -Wextra -Werror -std=c++98
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -g

SRCS := main.cpp \
	Channel.cpp \
	Client.cpp \
	Parsed.cpp \
	Server.cpp \
	Command.cpp \
	Utils.cpp 

OBJ_DIR := obj

OBJS := $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re


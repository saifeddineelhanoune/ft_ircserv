
CXX = c++

CXXFLAGS = -Wall -Wextra -Werror  -Iinclude -Ofast -std=c++98

SRC = src

SRC_DIR = $(wildcard $(SRC)/*.cpp) main.cpp

OBJ_DIR = obj
OBJ = $(SRC_DIR:src/%.cpp=$(OBJ_DIR)/%.o)

NAME = ircserv

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)
fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

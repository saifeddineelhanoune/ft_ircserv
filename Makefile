# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude

# Directories
SRC = src
# SRC_DIR = src/events.cpp src/server.cpp src/argsValidation.cpp \
#           main.cpp src/client.cpp src/cmd.cpp
SRC_DIR = $(wildcard $(SRC)/*.cpp) main.cpp

OBJ_DIR = obj
OBJ = $(SRC_DIR:src/%.cpp=$(OBJ_DIR)/%.o)

EXEC = ircServer

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: src/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create object directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up build files
clean:
	rm -rf $(OBJ_DIR) $(EXEC)

# Include dependencies
# -include $(OBJ:.o=.d)

.PHONY: all clean

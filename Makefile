NAME	=	webserv
CXX		=	c++
FLAGS	=	-Wall -Werror -Wextra -g -std=c++98
# FLAGS	+= -fsanitize=address

SRC_DIR =	./src/
OBJ_DIR	=	./obj/
I_DIR	=	./incl/
TEST_DIR = 	./test/
BUILD_DIR = ./build/


SRC		=	Config.cpp \
			main.cpp \
			ConfigUtils.cpp \
			Location.cpp \
			Log.cpp \
			ServerEngine.cpp \
			CgiHandler.cpp \
			CgiResponse.cpp \
			ContentTypes.cpp \
			ErrorPageGenerator.cpp \
			Exceptions.cpp \
			HttpHeaders.cpp \
			Request.cpp \
			RequestParser.cpp \
			RequestProcessor.cpp \
			RequestUtils.cpp \
			ServerBlock.cpp \
			StatusCodes.cpp \
			Utils.cpp

OBJ		=	$(SRC:cpp=o)
SRC_FILES = $(addprefix $(SRC_DIR), $(SRC))
OBJ_FILES = $(addprefix $(OBJ_DIR), $(OBJ))

TEST_TARGETS = test_utils test_http_GET test_http_POST test_http_DELETE

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

all: $(NAME)

$(NAME): $(OBJ_FILES)
	$(CXX) $(FLAGS) $^ -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	$(CXX) $(FLAGS) -I$(I_DIR) -c $< -o $@

# Build and run tests using CMake
$(BUILD_DIR)/CMakeCache.txt:
	@cmake -S . -B $(BUILD_DIR)

test_build: $(BUILD_DIR)/CMakeCache.txt
	@cmake --build $(BUILD_DIR) --target $(TEST_TARGETS)

test: test_build
	@echo "\n\033[1;34m[INFO]\033[0m Running CTest for all registered tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@for test in $(TEST_TARGETS); do \
		echo "\n\033[1;34m[INFO]\033[0m Running $$test..."; \
		$(BUILD_DIR)/$$test && echo "\033[1;32m[SUCCESS]\033[0m $$test passed!" || echo "\033[1;31m[FAILURE]\033[0m $$test failed!"; \
	done

clean:
	@if [ -d ./$(OBJ_DIR) ]; then rm -rf $(OBJ_DIR); fi
	@if [ -d ./build ]; then rm -rf $(BUILD_DIR); fi

fclean: clean
	rm -f $(NAME)
	rm -rf www/three-socketeers/uploads/*
	rm -rf www/cgi-bin/guest-book/*
	rm -rf logs/*
	rm -rf www/secondary/uploads/*

re:	fclean all

.PHONY: all clean fclean re

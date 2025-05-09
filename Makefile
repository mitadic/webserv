NAME	=	webserv
CXX		=	c++
FLAGS	=	-Wall -Werror -Wextra -g -std=c++98
# FLAGS	+= -fsanitize=address

SRC_DIR =	./src/
OBJ_DIR	=	./obj/
I_DIR	=	./incl/
TEST_DIR = 	./test/
BUILD_DIR = ./build/
SHELL	:= /usr/bin/bash


SRC		=	Config.cpp \
			ConfigUtils.cpp \
			Location.cpp \
			Log.cpp \
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
			ServerEngine.cpp \
			StatusCodes.cpp \
			Utils.cpp \
			main.cpp

OBJ		=	$(SRC:cpp=o)
SRC_FILES = $(addprefix $(SRC_DIR), $(SRC))
OBJ_FILES = $(addprefix $(OBJ_DIR), $(OBJ))

TEST_TARGETS = test_http_GET test_http_POST test_http_DELETE

all: $(NAME)

$(NAME): $(addprefix $(OBJ_DIR),$(OBJ))
	$(CXX) $(FLAGS) $^ -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(I_DIR)*
	@if [ ! -d $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi
	$(CXX) $(FLAGS) -I$(I_DIR) -c $< -o $@

# Build and run tests using CMake
$(BUILD_DIR)/CMakeCache.txt:
	@cmake -S . -B $(BUILD_DIR)

test: all
	@if [ ! -d .venv ]; then (python -m venv .venv || virtualenv .venv); fi
	@source .venv/bin/activate && \
	pip install --upgrade pip && pip install pytest requests && \
	pytest -v python_tester && \
	deactivate

test_build: $(BUILD_DIR)/CMakeCache.txt
	@cmake --build $(BUILD_DIR) --target $(TEST_TARGETS)

gtest: test_build
	@echo -e "\n\\e[1;34m[INFO]\\e[0m Running CTest for all registered tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@for test in $(TEST_TARGETS); do \
		echo -e "\n\\e[1;34m[INFO]\\e[0m Running $$test..."; \
		$(BUILD_DIR)/$$test && echo -e "\\e[1;32m[SUCCESS]\\e[0m $$test passed!" || echo -e "\\e[1;31m[FAILURE]\\e[0m $$test failed!"; \
	done

clean:
	@if [ -d ./$(OBJ_DIR) ]; then rm -rf $(OBJ_DIR); fi
	@if [ -d ./build ]; then rm -rf $(BUILD_DIR); fi

fclean: clean
	rm -f $(NAME)
	rm -rf .venv
	rm -rf www/logs/*
	rm -rf www/secondary/uploads/*

re:	fclean all

.PHONY: all clean fclean re test google_test test_build

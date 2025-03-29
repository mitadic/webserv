NAME	=	webserv
CXX		=	c++
FLAGS	=	-Wall -Werror -Wextra -g -std=c++98
# FLAGS	+= -fsanitize=address

SRC_DIR =	./src/
OBJ_DIR	=	./obj/
I_DIR	=	./incl/
TEST_DIR = 	./test/
BUILD_DIR = ./build/

TEST_EXEC =	$(BUILD_DIR)test_utils

SRC		=	Config.cpp \
			main.cpp \
			ConfigUtils.cpp \
			Location.cpp \
			Log.cpp \
			ServerEngine.cpp \
			CgiHandler.cpp \
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

TEST	=	test_utils.cpp \
			test_http.cpp

OBJ		=	$(SRC:cpp=o)
SRC_FILES = $(addprefix $(SRC_DIR), $(SRC))
OBJ_FILES = $(addprefix $(OBJ_DIR), $(OBJ))
TEST_FILES = $(addprefix $(TEST_DIR), $(TEST))

TEST_SRC_FILES = $(TEST_FILES) $(SRC_FILES)

all: $(NAME)

$(NAME): $(OBJ_FILES)
	$(CXX) $(FLAGS) $^ -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(I_DIR)*
	@if [ ! -d $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi
	$(CXX) $(FLAGS) -I$(I_DIR) -c $< -o $@

$(TEST_EXEC): $(TEST_SRC_FILES) CMakeLists.txt
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR) --target test_utils
	@cmake --build $(BUILD_DIR) --target test_http

test: $(TEST_EXEC)
	@echo "\033[1;34m[INFO]\033[0m Running CTest for all registered tests..."
	@cd $(BUILD_DIR) && ctest --output-on-failure
	@echo "\033[1;34m[INFO]\033[0m Running Utils unit test..."
	@$(BUILD_DIR)test_utils && echo "\033[1;32m[SUCCESS]\033[0m test_utils passed!" || echo "\033[1;31m[FAILURE]\033[0m test_utils failed!"
	@echo "\033[1;34m[INFO]\033[0m Running HTTP tests with cURL..."
	@$(BUILD_DIR)test_http && echo "\033[1;32m[SUCCESS]\033[0m test_http passed!" || echo "\033[1;31m[FAILURE]\033[0m test_http failed!"

clean:
	@if [ -d ./$(OBJ_DIR) ]; then rm -rf $(OBJ_DIR); fi
	@if [ -d ./build ]; then rm -rf $(BUILD_DIR); fi

fclean: clean
	rm -f $(NAME)

re:	fclean all

.PHONY: all clean fclean re

CXX		=	c++
# FLAGS	=	-Wall -Werror -Wextra -g -std=c++98
FLAGS	=	-g -std=c++98
SRC		=	main.cpp \
			ServerEngine.cpp \
			CgiHandler.cpp
SRC_DIR =	./src/
OBJ		=	$(SRC:cpp=o)
OBJ_DIR	=	./obj/
I_DIR	=	./incl/
NAME	=	a.out


all: $(NAME)

$(NAME): $(addprefix $(OBJ_DIR),$(OBJ))
	$(CXX) $(addprefix $(OBJ_DIR),$(OBJ)) -o $(NAME)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@if [ ! -d $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi
	$(CXX) $(FLAGS) -I$(I_DIR) -c $< -o $@

clean:
	@if [ -d ./$(OBJ_DIR) ]; then rm -rf $(OBJ_DIR); fi

fclean: clean
	rm -f $(NAME)

re:	fclean all

.PHONY: all clean fclean re

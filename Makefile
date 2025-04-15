CXX		=	c++
FLAGS	=	-Wall -Werror -Wextra -g -std=c++98
#FLAGS	=	-g -std=c++98
#FLAGS	+=	-Wall -Wextra -Werror
# FLAGS	+= -fsanitize=address
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

SRC_DIR =	./src/
OBJ		=	$(SRC:cpp=o)
OBJ_DIR	=	./obj/
I_DIR	=	./incl/
NAME	=	a.out

all: $(NAME)

$(NAME): $(addprefix $(OBJ_DIR),$(OBJ))
	$(CXX) $(FLAGS) $(addprefix $(OBJ_DIR),$(OBJ)) -o $(NAME)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(I_DIR)*
	@if [ ! -d $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi
	$(CXX) $(FLAGS) -I$(I_DIR) -c $< -o $@

clean:
	@if [ -d ./$(OBJ_DIR) ]; then rm -rf $(OBJ_DIR); fi

fclean: clean
	rm -f $(NAME)
	rm -rf www/three-socketeers/uploads/*
	rm -rf www/cgi-bin/guest-book/*
	rm -rf logs/*
	rm -rf www/secondary/uploads/*

re:	fclean all

.PHONY: all clean fclean re

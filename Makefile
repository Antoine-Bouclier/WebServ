NAME        =   webserv
CXX          =   c++
CFLAGS      =   -Wall -Wextra -Werror -std=c++98 -MMD

SRC_DIR     =   src/
OBJ_DIR     =   obj/
INC_DIR     =   inc/

SRC_FILES   =   main.cpp \
                config/ConfigParser.cpp \
                config/Lexer.cpp

OBJ         =   $(addprefix $(OBJ_DIR), $(SRC_FILES:.cpp=.o))
DEP         =   $(OBJ:.o=.d)

INCLUDES    =   -I $(INC_DIR)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ -c $<

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re
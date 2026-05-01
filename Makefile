.PHONY : all clean fclean re

NAME := webserv

# ╭━━━━━━━━━━━━══════════╕出 ❖ BASICS VARIABLES ❖ 力╒═══════════━━━━━━━━━━━━╮ #

CXX				:=	c++

CXXFLAGS		:=	-Wall -Wextra -Werror -MMD -std=c++98

RM				:=	rm	-rf

MKDIR			:=	mkdir -p

SHOW_MSG_CLEAN	=	true

MAKEFLAGS		+=	--no-print-directory

# ╰━━━━━━━━━━━━━━━━════════════════╛出 ❖ 力╘════════════════━━━━━━━━━━━━━━━━╯ #

# ╭━━━━━━━━━━━━══════════╕出 ❖ FILE TREE ❖ 力╒═══════════━━━━━━━━━━━━╮ #

D_SRC		=	src/
D_INC		=	inc/

D_BIN		=	.bin/

vpath %.cpp $(D_SRC):$(CURDIR):$(D_SRC)config:$(D_SRC)semantic

# file lists
SRCS		=	main.cpp			\
				Lexer.cpp			\
				checker.cpp			\
				AConfig.cpp			\
				ConfigParser.cpp	\
				ConfigServer.cpp	\
				ConfigLocation.cpp	\
				ConfigParserHandlers.cpp

OBJS		=	$(addprefix $(D_BIN), $(SRCS:.cpp=.o))
DEPS		=	$(addprefix $(D_BIN), $(SRCS:.cpp=.d))

INC			=	-I$(D_INC) -I$(D_INC)config/

# ╭━━━━━━━━━━━━══════════╕出 ❖ RULES ❖ 力╒═══════════━━━━━━━━━━━━╮ #

all:	$(NAME)

$(NAME):	$(OBJS) | Makefile $(D_BIN)
	@$(CXX) $(CXXFLAGS) $(INC) $(OBJS) -o $(NAME)
	@echo "\e[1;32m🎍 $(NAME) program created successfully ! 🎍\e[0m"

$(D_BIN):
	@$(MKDIR) $@

$(D_BIN)%.o: %.cpp | Makefile $(D_BIN)
	@echo "\e[1m🫧 Compiling $< 🫧\e[0m"
	@$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	@$(RM) $(D_BIN)
	@echo "\e[1;36m🫗 Deleted $(NAME) object files 🫗\e[0m"

fclean:
	@$(MAKE) -s SHOW_MSG_CLEAN=false clean
	@$(RM) $(NAME)
	@echo "\e[1;34m🧼 $(NAME) executable deleted ! 🧼\e[0m"

re:
	@$(MAKE) fclean
	@$(MAKE) all

valgrind:
	@$(MAKE)
	@clear
	@valgrind										\
		--leak-check=full								\
		--show-leak-kinds=all							\
		--track-origins=yes 							\
		--track-fds=yes									\
		./$(NAME)

-include $(DEPS)

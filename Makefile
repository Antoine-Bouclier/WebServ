SERVER_NAME	=	server
CLIENT_NAME	=	client

CXX			=	c++
CFLAGS		=	-Wall -Wextra -Werror -std=c++98 -MMD

SRC_DIR		=	src/
OBJ_DIR		=	obj/
INC_DIR		=	inc/

SERVER_SRC_FILES	=	server.cpp
CLIENT_SRC_FILES	=	client.cpp

SERVER_OBJ_FILES	=	$(SERVER_SRC_FILES:.cpp=.o)
CLIENT_OBJ_FILES	=	$(CLIENT_SRC_FILES:.cpp=.o)

SERVER_SRC	=	$(addprefix $(SRC_DIR), $(SERVER_SRC_FILES))
CLIENT_SRC	=	$(addprefix $(SRC_DIR), $(CLIENT_SRC_FILES))

SERVER_OBJ	=	$(addprefix $(OBJ_DIR), $(SERVER_OBJ_FILES))
CLIENT_OBJ	=	$(addprefix $(OBJ_DIR), $(CLIENT_OBJ_FILES))

SERVER_DEP	=	$(SERVER_OBJ:.o=.d)
CLIENT_DEP	=	$(CLIENT_OBJ:.o=.d)

all: $(SERVER_NAME) $(CLIENT_NAME)

$(SERVER_NAME): $(SERVER_OBJ)
	$(CXX) $(CFLAGS) $(SERVER_OBJ) -o $(SERVER_NAME)

$(CLIENT_NAME): $(CLIENT_OBJ)
	$(CXX) $(CFLAGS) $(CLIENT_OBJ) -o $(CLIENT_NAME)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I $(INC_DIR) -o $@ -c $<

clean:
	@if [ -d "$(OBJ_DIR)" ]; then \
		rm -rf $(OBJ_DIR); \
	fi

fclean: clean
	@if [ -f "$(SERVER_NAME)" ]; then \
		rm -f $(SERVER_NAME); \
	fi
	@if [ -f "$(CLIENT_NAME)" ]; then \
		rm -f $(CLIENT_NAME); \
	fi

re: fclean all

-include $(SERVER_DEP)
-include $(CLIENT_DEP)

.PHONY: all clean fclean re
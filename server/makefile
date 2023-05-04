

NAME = server

#---------- SRC ----------
SCR_DIR = src
SRC = $(wildcard $(SCR_DIR)/*.c)

#---------- OBJ ----------
OBJ_DIR = obj
OBJ = $(patsubst $(SCR_DIR)/%,$(OBJ_DIR)/%,$(SRC:.c=.o))

#---------- HEAD ----------
HEAD_DIR = include
HEAD = $(wildcard $(HEAD_DIR)/*.h)

#---------- FLAGS ----------
CC = gcc
HEAD_FLAG = -I$(HEAD_DIR)
ERR_FLAG = -Wall -Wextra
CFLAGS =  $(ERR_FLAG) $(HEAD_FLAG)
LDLIBS = -lpthread -lrt


#---------- RULES ----------
.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SCR_DIR)/%.c $(HEAD)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean 
	rm -f $(NAME)

re: fclean all
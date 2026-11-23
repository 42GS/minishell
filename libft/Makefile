NAME		= libft.a

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -I include

SRC_DIR		= src
OBJ_DIR		= obj

SRCS		= $(SRC_DIR)/ft_strlen.c \
			  $(SRC_DIR)/ft_strdup.c \
			  $(SRC_DIR)/ft_substr.c \
			  $(SRC_DIR)/ft_strjoin.c \
			  $(SRC_DIR)/ft_strcmp.c \
			  $(SRC_DIR)/ft_strchr.c \
			  $(SRC_DIR)/ft_calloc.c \
			  $(SRC_DIR)/ft_put_fd.c \
			  $(SRC_DIR)/ft_ischar.c \
			  $(SRC_DIR)/ft_itoa.c \
			  $(SRC_DIR)/ft_split.c \
			  $(SRC_DIR)/ft_strbuf.c

OBJS		= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	@ar rcs $(NAME) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c include/libft.h | $(OBJ_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

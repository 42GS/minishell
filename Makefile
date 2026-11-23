NAME		= minishell

CC			= cc
CFLAGS		= -Wall -Wextra -Werror

UNAME_S		:= $(shell uname -s)
RL_INC		:=
RL_LIB		:= -lreadline

ifeq ($(UNAME_S), Darwin)
	BREW_RL	:= $(shell brew --prefix readline 2>/dev/null)
	ifneq ($(BREW_RL),)
		RL_INC	:= -I$(BREW_RL)/include
		RL_LIB	:= -L$(BREW_RL)/lib -lreadline
	endif
endif

LIBFT_DIR	= libft
LIBFT		= $(LIBFT_DIR)/libft.a
LIBFT_SRCS	= $(LIBFT_DIR)/src/ft_strlen.c \
			  $(LIBFT_DIR)/src/ft_strdup.c \
			  $(LIBFT_DIR)/src/ft_substr.c \
			  $(LIBFT_DIR)/src/ft_strjoin.c \
			  $(LIBFT_DIR)/src/ft_strcmp.c \
			  $(LIBFT_DIR)/src/ft_strchr.c \
			  $(LIBFT_DIR)/src/ft_calloc.c \
			  $(LIBFT_DIR)/src/ft_put_fd.c \
			  $(LIBFT_DIR)/src/ft_ischar.c \
			  $(LIBFT_DIR)/src/ft_itoa.c \
			  $(LIBFT_DIR)/src/ft_split.c

INC_FLAGS	= -I include -I $(LIBFT_DIR)/include $(RL_INC)

SRC_DIR		= src
OBJ_DIR		= obj

SRCS		= $(SRC_DIR)/main.c \
			  $(SRC_DIR)/shell_utils.c \
			  $(SRC_DIR)/lexer/lexer.c \
			  $(SRC_DIR)/lexer/lexer_word.c \
			  $(SRC_DIR)/lexer/lexer_op.c \
			  $(SRC_DIR)/lexer/lexer_token.c \
			  $(SRC_DIR)/parser/parser.c \
			  $(SRC_DIR)/parser/parser_cmd.c \
			  $(SRC_DIR)/parser/parser_ast.c \
			  $(SRC_DIR)/parser/parser_utils.c \
			  $(SRC_DIR)/executor/executor.c \
			  $(SRC_DIR)/executor/executor_pipe.c \
			  $(SRC_DIR)/executor/executor_pipe_child.c \
			  $(SRC_DIR)/executor/executor_wait.c \
			  $(SRC_DIR)/executor/executor_path.c \
			  $(SRC_DIR)/executor/executor_path_utils.c \
			  $(SRC_DIR)/executor/executor_redir.c \
			  $(SRC_DIR)/executor/executor_heredoc.c \
			  $(SRC_DIR)/executor/executor_heredoc_child.c \
			  $(SRC_DIR)/expander/expander.c \
			  $(SRC_DIR)/expander/expander_utils.c \
			  $(SRC_DIR)/expander/expander_split.c \
			  $(SRC_DIR)/expander/expander_cmd.c \
			  $(SRC_DIR)/env/env_core.c \
			  $(SRC_DIR)/env/env_ops.c \
			  $(SRC_DIR)/env/env_init.c \
			  $(SRC_DIR)/env/env_array.c \
			  $(SRC_DIR)/builtins/builtins.c \
			  $(SRC_DIR)/builtins/builtin_echo.c \
			  $(SRC_DIR)/builtins/builtin_cd.c \
			  $(SRC_DIR)/builtins/builtin_pwd.c \
			  $(SRC_DIR)/builtins/builtin_export.c \
			  $(SRC_DIR)/builtins/builtin_export_print.c \
			  $(SRC_DIR)/builtins/builtin_unset.c \
			  $(SRC_DIR)/builtins/builtin_env.c \
			  $(SRC_DIR)/builtins/builtin_exit.c \
			  $(SRC_DIR)/signals/signals.c \
			  $(SRC_DIR)/signals/signals_term.c

OBJS		= $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

HEADERS		= include/minishell.h \
			  include/lexer.h \
			  include/parser.h \
			  include/executor.h \
			  include/builtins.h \
			  include/signals.h

all: $(NAME)

$(NAME): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) $(RL_LIB) -o $(NAME)

$(LIBFT): $(LIBFT_SRCS) $(LIBFT_DIR)/include/libft.h
	@$(MAKE) -C $(LIBFT_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@

bonus: all

clean:
	@$(MAKE) -C $(LIBFT_DIR) clean
	@rm -rf $(OBJ_DIR)

fclean: clean
	@$(MAKE) -C $(LIBFT_DIR) fclean
	@rm -f $(NAME)

re: fclean all

.PHONY: all bonus clean fclean re

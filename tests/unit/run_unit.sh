#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

make -C libft > /dev/null

RL_FLAGS="-lreadline"
if [ "$(uname -s)" = "Darwin" ] && command -v brew >/dev/null 2>&1; then
    BREW_RL="$(brew --prefix readline 2>/dev/null || true)"
    if [ -n "$BREW_RL" ]; then
        RL_FLAGS="-I${BREW_RL}/include -L${BREW_RL}/lib -lreadline"
    fi
fi

run_with_leak_check() {
    local bin="$1"
    if command -v valgrind >/dev/null 2>&1; then
        valgrind \
            --leak-check=full \
            --show-leak-kinds=all \
            --errors-for-leak-kinds=all \
            --error-exitcode=42 \
            "$bin"
    else
        "$bin"
    fi
}

cc -Wall -Wextra -Werror -I libft/include \
    tests/unit/test_libft.c libft/libft.a \
    -o /tmp/minishell_unit_libft
run_with_leak_check /tmp/minishell_unit_libft
rm -f /tmp/minishell_unit_libft

cc -Wall -Wextra -Werror -I include -I libft/include \
    tests/unit/test_lexer.c \
    src/lexer/lexer.c src/lexer/lexer_word.c \
    src/lexer/lexer_op.c src/lexer/lexer_token.c \
    libft/libft.a \
    -o /tmp/minishell_unit_lexer
run_with_leak_check /tmp/minishell_unit_lexer
rm -f /tmp/minishell_unit_lexer

cc -Wall -Wextra -Werror -I include -I libft/include \
    tests/unit/test_parser.c \
    src/lexer/lexer.c src/lexer/lexer_word.c \
    src/lexer/lexer_op.c src/lexer/lexer_token.c \
    src/parser/parser.c src/parser/parser_cmd.c \
    src/parser/parser_ast.c src/parser/parser_utils.c \
    libft/libft.a \
    -o /tmp/minishell_unit_parser
run_with_leak_check /tmp/minishell_unit_parser
rm -f /tmp/minishell_unit_parser

CORE_SRCS="src/shell_utils.c \
    src/lexer/lexer.c src/lexer/lexer_word.c \
    src/lexer/lexer_op.c src/lexer/lexer_token.c \
    src/parser/parser.c src/parser/parser_cmd.c \
    src/parser/parser_ast.c src/parser/parser_utils.c \
    src/executor/executor.c src/executor/executor_pipe.c \
    src/executor/executor_pipe_child.c src/executor/executor_wait.c \
    src/executor/executor_path.c src/executor/executor_redir.c \
    src/executor/executor_heredoc.c src/executor/executor_heredoc_child.c \
    src/expander/expander.c src/expander/expander_utils.c \
    src/env/env_core.c src/env/env_ops.c \
    src/env/env_init.c src/env/env_array.c \
    src/builtins/builtins.c src/builtins/builtin_echo.c \
    src/builtins/builtin_cd.c src/builtins/builtin_pwd.c \
    src/builtins/builtin_export.c src/builtins/builtin_export_print.c \
    src/builtins/builtin_unset.c src/builtins/builtin_env.c \
    src/builtins/builtin_exit.c \
    src/signals/signals.c src/signals/signals_term.c"

cc -Wall -Wextra -Werror -I include -I libft/include \
    tests/unit/test_executor.c $CORE_SRCS libft/libft.a $RL_FLAGS \
    -o /tmp/minishell_unit_executor
run_with_leak_check /tmp/minishell_unit_executor
rm -f /tmp/minishell_unit_executor

cc -Wall -Wextra -Werror -I include -I libft/include \
    tests/unit/test_redir.c $CORE_SRCS libft/libft.a $RL_FLAGS \
    -o /tmp/minishell_unit_redir
run_with_leak_check /tmp/minishell_unit_redir
rm -f /tmp/minishell_unit_redir

cc -Wall -Wextra -Werror -I include -I libft/include \
    tests/unit/test_builtins.c $CORE_SRCS libft/libft.a $RL_FLAGS \
    -o /tmp/minishell_unit_builtins
run_with_leak_check /tmp/minishell_unit_builtins
rm -f /tmp/minishell_unit_builtins

cc -Wall -Wextra -Werror -I include -I libft/include \
    tests/unit/test_signals.c $CORE_SRCS libft/libft.a $RL_FLAGS \
    -o /tmp/minishell_unit_signals
run_with_leak_check /tmp/minishell_unit_signals
rm -f /tmp/minishell_unit_signals

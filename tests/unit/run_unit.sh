#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

make -C libft > /dev/null

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

cc -Wall -Wextra -Werror -I include -I libft/include \
    tests/unit/test_executor.c \
    src/shell_utils.c \
    src/lexer/lexer.c src/lexer/lexer_word.c \
    src/lexer/lexer_op.c src/lexer/lexer_token.c \
    src/parser/parser.c src/parser/parser_cmd.c \
    src/parser/parser_ast.c src/parser/parser_utils.c \
    src/executor/executor.c src/executor/executor_pipe.c \
    src/executor/executor_pipe_child.c src/executor/executor_path.c \
    src/executor/executor_redir_stub.c \
    src/env/env_core.c src/env/env_array.c \
    src/builtins/builtins_stub.c \
    libft/libft.a \
    -o /tmp/minishell_unit_executor
run_with_leak_check /tmp/minishell_unit_executor
rm -f /tmp/minishell_unit_executor

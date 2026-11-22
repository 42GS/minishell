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

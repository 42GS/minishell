#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

make -C libft > /dev/null

cc -Wall -Wextra -Werror -I libft/include \
    tests/unit/test_libft.c libft/libft.a \
    -o /tmp/minishell_unit_libft

if command -v valgrind >/dev/null 2>&1; then
    valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --errors-for-leak-kinds=all \
        --error-exitcode=42 \
        /tmp/minishell_unit_libft
else
    /tmp/minishell_unit_libft
fi

rm -f /tmp/minishell_unit_libft

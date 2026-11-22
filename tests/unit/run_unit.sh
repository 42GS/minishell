#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

make -C libft > /dev/null

cc -Wall -Wextra -Werror -I libft/include \
    tests/unit/test_libft.c libft/libft.a \
    -o /tmp/minishell_unit_libft

/tmp/minishell_unit_libft
rm -f /tmp/minishell_unit_libft

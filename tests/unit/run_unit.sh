#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"
make -j"$JOBS" all > /dev/null

CORE_ARCHIVE="/tmp/libms_core_$$.a"
trap 'rm -f "$CORE_ARCHIVE" "/tmp/minishell_unit_"*"_$$"' EXIT

CORE_OBJS="$(find obj -name '*.o' ! -name 'main.o' | sort)"
rm -f "$CORE_ARCHIVE"
ar rcs "$CORE_ARCHIVE" $CORE_OBJS

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

run_unit_suite() {
    local name="$1"
    local src="$2"
    shift 2
    local bin="/tmp/minishell_unit_${name}_$$"

    cc -Wall -Wextra -Werror -I include -I libft/include "$src" "$@" -o "$bin"
    run_with_leak_check "$bin"
    rm -f "$bin"
}

run_unit_suite libft tests/unit/test_libft.c libft/libft.a
run_unit_suite lexer tests/unit/test_lexer.c "$CORE_ARCHIVE" libft/libft.a $RL_FLAGS
run_unit_suite parser tests/unit/test_parser.c "$CORE_ARCHIVE" libft/libft.a $RL_FLAGS
run_unit_suite executor tests/unit/test_executor.c "$CORE_ARCHIVE" libft/libft.a $RL_FLAGS
run_unit_suite redir tests/unit/test_redir.c "$CORE_ARCHIVE" libft/libft.a $RL_FLAGS
run_unit_suite builtins tests/unit/test_builtins.c "$CORE_ARCHIVE" libft/libft.a $RL_FLAGS
run_unit_suite signals tests/unit/test_signals.c "$CORE_ARCHIVE" libft/libft.a $RL_FLAGS

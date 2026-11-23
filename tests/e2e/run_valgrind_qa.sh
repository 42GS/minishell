#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

if [ ! -x "./minishell" ]; then
    echo "[ERROR] ./minishell not found. Run 'make' first."
    exit 1
fi

if ! command -v valgrind >/dev/null 2>&1; then
    echo "[ERROR] valgrind is required for QA leak audit."
    exit 1
fi

LOG="/tmp/ms_vg_qa.log"
FD_LOG="/tmp/ms_vg_fd.log"
BASE_LOG="/tmp/ms_vg_base.log"
PASSED=0

valgrind --track-fds=yes --log-file="$BASE_LOG" /bin/true > /dev/null 2>&1 || true
BASE_FDS="$(grep -oE "FILE DESCRIPTORS: [0-9]+" "$BASE_LOG" | tail -n 1 | awk '{print $3}')"
BASE_FDS="${BASE_FDS:-3}"
rm -f "$BASE_LOG"

run_vg_case() {
    local name="$1"
    local expected_rc="$2"
    local script="$3"
    local rc=0
    local open_fds

    set +e
    printf "%b" "$script" | valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --errors-for-leak-kinds=all \
        --suppressions=tests/readline.supp \
        --error-exitcode=42 \
        --log-file="$LOG" \
        ./minishell > /dev/null 2>&1
    rc=$?
    set -e

    if [ "$rc" -ne "$expected_rc" ]; then
        echo "[FAIL] Valgrind QA ($name): expected rc=$expected_rc, got rc=$rc"
        cat "$LOG"
        rm -f "$LOG" "$FD_LOG"
        exit 1
    fi
    if ! grep -q "ERROR SUMMARY: 0 errors" "$LOG"; then
        echo "[FAIL] Valgrind QA ($name): Valgrind errors reported"
        cat "$LOG"
        rm -f "$LOG" "$FD_LOG"
        exit 1
    fi

    set +e
    printf "%b" "$script" | valgrind \
        --track-fds=yes \
        --log-file="$FD_LOG" \
        ./minishell > /dev/null 2>&1
    set -e

    open_fds="$(grep -oE "FILE DESCRIPTORS: [0-9]+" "$FD_LOG" | tail -n 1 | awk '{print $3}')"
    open_fds="${open_fds:-$BASE_FDS}"
    if [ "$open_fds" -gt "$BASE_FDS" ] || grep -Eq "Open file descriptor ([3-9]|[1-9][0-9]+):" "$FD_LOG"; then
        echo "[FAIL] Valgrind QA ($name): leaked FDs (open=$open_fds > base=$BASE_FDS)"
        cat "$FD_LOG"
        rm -f "$LOG" "$FD_LOG"
        exit 1
    fi
    echo "  [PASS] $name (rc=$rc, open_fds=$open_fds/$BASE_FDS, 0 errors)"
    PASSED=$((PASSED + 1))
}

echo "[VALGRIND-QA] Baseline inherited FDs: $BASE_FDS"

run_vg_case "mandatory_pipeline_and_builtins" 0 \
    "/bin/echo vg_pipe | tr a-z A-Z | cat\nexport VG_K=a_b\necho \$VG_K \$?\nunset VG_K\ncd ..\npwd\ncd -\npwd\nenv\necho -n done\n"

run_vg_case "builtin_with_redir" 0 \
    "echo hi > /tmp/ms_vg_bi.txt\ncat < /tmp/ms_vg_bi.txt\nrm -f /tmp/ms_vg_bi.txt\n"

run_vg_case "redirs_and_ambiguous" 0 \
    "/bin/echo l1 > /tmp/ms_vg_r.txt\n/bin/echo l2 >> /tmp/ms_vg_r.txt\ncat < /tmp/ms_vg_r.txt\ncat < /tmp/no_such_vg_999\nexport VG_S=\"a b\"\necho hi > \$VG_S\nrm -f /tmp/ms_vg_r.txt\ntrue\n"

run_vg_case "heredocs_and_pipe_fd" 0 \
    "cat << EOF\nhd1 \$USER\nEOF\ncat << 'QEOF'\n\$USER\nQEOF\ncat << H1 << H2 | tr a-z A-Z\nfirst\nH1\nsecond\nH2\n"

run_vg_case "bonus_logic_and_subshells" 0 \
    "true && echo ok1\nfalse || echo ok2\nfalse | true && echo p1\n( ( echo deep ) )\nexport VG_ISO=parent\n(export VG_ISO=child && cd .. && echo \$VG_ISO)\n(echo a && echo b) | tr a-z A-Z\n(cat << A && cat << B)\nx\nA\ny\nB\n(exit 130) || true\n"

run_vg_case "bonus_wildcards_cwd" 0 \
    "echo *\necho Make*\necho .*\necho \"*\" '*' \"Make\"* Make\"*\"\necho nonexistent_vg_*\nwc -l < Make*\ncat < * || true\n"

run_vg_case "syntax_error_pipe" 2 "cat | | wc\n"
run_vg_case "syntax_error_triple_amp" 2 "echo a &&& echo b\n"
run_vg_case "syntax_error_unclosed_quote" 2 "echo \"unclosed\n"
run_vg_case "exec_not_found_127" 127 "nonexistent_vg_cmd\n"
run_vg_case "exec_directory_126" 126 "/tmp\n"
run_vg_case "exit_mod_259" 3 "exit 259\n"
run_vg_case "exit_non_num_2" 2 "exit not_a_num\n"

rm -f "$LOG" "$FD_LOG"
echo "[VALGRIND-QA] All $PASSED scenarios PASSED with 0 memory leaks and 0 FD leaks!"

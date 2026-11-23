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

TMP_DIR="$(mktemp -d /tmp/ms_vg_qa.XXXXXX)"
trap 'rm -rf "$TMP_DIR"' EXIT

BASE_LOG="$TMP_DIR/base.log"
valgrind --track-fds=yes --log-file="$BASE_LOG" /bin/true > /dev/null 2>&1 || true
BASE_FDS="$(grep -oE "FILE DESCRIPTORS: [0-9]+" "$BASE_LOG" | tail -n 1 | awk '{print $3}')"
BASE_FDS="${BASE_FDS:-3}"

JOBS="$(nproc 2>/dev/null || echo 4)"
PIDS=()
NAMES=()

run_vg_case() {
    local name="$1"
    local expected_rc="$2"
    local script="$3"
    local log="$TMP_DIR/${name}.mem.log"
    local fd_log="$TMP_DIR/${name}.fd.log"
    local out="$TMP_DIR/${name}.out"
    local rc=0
    local open_fds

    set +e
    printf "%b" "$script" | valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --errors-for-leak-kinds=all \
        --suppressions=tests/readline.supp \
        --error-exitcode=42 \
        --log-file="$log" \
        ./minishell > /dev/null 2>&1
    rc=$?
    set -e

    if [ "$rc" -ne "$expected_rc" ]; then
        {
            echo "[FAIL] Valgrind QA ($name): expected rc=$expected_rc, got rc=$rc"
            cat "$log"
        } > "$out"
        return 1
    fi
    if ! grep -q "ERROR SUMMARY: 0 errors" "$log"; then
        {
            echo "[FAIL] Valgrind QA ($name): Valgrind errors reported"
            cat "$log"
        } > "$out"
        return 1
    fi

    set +e
    printf "%b" "$script" | valgrind \
        --track-fds=yes \
        --log-file="$fd_log" \
        ./minishell > /dev/null 2>&1
    set -e

    open_fds="$(grep -oE "FILE DESCRIPTORS: [0-9]+" "$fd_log" | tail -n 1 | awk '{print $3}')"
    open_fds="${open_fds:-$BASE_FDS}"
    if [ "$open_fds" -gt "$BASE_FDS" ] || grep -E "Open file descriptor ([3-9]|[1-9][0-9]+):" "$fd_log" | grep -qvF "$fd_log"; then
        {
            echo "[FAIL] Valgrind QA ($name): leaked FDs (open=$open_fds > base=$BASE_FDS)"
            cat "$fd_log"
        } > "$out"
        return 1
    fi
    echo "  [PASS] $name (rc=$rc, open_fds=$open_fds/$BASE_FDS, 0 errors)" > "$out"
    return 0
}

enqueue_vg_case() {
    local name="$1"
    while [ "$(jobs -pr | wc -l)" -ge "$JOBS" ]; do
        sleep 0.05
    done
    run_vg_case "$@" &
    PIDS+=("$!")
    NAMES+=("$name")
}

echo "[VALGRIND-QA] Baseline inherited FDs: $BASE_FDS (workers=$JOBS)"

enqueue_vg_case "mandatory_pipeline_and_builtins" 0 \
    "/bin/echo vg_pipe | tr a-z A-Z | cat\nexport VG_K=a_b\necho \$VG_K \$?\nunset VG_K\ncd ..\npwd\ncd -\npwd\nenv\necho -n done\n"

enqueue_vg_case "builtin_with_redir" 0 \
    "echo hi > /tmp/ms_vg_bi.txt\ncat < /tmp/ms_vg_bi.txt\nrm -f /tmp/ms_vg_bi.txt\n"

enqueue_vg_case "redirs_and_ambiguous" 0 \
    "/bin/echo l1 > /tmp/ms_vg_r.txt\n/bin/echo l2 >> /tmp/ms_vg_r.txt\ncat < /tmp/ms_vg_r.txt\ncat < /tmp/no_such_vg_999\nexport VG_S=\"a b\"\necho hi > \$VG_S\nrm -f /tmp/ms_vg_r.txt\ntrue\n"

enqueue_vg_case "heredocs_and_pipe_fd" 0 \
    "cat << EOF\nhd1 \$USER\nEOF\ncat << 'QEOF'\n\$USER\nQEOF\ncat << H1 << H2 | tr a-z A-Z\nfirst\nH1\nsecond\nH2\n"

enqueue_vg_case "bonus_logic_and_subshells" 0 \
    "true && echo ok1\nfalse || echo ok2\nfalse | true && echo p1\n( ( echo deep ) )\nexport VG_ISO=parent\n(export VG_ISO=child && cd .. && echo \$VG_ISO)\n(echo a && echo b) | tr a-z A-Z\n(cat << A && cat << B)\nx\nA\ny\nB\n(exit 130) || true\n"

enqueue_vg_case "bonus_wildcards_cwd" 0 \
    "echo *\necho Make*\necho .*\necho \"*\" '*' \"Make\"* Make\"*\"\necho nonexistent_vg_*\nwc -l < Make*\ncat < * || true\n"

enqueue_vg_case "syntax_error_pipe" 2 "cat | | wc\n"
enqueue_vg_case "syntax_error_triple_amp" 2 "echo a &&& echo b\n"
enqueue_vg_case "syntax_error_unclosed_quote" 2 "echo \"unclosed\n"
enqueue_vg_case "exec_not_found_127" 127 "nonexistent_vg_cmd\n"
enqueue_vg_case "exec_directory_126" 126 "/tmp\n"
enqueue_vg_case "exit_mod_259" 3 "exit 259\n"
enqueue_vg_case "exit_non_num_2" 2 "exit not_a_num\n"

FAILED=0
PASSED=0
for idx in "${!PIDS[@]}"; do
    set +e
    wait "${PIDS[$idx]}"
    wrc=$?
    set -e
    name="${NAMES[$idx]}"
    if [ -f "$TMP_DIR/${name}.out" ]; then
        cat "$TMP_DIR/${name}.out"
    fi
    if [ "$wrc" -ne 0 ]; then
        FAILED=$((FAILED + 1))
    else
        PASSED=$((PASSED + 1))
    fi
done

if [ "$FAILED" -ne 0 ]; then
    exit 1
fi
echo "[VALGRIND-QA] All $PASSED scenarios PASSED with 0 memory leaks and 0 FD leaks!"

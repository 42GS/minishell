#!/usr/bin/env python3
"""
Minishell E2E Differential Test Runner (Bash vs Minishell)
Compares stdout, exit codes, and checks for abnormal signals/crashes.
"""

import argparse
import glob
import json
import os
import subprocess
import sys

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
MINISHELL_BIN = os.path.join(ROOT_DIR, "minishell")
SUITES_DIR = os.path.join(os.path.dirname(__file__), "suites")


def run_shell(cmd_bin: str, script: str, timeout: int = 5):
    proc = subprocess.run(
        [cmd_bin],
        input=script,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        timeout=timeout,
        cwd=ROOT_DIR,
    )
    return proc.returncode, proc.stdout, proc.stderr


def main():
    parser = argparse.ArgumentParser(description="Minishell E2E Differential Runner")
    parser.add_argument("--ci", action="store_true", help="Run smoke cases only for scaffolding/CI")
    parser.add_argument("--all", action="store_true", help="Run full 42evalhub suite")
    parser.add_argument("--suite", type=str, help="Run a specific suite JSON filename")
    args = parser.parse_args()

    if not os.path.exists(MINISHELL_BIN):
        print(f"[ERROR] Binary not found: {MINISHELL_BIN}. Run 'make' first.")
        sys.exit(1)

    if args.ci:
        smoke_cases = [
            ("empty_eof", ""),
            ("simple_abs_cmd", "/bin/echo minishell_ci_ok\n"),
            ("multi_pipeline", "/bin/echo pipe_ok | tr a-z A-Z | cat\n"),
            ("cmd_not_found_127", "nonexistent_ci_cmd_xyz\n"),
            ("syntax_error_2", "cat | | wc\n"),
        ]
        for name, script in smoke_cases:
            b_rc, b_out, _ = run_shell("bash", script)
            m_rc, m_out, m_err = run_shell(MINISHELL_BIN, script)
            if m_rc != b_rc or m_out != b_out:
                print(
                    f"[FAIL] CI Smoke ({name}): expected rc={b_rc} out={repr(b_out)}, "
                    f"got rc={m_rc} out={repr(m_out)} err={repr(m_err)}"
                )
                sys.exit(1)
        print(f"[E2E-CI] All {len(smoke_cases)} smoke checks PASSED (matched Bash).")
        return

    suite_files = sorted(glob.glob(os.path.join(SUITES_DIR, "*.json")))
    if args.suite:
        suite_files = [os.path.join(SUITES_DIR, args.suite)]

    total = 0
    passed = 0
    failed = 0

    for sf in suite_files:
        with open(sf, "r", encoding="utf-8") as f:
            data = json.load(f)
        suite_name = data.get("suite", os.path.basename(sf))
        print(f"\n=== Running Suite: {suite_name} ===")
        for case in data.get("cases", []):
            total += 1
            name = case["name"]
            cmd_input = case["input"]
            bash_rc, bash_out, _ = run_shell("bash", cmd_input)
            mini_rc, mini_out, mini_err = run_shell(MINISHELL_BIN, cmd_input)

            if mini_rc in (134, 138, 139):
                print(f"  [CRASH] {name}: exit={mini_rc}, err={mini_err.strip()}")
                failed += 1
                continue

            if bash_out == mini_out and bash_rc == mini_rc:
                print(f"  [PASS] {name}")
                passed += 1
            else:
                print(f"  [FAIL] {name}")
                print(f"    Input   : {repr(cmd_input)}")
                print(f"    Expected: rc={bash_rc}, out={repr(bash_out)}")
                print(f"    Actual  : rc={mini_rc}, out={repr(mini_out)}, err={repr(mini_err)}")
                failed += 1

    print(f"\nSummary: {passed}/{total} passed, {failed} failed.")
    sys.exit(1 if failed > 0 else 0)


if __name__ == "__main__":
    main()

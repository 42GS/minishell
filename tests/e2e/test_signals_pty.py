#!/usr/bin/env python3
"""
Interactive PTY Signal & Terminal Control Tests for Minishell.
Verifies Ctrl+C (SIGINT), Ctrl+\\ (SIGQUIT), Ctrl+D (EOF),
ECHOCTL suppression (^C not echoed), and Here-doc interruption.
"""

import os
import pty
import select
import sys
import time

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
MINISHELL_BIN = os.path.join(ROOT_DIR, "minishell")


def read_until(fd, marker, timeout=3.0):
    buf = b""
    deadline = time.time() + timeout
    while time.time() < deadline:
        r, _, _ = select.select([fd], [], [], 0.05)
        if fd in r:
            try:
                chunk = os.read(fd, 4096)
            except OSError:
                break
            if not chunk:
                break
            buf += chunk
            if marker and marker in buf:
                return buf.decode("utf-8", errors="replace")
    return buf.decode("utf-8", errors="replace")


def drain_all(fd, timeout=2.0):
    buf = b""
    deadline = time.time() + timeout
    while time.time() < deadline:
        r, _, _ = select.select([fd], [], [], 0.05)
        if fd in r:
            try:
                chunk = os.read(fd, 4096)
            except OSError:
                break
            if not chunk:
                break
            buf += chunk
        elif buf:
            time.sleep(0.05)
    return buf.decode("utf-8", errors="replace")


def spawn_pty():
    pid, master = pty.fork()
    if pid == 0:
        os.chdir(ROOT_DIR)
        os.execv(MINISHELL_BIN, [MINISHELL_BIN])
    init = read_until(master, b"minishell$ ", timeout=3.0)
    assert "minishell$ " in init, f"Missing initial prompt: {init!r}"
    return pid, master


def run_pty_tests():
    if not os.path.exists(MINISHELL_BIN):
        print(f"[ERROR] Missing binary: {MINISHELL_BIN}")
        sys.exit(1)

    pid, master = spawn_pty()
    try:
        # 1. Empty prompt Ctrl+C -> new prompt, $? = 130, no ^C
        os.write(master, b"\x03")
        out = read_until(master, b"minishell$ ", timeout=2.0)
        assert "^C" not in out, f"Unexpected ^C echo in: {out!r}"
        assert "minishell$ " in out, f"No new prompt after Ctrl+C: {out!r}"

        os.write(master, b"echo sigint_code=$?\n")
        out = read_until(master, b"minishell$ ", timeout=2.0)
        assert "sigint_code=130" in out, f"Expected 130 after Ctrl+C, got: {out!r}"

        # 2. Typed buffer + Ctrl+C -> buffer discarded, next cmd clean
        os.write(master, b"nonexistent_garbage_cmd")
        read_until(master, b"nonexistent_garbage_cmd", timeout=2.0)
        os.write(master, b"\x03")
        read_until(master, b"minishell$ ", timeout=2.0)
        os.write(master, b"echo buf_clean_ok\n")
        out = read_until(master, b"minishell$ ", timeout=2.0)
        assert "buf_clean_ok" in out, f"Expected buf_clean_ok, got: {out!r}"
        assert "command not found" not in out, f"Garbage buffer ran: {out!r}"

        # 3. Interactive prompt Ctrl+\ (SIGQUIT) -> ignored
        os.write(master, b"\x1c")
        time.sleep(0.1)
        os.write(master, b"echo quit_ignored\n")
        out = read_until(master, b"minishell$ ", timeout=2.0)
        assert "quit_ignored" in out, f"Ctrl+\\ disturbed prompt: {out!r}"

        # 4. Blocking child 'cat' + Ctrl+C -> killed with 130
        os.write(master, b"cat\n")
        read_until(master, b"cat\r\n", timeout=2.0)
        time.sleep(0.1)
        os.write(master, b"\x03")
        out = read_until(master, b"minishell$ ", timeout=2.0)
        assert "minishell$ " in out, f"No prompt after cat Ctrl+C: {out!r}"
        os.write(master, b"echo cat_int=$?\n")
        out2 = read_until(master, b"minishell$ ", timeout=2.0)
        assert "cat_int=130" in out2, f"Expected cat_int=130, got: {out2!r}"

        # 5. Blocking child 'cat' + Ctrl+\ -> Quit (core dumped) & 131
        os.write(master, b"cat\n")
        read_until(master, b"cat\r\n", timeout=2.0)
        time.sleep(0.1)
        os.write(master, b"\x1c")
        out = read_until(master, b"minishell$ ", timeout=2.0)
        assert "Quit (core dumped)" in out, f"Expected Quit msg, got: {out!r}"
        os.write(master, b"echo cat_quit=$?\n")
        out2 = read_until(master, b"minishell$ ", timeout=2.0)
        assert "cat_quit=131" in out2, f"Expected cat_quit=131, got: {out2!r}"

        # 6. Interactive Here-doc + Ctrl+C -> aborts heredoc, $? = 130
        os.write(master, b"cat << EOF\n")
        read_until(master, b"> ", timeout=2.0)
        os.write(master, b"should_not_print\n")
        read_until(master, b"> ", timeout=2.0)
        os.write(master, b"\x03")
        out = read_until(master, b"minishell$ ", timeout=2.0)
        assert "minishell$ " in out, f"No prompt after heredoc Ctrl+C: {out!r}"
        os.write(master, b"echo hd_int=$?\n")
        out = read_until(master, b"minishell$ ", timeout=2.0)
        assert "hd_int=130" in out, f"Expected hd_int=130, got: {out!r}"

        # 7. Blocking child in || list + Ctrl+C -> aborts right-hand side, $? = 130
        os.write(master, b"cat || echo RAN_AFTER_SIGINT\n")
        read_until(master, b"RAN_AFTER_SIGINT\r\n", timeout=2.0)
        time.sleep(0.1)
        os.write(master, b"\x03")
        out = read_until(master, b"minishell$ ", timeout=2.0)
        assert "RAN_AFTER_SIGINT" not in out, f"OR rhs ran after Ctrl+C: {out!r}"
        os.write(master, b"echo or_int=$?\n")
        out2 = read_until(master, b"minishell$ ", timeout=2.0)
        assert "or_int=130" in out2, f"Expected or_int=130, got: {out2!r}"

        # 8. Empty prompt Ctrl+D -> prints exit and exits 0
        os.write(master, b"\x04")
        tail = drain_all(master, timeout=2.0)
        _, status = os.waitpid(pid, 0)
        assert "exit" in tail, f"Expected exit on Ctrl+D, got: {tail!r}"
        assert os.WIFEXITED(status) and os.WEXITSTATUS(status) == 0, f"Status: {status}"
        pid = None
    finally:
        if pid is not None:
            try:
                os.kill(pid, 9)
                os.waitpid(pid, 0)
            except OSError:
                pass
        os.close(master)

    print("[PTY-SIGNALS] All 8 interactive PTY signal tests PASSED!")


if __name__ == "__main__":
    run_pty_tests()

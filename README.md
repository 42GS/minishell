# Minishell — As Beautiful as a Shell

> **42 School Common Core System Programming Project**  
> **Knowledge Base & System Architecture Wiki**: [42GS/minishell-wiki](https://github.com/42GS/minishell-wiki)  
> **Reference Standard**: GNU Bash (`bash --posix` / POSIX.1-2017)

---

## 1. Architecture & Implemented Features

`minishell`은 POSIX 호환 유닉스 셸을 C 언어로 구현한 프로젝트로, 어휘 분석(Lexer), 구문 분석(Recursive Descent AST Parser), 확장기(Expander), IPC 실행 엔진(Executor), 비동기 시그널 상태 머신, 7대 내장 유틸리티가 모듈형 아키텍처로 엄격히 분리되어 있습니다.

```text
Input Stream ──▶ Lexer (DFA) ──▶ Recursive Descent Parser ──▶ AST Binary Tree
                                                                    │
Signals & termios ◀── Builtins & Env ◀── Expander & Glob ◀── Executor (fork/pipe/dup2)
```

### Mandatory & Bonus Highlights
- **Parsing & Quoting**: `'...'`, `"..."` 상태 기계 기반 토큰화, 연산자 우선순위(`Redirection` > `|` > `&&`, `||`) 및 중첩 괄호 `( )` 서브셸 AST 조립, 구문 오류(`2`) 릭 프리 해제
- **Expansions & Wildcards**: `$VAR`, `$?`, `$0`, `$1`..`$9`, `$"..."`/`$'...'` 호환 확장, 비인용 IFS 단어 분할, `ambiguous redirect` 검출, 인용 마스크 기반 CWD 와일드카드 `*` Globbing (`opendir`/`readdir`/`closedir`, ASCII 정렬, 숨김 파일 규칙)
- **Redirections, Here-doc & Pipelines**: `<`, `>`, `>>`, `<<`(익명 `O_CREAT | O_EXCL` + 즉시 `unlink` 및 형제 FD 차단), 다중 파이프라인 엄격한 Pipe-Close 프로토콜, POSIX 빈 `PATH` 세그먼트 및 `126`/`127` 종료 코드
- **Builtins & Signals**: `echo -n`, `cd`, `pwd`, `export`, `unset`, `env`, 64비트 오버플로 방어 `exit` 구현, 단일 전역 변수(`volatile sig_atomic_t g_signal_num`)와 `termios` `ECHOCTL` 제어, `&&`/`||` 리스트 `SIGINT` 중단 처리

---

## 2. Directory Structure

| Path | Description |
| :--- | :--- |
| `include/` | 6대 모듈 인터페이스 헤더 (`minishell.h`, `lexer.h`, `parser.h`, `executor.h`, `builtins.h`, `signals.h`) |
| `src/` | 모듈별 C 프로덕션 소스 (`lexer/`, `parser/`, `executor/`, `expander/`, `env/`, `builtins/`, `signals/`) |
| `libft/` | 42 표준 문자열·메모리 및 capacity-doubling `t_strbuf` 유틸리티 정적 라이브러리 |
| `tests/` | 단위 테스트(`unit/`), Bash 차등 E2E 스위트(`e2e/suites/01~05.json`), PTY 시그널 및 Valgrind QA(`e2e/`) |
| `.github/` | Norminette, 엄격 빌드(`-Wall -Wextra -Werror`), No-Relink, E2E/PTY/Valgrind CI 게이트웨이 |

---

## 3. Build & Verification

```bash
# Mandatory & Bonus Build (-Wall -Wextra -Werror, No-Relink)
make all && make bonus

# Automated Verification Suites
bash tests/unit/run_unit.sh
python3 tests/e2e/run_e2e.py --ci
python3 tests/e2e/run_e2e.py --all
python3 tests/e2e/test_signals_pty.py
bash tests/e2e/run_valgrind_qa.sh   # Requires valgrind (Linux/CI)

# Clean Build Artifacts
make fclean
```

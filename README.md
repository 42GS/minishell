# Minishell — As Beautiful as a Shell

> **42 School Common Core System Programming Project**  
> **Knowledge Base & System Architecture Wiki**: [42GS/minishell-wiki](https://github.com/42GS/minishell-wiki)  
> **Reference Standard**: GNU Bash (`bash --posix` / POSIX.1-2017)

---

## 1. Architecture Overview

`minishell`은 POSIX 호환 유닉스 셸을 C 언어로 구현한 프로젝트로, 어휘 분석(Lexer), 구문 분석(AST Parser), IPC 실행 엔진(Executor), 비동기 시그널 상태 머신, 7대 내장 유틸리티가 모듈형 아키텍처로 엄격히 분리되어 있습니다.

```text
Input Stream ──▶ Lexer (DFA) ──▶ Recursive Descent Parser ──▶ AST Binary Tree
                                                                    │
                 Signals & termios ◀── Builtins & Env ◀── Executor (fork/pipe/dup2)
```

---

## 2. Directory Structure

| Path | Description |
| :--- | :--- |
| `include/` | 6대 모듈 인터페이스 헤더 (`minishell.h`, `lexer.h`, `parser.h`, `executor.h`, `builtins.h`, `signals.h`) |
| `src/` | 모듈별 C 프로덕션 소스 (`lexer/`, `parser/`, `executor/`, `env/`, `builtins/`, `signals/`) |
| `libft/` | 42 표준 문자열·메모리 유틸리티 정적 라이브러리 |
| `tests/` | 단위 테스트(`unit/`) 및 Bash 1:1 비교 E2E 통합 테스트(`e2e/`) 하네스 |
| `.github/` | Norminette, 엄격 빌드(`-Wall -Wextra -Werror`), No-Relink, Valgrind CI 게이트웨이 |

---

## 3. Build & Verification

```bash
# Mandatory Build
make

# Bonus Build
make bonus

# Run Automated Test Suite
bash tests/unit/run_unit.sh
python3 tests/e2e/run_e2e.py

# Clean Build Artifacts
make fclean
```

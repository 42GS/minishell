# Minishell Automated Test Suite

> **Architecture Reference**: [42GS/minishell-wiki — Evaluation Matrix](https://github.com/42GS/minishell-wiki/blob/main/wiki/requirements/evaluation_matrix.md)

## 1. Structure

- `unit/`: 모듈별 C 단위 테스트 (AAA 패턴 & FIRST 원칙 준수)
- `e2e/`: GNU Bash vs Minishell 1:1 출력/종료코드 비교 통합 테스트 (42evalhub 21개 항목 커버)
- `readline.supp`: GNU Readline 라이브러리 자체 `still reachable` 메모리 억제용 Valgrind 설정

## 2. Usage

```bash
# Run Unit Tests (C modules & libft)
bash tests/unit/run_unit.sh

# Run E2E Differential Tests (CI Smoke Mode)
python3 tests/e2e/run_e2e.py --ci

# Run Full 42evalhub E2E Suite
python3 tests/e2e/run_e2e.py --all

# Run Specific Suite
python3 tests/e2e/run_e2e.py --suite 04_builtins.json
```

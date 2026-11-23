/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:35:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:35:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <stdbool.h>
# include <signal.h>
# include <termios.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "libft.h"
# include "lexer.h"
# include "parser.h"
# include "executor.h"
# include "builtins.h"
# include "signals.h"

/**
 * @file minishell.h
 * @brief Top-level shell state, shared cleanup, and module orchestration.
 */

/** @brief Interactive prompt string passed to readline(). */
# define SHELL_PROMPT "minishell$ "

/**
 * @brief Async-signal-safe slot storing the latest caught signal number.
 *
 * The 42 subject permits a single global for signals; handlers write the
 * signal number here and main-loop code converts it to exit status later.
 */
extern volatile sig_atomic_t	g_signal_num;

/**
 * @struct s_shell
 * @brief  Root execution context threaded through parser, executor, and
 *         builtins to avoid extra globals.
 *
 * Key fields:
 * - env_list: Owned linked list of exported/local environment entries.
 * - last_exit_status: Status reported by $? and returned on normal exit.
 * - cwd: Heap-cached working directory kept in sync by cd.
 * - line, current_tokens, current_ast: Current command artifacts tracked so
 *   forked children can release inherited heap memory before exec/exit.
 * - is_interactive: True if STDIN_FILENO is a TTY; selects readline vs read.
 * - in_subshell: True in forked pipeline/subshell children to guard TTY/signal
 *   state owned by the parent shell.
 * - interrupted: Marks SIGINT child abort to halt && / || chains.
 * - has_backup_fds, saved_in, saved_out: Saved stdin/stdout around parent
 *   builtin redirections.
 * - orig_term: Initial termios snapshot restored when interactive shell exits.
 */
typedef struct s_shell
{
	t_env_node		*env_list;
	int				last_exit_status;
	char			*cwd;
	char			*line;
	bool			is_interactive;
	bool			in_subshell;
	bool			interrupted;
	bool			has_backup_fds;
	int				saved_in;
	int				saved_out;
	t_ast_node		*current_ast;
	t_token			*current_tokens;
	struct termios	orig_term;
}	t_shell;

/**
 * @brief  Free a NULL-terminated string array and every element inside it.
 *
 * @param  arr  Heap-allocated NULL-terminated array (safe no-op if NULL).
 */
void	free_str_array(char **arr);

/**
 * @brief  Release inherited heap buffers and backup FDs in a shell context.
 *
 * Used by forked children and exit paths to prevent Valgrind leaks without
 * altering parent readline history or terminal attributes.
 *
 * @param  ctx  Shell context to tear down (safe no-op if NULL).
 */
void	free_child_resources(t_shell *ctx);

/**
 * @brief  Read a single line from STDIN_FILENO without readline overhead.
 *
 * Reads byte-by-byte until newline or EOF so non-interactive pipelines and
 * scripts never emit prompts or consume ahead across child processes.
 *
 * @return Caller-owned line without trailing newline, or NULL on EOF/error.
 */
char	*read_stdin_line(void);

/**
 * @brief  Free child exec temporaries and context, then terminate via exit().
 *
 * @param  ctx   Shell context owned by the child process.
 * @param  path  Resolved executable path to free (may be NULL).
 * @param  envp  Serialized environment array to free (may be NULL).
 * @param  code  Process exit code passed to exit().
 */
void	exit_child_clean(t_shell *ctx, char *path, char **envp, int code);

#endif

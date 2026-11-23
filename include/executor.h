/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:48:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:48:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXECUTOR_H
# define EXECUTOR_H

# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <errno.h>
# include <string.h>
# include <dirent.h>
# include "parser.h"

/**
 * @file executor.h
 * @brief AST execution, pipelines, redirections, heredocs, and expansion.
 */

/** @brief Forward declaration breaking circular include with minishell.h. */
typedef struct s_shell	t_shell;

/**
 * @struct s_pipe_ctx
 * @brief  Transient execution state for a flattened multi-stage pipeline.
 *
 * Key fields:
 * - cmds: Flat array of count stage nodes (NODE_CMD or NODE_SUBSHELL).
 * - count: Total pipeline stages.
 * - pipes: Flat FD array of size 2 * (count - 1) storing [r0, w0, ...].
 * - pids: Child PIDs in stage order for wait_pipeline().
 */
typedef struct s_pipe_ctx
{
	t_ast_node	**cmds;
	int			count;
	int			*pipes;
	pid_t		*pids;
}	t_pipe_ctx;

/**
 * @struct s_exp_buf
 * @brief  Accumulator for quote-aware word splitting and wildcard masking.
 *
 * Key fields:
 * - fields: NULL-terminated output vector grown as fields flush.
 * - cur: Current field character buffer.
 * - mask: Parallel '0'/'1' buffer marking unquoted '*' eligible for globbing.
 * - has_wild: True if cur contains at least one masked-in wildcard.
 * - st: Active quote FSM state across the scanned word.
 */
typedef struct s_exp_buf
{
	char			**fields;
	t_strbuf		cur;
	t_strbuf		mask;
	bool			has_wild;
	t_quote_state	st;
}	t_exp_buf;

/**
 * @brief  Execute an AST subtree and update ctx->last_exit_status.
 *
 * Short-circuits && and || when ctx->interrupted is set by a child SIGINT.
 *
 * @param  node  Root or subtree AST node (NULL returns 0).
 * @param  ctx   Shell runtime context.
 * @return Exit status of the executed node (0..255).
 */
int		execute_ast(t_ast_node *node, t_shell *ctx);

/**
 * @brief  Fork and wire a pipeline of commands, returning last stage status.
 *
 * @param  node  NODE_PIPE root subtree.
 * @param  ctx   Shell runtime context.
 * @return Normalized exit status of final stage, or 1 on pipe/fork error.
 */
int		execute_pipeline(t_ast_node *node, t_shell *ctx);

/**
 * @brief  Apply a command's redirection list left-to-right on current FDs.
 *
 * Consumes and closes any pre-opened heredoc_fd entries on success.
 *
 * @param  redirs  Linked list of redirections to apply.
 * @param  ctx     Shell runtime context for target expansion.
 * @return 0 on success, 1 on expansion, open, or dup2 failure.
 */
int		apply_redirections(t_redir *redirs, t_shell *ctx);

/**
 * @brief  Pre-collect all heredocs in an AST before command execution.
 *
 * Traverses post-order (left, right, node) using unlinked temp files so
 * pipeline stages and &&/|| branches never contend for STDIN simultaneously.
 *
 * @param  node  AST subtree to scan for REDIR_HEREDOC entries.
 * @param  ctx   Shell runtime context.
 * @return 1 on success, 0 on temp-file error or heredoc SIGINT abort.
 */
int		collect_heredocs(t_ast_node *node, t_shell *ctx);

/**
 * @brief  Resolve an external command name to an executable path.
 *
 * Treats names containing '/' or invocations with empty/unset PATH as direct
 * paths; otherwise searches ':'-separated PATH entries.
 *
 * @param  cmd       Command name or path from argv[0].
 * @param  ctx       Shell context supplying env_list.
 * @param  err_code  Out exit code: 127 not found, 126 dir/EACCES, 1 OOM.
 * @return Caller-owned resolved path, or NULL after printing diagnostics.
 */
char	*resolve_cmd_path(const char *cmd, t_shell *ctx, int *err_code);

/**
 * @brief  Close all valid (>= 0) pipe FDs across pipe_count pipe pairs.
 *
 * @param  pipes       Flat array of pipe_count * 2 FDs (safe if NULL).
 * @param  pipe_count  Number of pipe pairs in pipes.
 */
void	close_all_pipes(int *pipes, int pipe_count);

/**
 * @brief  Fork all stages of pctx, wiring pipe FDs in each child process.
 *
 * @param  pctx  Initialized pipeline context with open pipes.
 * @param  ctx   Shell runtime context inherited by children.
 * @return Number of children successfully forked (< pctx->count on error).
 */
int		spawn_pipe_children(t_pipe_ctx *pctx, t_shell *ctx);

/**
 * @brief  Wait for count pipeline children and restore interactive signals.
 *
 * Reports child signal termination once and sets ctx->interrupted on SIGINT.
 *
 * @param  pids   Array of spawned child PIDs.
 * @param  count  Number of spawned children to reap.
 * @param  ctx    Shell runtime context.
 * @return Normalized status of the last stage (or 1 if waitpid fails).
 */
int		wait_pipeline(pid_t *pids, int count, t_shell *ctx);

/**
 * @brief  Wait for a single child PID, update interrupt state, and normalize.
 *
 * @param  pid  Child process ID to wait on.
 * @param  ctx  Shell runtime context.
 * @return Normalized exit status (0..255).
 */
int		wait_child_status(pid_t pid, t_shell *ctx);

/** @brief Convert raw waitpid status into shell exit code (128+sig on sig). */
int		normalize_wait_status(int status);

/**
 * @brief  Child-process entry applying redirs, expanding args, and execing.
 *
 * Closes inherited heredoc FDs and always terminates via exit_child_clean().
 *
 * @param  cmd  NODE_CMD AST node to run in the child.
 * @param  ctx  Child-owned shell context.
 */
void	exec_cmd_in_child(t_ast_node *cmd, t_shell *ctx);

/** @brief Write "minishell: <cmd>: <msg>\n" diagnostic to STDERR_FILENO. */
void	print_exec_error(const char *cmd, const char *msg);

/**
 * @brief  Child-process heredoc reader writing body lines to fds[1].
 *
 * @param  fds    Pipe/temp-file pair (closes read end fds[0] immediately).
 * @param  redir  Heredoc descriptor supplying raw delimiter and quote flag.
 * @param  ctx    Child-owned shell context freed prior to exit.
 */
void	run_heredoc_child(int fds[2], t_redir *redir, t_shell *ctx);

/**
 * @brief  Close open heredoc_fd handles in node except inside subtree skip.
 *
 * Prevents forked subshells and children from holding sibling heredoc FDs.
 *
 * @param  node  AST root to traverse.
 * @param  skip  Subtree whose heredoc FDs remain open for execution (or NULL).
 */
void	close_ast_heredocs(t_ast_node *node, t_ast_node *skip);

/**
 * @brief  Extract next ':'-delimited directory from PATH (empty -> ".").
 *
 * @param  p     PATH environment value string.
 * @param  pos   In/out scan offset advanced past ':'.
 * @param  done  Set to true when end of p is reached.
 * @return Caller-owned directory string, or NULL on allocation failure.
 */
char	*extract_path_dir(const char *p, size_t *pos, bool *done);

/**
 * @brief  Join directory and command with '/', taking ownership of dir.
 *
 * Frees dir unconditionally before returning.
 *
 * @param  dir  Heap-allocated directory prefix (consumed/freed).
 * @param  cmd  Command basename.
 * @return Caller-owned joined path, or NULL on error.
 */
char	*join_path_cmd(char *dir, const char *cmd);

/**
 * @brief  Test candidate executable path; consume cand on miss or denial.
 *
 * @param  cand    Heap candidate path (kept if 1, or saved in *denied).
 * @param  denied  Receives first non-directory EACCES path for diagnostics.
 * @return 1 if cand is executable (frees *denied), 0 otherwise.
 */
int		check_candidate(char *cand, char **denied);

/**
 * @brief  Expand $?, $0, $<digit>, $VAR, or literal $ starting at s[*i].
 *
 * @param  s    Source string containing '$' at *i.
 * @param  i    In/out cursor advanced past the consumed '$' sequence.
 * @param  ctx  Shell context supplying last_exit_status and env_list.
 * @return Caller-owned expansion string, or NULL on allocation failure.
 */
char	*expand_dollar(const char *s, size_t *i, t_shell *ctx);

/**
 * @brief  Expand $ variables and strip quotes without splitting or globbing.
 *
 * Used for export NAME=VAL arguments where field splitting must not split
 * the assigned value.
 *
 * @param  raw  Raw word string.
 * @param  ctx  Shell context.
 * @return Caller-owned expanded string, or NULL on allocation failure.
 */
char	*expand_word(const char *raw, t_shell *ctx);

/**
 * @brief  Expand $ variables in an unquoted heredoc line keeping quotes.
 *
 * @param  raw  Single input line from a heredoc body.
 * @param  ctx  Shell context.
 * @return Caller-owned expanded line, or NULL on allocation failure.
 */
char	*expand_heredoc_line(const char *raw, t_shell *ctx);

/**
 * @brief  Remove quote delimiters without expanding $ (heredoc delimiter).
 *
 * @param  raw  Raw delimiter word.
 * @return Caller-owned unquoted string, or NULL on allocation failure.
 */
char	*strip_quotes_only(const char *raw);

/**
 * @brief  Expand a raw word with quote removal, field splitting, and glob.
 *
 * @param  raw  Raw token string.
 * @param  ctx  Shell context.
 * @param  err  Out error flag set to 1 on allocation failure, 0 otherwise.
 * @return Caller-owned NULL-terminated field array (NULL if empty or error).
 */
char	**expand_word_fields(const char *raw, t_shell *ctx, int *err);

/**
 * @brief  Expand a redirection target and reject 0 or >1 resulting fields.
 *
 * @param  raw  Raw redirection target word.
 * @param  ctx  Shell context.
 * @return Caller-owned single path, or NULL on OOM or ambiguous redirect.
 */
char	*expand_redir_target(const char *raw, t_shell *ctx);

/**
 * @brief  Expand cmd->argv in place once, guarded by cmd->is_expanded.
 *
 * @param  cmd  NODE_CMD AST node whose argv is expanded.
 * @param  ctx  Shell context.
 * @return 1 on success, 0 on allocation failure.
 */
int		expand_cmd_args(t_ast_node *cmd, t_shell *ctx);

/**
 * @brief  Expand unmasked '*' wildcards against sorted cwd entries into dst.
 *
 * Leaves patterns with '/' or zero matches as a literal entry; hides dotfiles
 * unless pat explicitly starts with '.'.
 *
 * @param  pat   Expanded pattern string.
 * @param  mask  Parallel '0'/'1' mask identifying unquoted '*' wildcards.
 * @param  dst   Address of NULL-terminated destination argv array.
 * @return 1 on success, 0 on allocation failure.
 */
int		expand_glob_field(const char *pat, const char *mask, char ***dst);

/** @brief True when unquoted '$' at s[i] precedes a quote (elided). */
bool	is_dollar_quote(const char *s, size_t i, t_quote_state st);

/**
 * @brief  Append heap chunk to sb and free chunk (frees sb if chunk is NULL).
 *
 * @param  sb     Target string buffer.
 * @param  chunk  Owned chunk string to append and free (NULL aborts/frees sb).
 * @return 1 on success, 0 on NULL chunk or buffer allocation failure.
 */
int		sb_push_str_free(t_strbuf *sb, char *chunk);

#endif

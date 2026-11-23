/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_redir.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:48:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:48:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file executor_redir.c
 * @brief File/here-doc redirection application and child command execution.
 */

/** @brief Open a redirection file path with POSIX flags and mode 0644. */
static int	open_redir_file(t_redir *redir, const char *path)
{
	if (redir->type == REDIR_IN)
		return (open(path, O_RDONLY));
	if (redir->type == REDIR_OUT)
		return (open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
	return (open(path, O_WRONLY | O_CREAT | O_APPEND, 0644));
}

/**
 * @brief  Attach a pre-collected here-doc FD to STDIN and close the original.
 * @param  redir  REDIR_HEREDOC entry holding heredoc_fd.
 * @return 0 on success, 1 if heredoc_fd is invalid or dup2() fails.
 *
 * Resets heredoc_fd to -1 after closing so repeated cleanup passes never
 * double-close an already released descriptor.
 */
static int	apply_heredoc_fd(t_redir *redir)
{
	if (redir->heredoc_fd < 0)
		return (1);
	if (dup2(redir->heredoc_fd, STDIN_FILENO) == -1)
		return (1);
	close(redir->heredoc_fd);
	redir->heredoc_fd = -1;
	return (0);
}

/**
 * @brief  Expand and apply a single file or here-doc redirection entry.
 * @param  redir  Redirection descriptor to apply.
 * @param  ctx    Shell context used for target expansion.
 * @return 0 on success, 1 on expansion, open, or dup2 failure.
 */
static int	apply_single_redir(t_redir *redir, t_shell *ctx)
{
	char	*path;
	int		fd;
	int		target_fd;
	int		ret;

	if (redir->type == REDIR_HEREDOC)
		return (apply_heredoc_fd(redir));
	path = expand_redir_target(redir->target, ctx);
	if (!path)
		return (1);
	fd = open_redir_file(redir, path);
	if (fd < 0)
	{
		print_exec_error(path, strerror(errno));
		free(path);
		return (1);
	}
	target_fd = STDOUT_FILENO;
	if (redir->type == REDIR_IN)
		target_fd = STDIN_FILENO;
	ret = (dup2(fd, target_fd) == -1);
	close(fd);
	free(path);
	return (ret);
}

/**
 * @brief  Apply a linked list of redirections in left-to-right order.
 * @param  redirs  Head of the redirection list (may be NULL).
 * @param  ctx     Shell context passed to target expansion.
 * @return 0 if all redirections succeeded, 1 at the first failure.
 *
 * Sequential order ensures later redirections override earlier ones on the
 * same FD and aborts immediately when any open fails, matching Bash.
 */
int	apply_redirections(t_redir *redirs, t_shell *ctx)
{
	while (redirs)
	{
		if (apply_single_redir(redirs, ctx) != 0)
			return (1);
		redirs = redirs->next;
	}
	return (0);
}

/**
 * @brief  Execute a simple command inside an already-forked child process.
 * @param  cmd  Simple command AST node.
 * @param  ctx  Shell context cleaned via exit_child_clean() on every exit.
 *
 * Applies redirections, then calls close_ast_heredocs() so no unused here-doc
 * FDs remain open across execve(). Dispatches builtins or resolves PATH and
 * calls execve(), exiting 126 if execve() itself fails. Never returns.
 */
void	exec_cmd_in_child(t_ast_node *cmd, t_shell *ctx)
{
	char	*path;
	char	**envp;
	int		err_code;

	if (apply_redirections(cmd->redirs, ctx) != 0)
		exit_child_clean(ctx, NULL, NULL, 1);
	close_ast_heredocs(ctx->current_ast, NULL);
	if (!expand_cmd_args(cmd, ctx))
		exit_child_clean(ctx, NULL, NULL, 1);
	if (!cmd->argv || !cmd->argv[0])
		exit_child_clean(ctx, NULL, NULL, 0);
	if (is_builtin(cmd->argv[0]))
		exit_child_clean(ctx, NULL, NULL,
			dispatch_builtin(cmd->argv, ctx, true));
	err_code = 0;
	path = resolve_cmd_path(cmd->argv[0], ctx, &err_code);
	if (!path)
		exit_child_clean(ctx, NULL, NULL, err_code);
	envp = env_list_to_array(ctx->env_list);
	if (!envp)
		exit_child_clean(ctx, path, NULL, 1);
	execve(path, cmd->argv, envp);
	print_exec_error(cmd->argv[0], strerror(errno));
	exit_child_clean(ctx, path, envp, 126);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_heredoc.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:48:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 13:34:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file executor_heredoc.c
 * @brief Pre-execution here-document collection using unlinked temp files.
 */

/** @brief Build `/tmp/.minishell_hd_<id>` path; caller owns the string. */
static char	*make_heredoc_tmp_path(int id)
{
	char	*num;
	char	*path;

	num = ft_itoa(id);
	if (!num)
		return (NULL);
	path = ft_strjoin("/tmp/.minishell_hd_", num);
	free(num);
	return (path);
}

/**
 * @brief  Open an anonymous temp-file FD pair (read/write) for a here-doc.
 * @param  fds  Output array receiving read FD at fds[0] and write FD at fds[1].
 * @return 1 on success, 0 on open/allocation/collision-limit failure.
 *
 * Uses O_CREAT | O_EXCL then immediately unlink()s the file so the kernel
 * reclaims disk storage on close even after a crash. Backing with a regular
 * file avoids the 64 KB OS pipe-buffer deadlock on large here-documents.
 */
static int	open_anon_heredoc_fds(int fds[2])
{
	char	*path;
	int		id;

	id = 0;
	while (id < 100000)
	{
		path = make_heredoc_tmp_path(id++);
		if (!path)
			return (0);
		fds[1] = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
		if (fds[1] >= 0)
		{
			fds[0] = open(path, O_RDONLY);
			unlink(path);
			free(path);
			if (fds[0] < 0)
				close(fds[1]);
			return (fds[0] >= 0);
		}
		free(path);
		if (errno != EEXIST)
			return (0);
	}
	return (0);
}

/**
 * @brief  Reap the here-doc reader child and restore interactive signals.
 * @param  pid  PID of the forked here-doc reader child.
 * @param  ctx  Shell context checked for subshell state.
 * @return 1 if child exited 0, 0 on error or SIGINT abort.
 *
 * Retries waitpid() on EINTR so caught signals do not leave a zombie; on
 * child SIGINT termination, prints a newline and records g_signal_num.
 */
static int	wait_heredoc_child(pid_t pid, t_shell *ctx)
{
	int	status;

	while (waitpid(pid, &status, 0) == -1)
	{
		if (errno != EINTR)
		{
			if (!ctx->in_subshell)
				set_signal_state(SIG_STATE_INTERACTIVE);
			return (0);
		}
	}
	if (!ctx->in_subshell)
		set_signal_state(SIG_STATE_INTERACTIVE);
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
	{
		write(STDOUT_FILENO, "\n", 1);
		g_signal_num = SIGINT;
		return (0);
	}
	return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/**
 * @brief  Fork a child to populate one REDIR_HEREDOC descriptor.
 * @param  redir  Redirection node whose heredoc_fd is populated on success.
 * @param  ctx    Shell context for signals and child cleanup.
 * @return 1 on success with redir->heredoc_fd open, 0 on failure/abort.
 *
 * Closes the parent's write FD immediately after fork and discards fds[0] if
 * the child fails or is interrupted so no FDs leak.
 */
static int	process_heredoc_redir(t_redir *redir, t_shell *ctx)
{
	int		fds[2];
	pid_t	pid;
	int		ok;

	if (!open_anon_heredoc_fds(fds))
		return (0);
	if (!ctx->in_subshell)
		set_signal_state(SIG_STATE_EXECUTION);
	pid = fork();
	if (pid == 0)
		run_heredoc_child(fds, redir, ctx);
	close(fds[1]);
	ok = 0;
	if (pid > 0)
		ok = wait_heredoc_child(pid, ctx);
	else if (!ctx->in_subshell)
		set_signal_state(SIG_STATE_INTERACTIVE);
	if (ok)
	{
		redir->heredoc_fd = fds[0];
		return (1);
	}
	close(fds[0]);
	return (0);
}

/**
 * @brief  Collect all here-documents in the AST before execution begins.
 * @param  node  AST root to traverse in left-to-right order.
 * @param  ctx   Shell context passed to each here-doc collector.
 * @return 1 if all here-docs succeeded, 0 on first error or SIGINT abort.
 *
 * Pre-collecting in AST order matches Bash prompt sequencing and ensures all
 * heredoc_fds are ready before pipelines or subshells fork.
 */
int	collect_heredocs(t_ast_node *node, t_shell *ctx)
{
	t_redir	*redir;

	if (!node)
		return (1);
	if (node->left && !collect_heredocs(node->left, ctx))
		return (0);
	if (node->right && !collect_heredocs(node->right, ctx))
		return (0);
	redir = node->redirs;
	while (redir)
	{
		if (redir->type == REDIR_HEREDOC)
		{
			if (!process_heredoc_redir(redir, ctx))
				return (0);
		}
		redir = redir->next;
	}
	return (1);
}

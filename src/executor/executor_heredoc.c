/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_heredoc.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:48:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 12:15:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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

static int	open_heredoc_rw(t_redir *redir, t_shell *ctx, char *path)
{
	int		fd;
	pid_t	pid;
	int		ok;

	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
		return (0);
	if (!ctx->in_subshell)
		set_signal_state(SIG_STATE_EXECUTION);
	pid = fork();
	if (pid == 0)
		run_heredoc_child(fd, redir, path, ctx);
	close(fd);
	ok = 0;
	if (pid > 0)
		ok = wait_heredoc_child(pid, ctx);
	else if (!ctx->in_subshell)
		set_signal_state(SIG_STATE_INTERACTIVE);
	return (ok);
}

static int	process_heredoc_redir(t_redir *redir, t_shell *ctx, int *id)
{
	char	*path;
	int		ok;

	path = make_heredoc_tmp_path((*id)++);
	if (!path)
		return (0);
	ok = open_heredoc_rw(redir, ctx, path);
	if (ok)
		redir->heredoc_fd = open(path, O_RDONLY);
	unlink(path);
	free(path);
	return (ok && redir->heredoc_fd >= 0);
}

int	collect_heredocs(t_ast_node *node, t_shell *ctx)
{
	static int	hd_id;
	t_redir		*redir;

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
			if (!process_heredoc_redir(redir, ctx, &hd_id))
				return (0);
		}
		redir = redir->next;
	}
	return (1);
}

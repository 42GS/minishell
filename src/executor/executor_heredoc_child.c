/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_heredoc_child.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 12:15:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 12:15:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	print_heredoc_eof_warn(const char *delim)
{
	ft_putstr_fd("minishell: warning: here-document delimited by ",
		STDERR_FILENO);
	ft_putstr_fd("end-of-file (wanted `", STDERR_FILENO);
	ft_putstr_fd((char *)delim, STDERR_FILENO);
	ft_putendl_fd("')", STDERR_FILENO);
}

static int	write_heredoc_line(int fd, char *line, bool quoted, t_shell *ctx)
{
	char	*out;

	if (quoted)
		out = line;
	else
	{
		out = expand_heredoc_line(line, ctx);
		if (!out)
			return (0);
	}
	ft_putendl_fd(out, fd);
	if (!quoted)
		free(out);
	return (1);
}

static int	read_heredoc_loop(int fd, const char *delim, bool quoted,
	t_shell *ctx)
{
	char	*line;
	int		ok;

	while (1)
	{
		if (ctx->is_interactive)
			line = readline("> ");
		else
			line = read_stdin_line();
		if (!line || ft_strcmp(line, delim) == 0)
		{
			if (!line)
				print_heredoc_eof_warn(delim);
			free(line);
			break ;
		}
		ok = write_heredoc_line(fd, line, quoted, ctx);
		free(line);
		if (!ok)
			return (0);
	}
	return (1);
}

void	run_heredoc_child(int fds[2], t_redir *redir, t_shell *ctx)
{
	char	*delim;
	int		ok;

	close(fds[0]);
	set_signal_state(SIG_STATE_HEREDOC);
	delim = strip_quotes_only(redir->target);
	ok = 0;
	if (delim)
		ok = read_heredoc_loop(fds[1], delim, redir->quoted_delim, ctx);
	close(fds[1]);
	free(delim);
	free_child_resources(ctx);
	if (ok)
		exit(0);
	exit(1);
}

void	close_ast_heredocs(t_ast_node *node)
{
	t_redir	*redir;

	if (!node)
		return ;
	close_ast_heredocs(node->left);
	close_ast_heredocs(node->right);
	redir = node->redirs;
	while (redir)
	{
		if (redir->heredoc_fd >= 0)
		{
			close(redir->heredoc_fd);
			redir->heredoc_fd = -1;
		}
		redir = redir->next;
	}
}

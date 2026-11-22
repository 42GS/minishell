/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_heredoc.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:48:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:48:00 by yoshin           ###   ########.fr       */
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
		if (!line || g_signal_num == SIGINT || ft_strcmp(line, delim) == 0)
		{
			free(line);
			break ;
		}
		ok = write_heredoc_line(fd, line, quoted, ctx);
		free(line);
		if (!ok)
			return (0);
	}
	return (g_signal_num != SIGINT);
}

static int	process_heredoc_redir(t_redir *redir, t_shell *ctx, int *id)
{
	char	*delim;
	char	*path;
	int		fd;
	int		ok;

	delim = strip_quotes_only(redir->target);
	path = make_heredoc_tmp_path((*id)++);
	ok = 0;
	if (delim && path)
	{
		fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
		if (fd >= 0)
		{
			ok = read_heredoc_loop(fd, delim, redir->quoted_delim, ctx);
			close(fd);
		}
		if (ok)
			redir->heredoc_fd = open(path, O_RDONLY);
		unlink(path);
	}
	free(delim);
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

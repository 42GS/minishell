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

static int	open_redir_file(t_redir *redir, const char *path)
{
	if (redir->type == REDIR_IN)
		return (open(path, O_RDONLY));
	if (redir->type == REDIR_OUT)
		return (open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
	return (open(path, O_WRONLY | O_CREAT | O_APPEND, 0644));
}

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

static int	apply_single_redir(t_redir *redir, t_shell *ctx)
{
	char	*path;
	int		fd;
	int		target_fd;
	int		ret;

	if (redir->type == REDIR_HEREDOC)
		return (apply_heredoc_fd(redir));
	path = expand_word(redir->target, ctx);
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

void	exec_cmd_in_child(t_ast_node *cmd, t_shell *ctx)
{
	char	*path;
	char	**envp;
	int		err_code;

	if (apply_redirections(cmd->redirs, ctx) != 0)
		exit_child_clean(ctx, NULL, NULL, 1);
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

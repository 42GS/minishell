/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_pipe_child.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:35:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:35:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	close_all_pipes(int *pipes, int pipe_count)
{
	int	i;

	if (!pipes)
		return ;
	i = 0;
	while (i < pipe_count * 2)
	{
		if (pipes[i] >= 0)
			close(pipes[i]);
		i++;
	}
}

static void	run_pipe_node(t_pipe_ctx *pctx, int idx, t_shell *ctx)
{
	t_ast_node	*cmd;
	int			status;

	if (idx > 0)
		dup2(pctx->pipes[(idx - 1) * 2], STDIN_FILENO);
	if (idx < pctx->count - 1)
		dup2(pctx->pipes[idx * 2 + 1], STDOUT_FILENO);
	close_all_pipes(pctx->pipes, pctx->count - 1);
	cmd = pctx->cmds[idx];
	free(pctx->cmds);
	free(pctx->pids);
	free(pctx->pipes);
	ctx->in_subshell = true;
	if (cmd->type == NODE_CMD)
		exec_cmd_in_child(cmd, ctx);
	if (apply_redirections(cmd->redirs, ctx) != 0)
	{
		free_child_resources(ctx);
		exit(1);
	}
	status = execute_ast(cmd->left, ctx);
	free_child_resources(ctx);
	exit(status);
}

int	spawn_pipe_children(t_pipe_ctx *pctx, t_shell *ctx)
{
	int	i;

	i = 0;
	while (i < pctx->count)
	{
		pctx->pids[i] = fork();
		if (pctx->pids[i] == -1)
		{
			perror("minishell: fork");
			return (i);
		}
		if (pctx->pids[i] == 0)
			run_pipe_node(pctx, i, ctx);
		i++;
	}
	return (i);
}

int	normalize_wait_status(int status)
{
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	if (WIFSIGNALED(status))
		return (128 + WTERMSIG(status));
	return (1);
}

int	wait_pipeline(pid_t *pids, int count)
{
	int	i;
	int	status;
	int	last_status;

	last_status = 0;
	i = 0;
	while (i < count)
	{
		if (waitpid(pids[i], &status, 0) == -1)
			return (1);
		if (i == count - 1)
			last_status = normalize_wait_status(status);
		i++;
	}
	return (last_status);
}

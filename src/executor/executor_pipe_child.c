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

static int	setup_pipe_fds(t_pipe_ctx *pctx, int idx)
{
	if (idx > 0 && dup2(pctx->pipes[(idx - 1) * 2], STDIN_FILENO) == -1)
	{
		perror("minishell: dup2");
		return (1);
	}
	if (idx < pctx->count - 1
		&& dup2(pctx->pipes[idx * 2 + 1], STDOUT_FILENO) == -1)
	{
		perror("minishell: dup2");
		return (1);
	}
	return (0);
}

static void	run_pipe_node(t_pipe_ctx *pctx, int idx, t_shell *ctx)
{
	t_ast_node	*cmd;
	int			status;
	int			dup_err;

	dup_err = setup_pipe_fds(pctx, idx);
	close_all_pipes(pctx->pipes, pctx->count - 1);
	cmd = pctx->cmds[idx];
	free(pctx->cmds);
	free(pctx->pids);
	free(pctx->pipes);
	if (dup_err != 0)
		exit_child_clean(ctx, NULL, NULL, 1);
	ctx->in_subshell = true;
	set_signal_state(SIG_STATE_CHILD);
	if (cmd->type == NODE_CMD)
		exec_cmd_in_child(cmd, ctx);
	if (apply_redirections(cmd->redirs, ctx) != 0)
		exit_child_clean(ctx, NULL, NULL, 1);
	close_ast_heredocs(ctx->current_ast, cmd->left);
	status = execute_ast(cmd->left, ctx);
	exit_child_clean(ctx, NULL, NULL, status);
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

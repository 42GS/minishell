/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_pipe.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:35:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:35:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	count_pipe_cmds(t_ast_node *node)
{
	if (!node)
		return (0);
	if (node->type != NODE_PIPE)
		return (1);
	return (count_pipe_cmds(node->left) + count_pipe_cmds(node->right));
}

static void	fill_pipe_cmds(t_ast_node *node, t_ast_node **cmds, int *idx)
{
	if (!node)
		return ;
	if (node->type != NODE_PIPE)
	{
		cmds[*idx] = node;
		(*idx)++;
		return ;
	}
	fill_pipe_cmds(node->left, cmds, idx);
	fill_pipe_cmds(node->right, cmds, idx);
}

static int	init_pipe_ctx(t_pipe_ctx *pctx, t_ast_node *node)
{
	int	i;

	pctx->count = count_pipe_cmds(node);
	pctx->cmds = (t_ast_node **)ft_calloc(pctx->count, sizeof(t_ast_node *));
	pctx->pids = (pid_t *)ft_calloc(pctx->count, sizeof(pid_t));
	pctx->pipes = (int *)malloc(sizeof(int) * 2 * (pctx->count - 1));
	if (!pctx->cmds || !pctx->pids || !pctx->pipes)
		return (0);
	i = 0;
	fill_pipe_cmds(node, pctx->cmds, &i);
	i = 0;
	while (i < pctx->count - 1)
	{
		if (pipe(&pctx->pipes[i * 2]) == -1)
		{
			close_all_pipes(pctx->pipes, i);
			return (0);
		}
		i++;
	}
	return (1);
}

static void	cleanup_pipe_ctx(t_pipe_ctx *pctx)
{
	free(pctx->cmds);
	free(pctx->pids);
	free(pctx->pipes);
}

int	execute_pipeline(t_ast_node *node, t_shell *ctx)
{
	t_pipe_ctx	pctx;
	int			spawned;
	int			status;

	ft_bzero(&pctx, sizeof(t_pipe_ctx));
	if (!init_pipe_ctx(&pctx, node))
	{
		cleanup_pipe_ctx(&pctx);
		return (1);
	}
	if (!ctx->in_subshell)
		set_signal_state(SIG_STATE_EXECUTION);
	spawned = spawn_pipe_children(&pctx, ctx);
	close_all_pipes(pctx.pipes, pctx.count - 1);
	status = wait_pipeline(pctx.pids, spawned, ctx);
	cleanup_pipe_ctx(&pctx);
	if (spawned < pctx.count)
		return (1);
	return (status);
}

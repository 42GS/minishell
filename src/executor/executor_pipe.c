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

/**
 * @file executor_pipe.c
 * @brief Pipeline flattening, pipe allocation, and parent coordination.
 */

/** @brief Count leaf command/subshell stages in a NODE_PIPE subtree. */
static int	count_pipe_cmds(t_ast_node *node)
{
	if (!node)
		return (0);
	if (node->type != NODE_PIPE)
		return (1);
	return (count_pipe_cmds(node->left) + count_pipe_cmds(node->right));
}

/**
 * @brief  Flatten a binary NODE_PIPE tree into a left-to-right stage array.
 * @param  node  Current AST node in the pipeline tree.
 * @param  cmds  Preallocated array receiving leaf command/subshell nodes.
 * @param  idx   In/out write cursor into cmds.
 */
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

/**
 * @brief  Allocate stage arrays and open all `(count - 1)` OS pipes upfront.
 * @param  pctx  Pipeline context populated with cmds, pids, and pipe FDs.
 * @param  node  Root NODE_PIPE AST node.
 * @return 1 on success, 0 on OOM or pipe() failure (partial pipes closed).
 */
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

/** @brief Free heap arrays inside t_pipe_ctx after pipes are closed. */
static void	cleanup_pipe_ctx(t_pipe_ctx *pctx)
{
	free(pctx->cmds);
	free(pctx->pids);
	free(pctx->pipes);
}

/**
 * @brief  Execute a flattened pipeline and wait for all spawned stages.
 * @param  node  Root NODE_PIPE AST node.
 * @param  ctx   Shell execution context.
 * @return Last stage's normalized exit status, or 1 on pipe/fork failure.
 *
 * Follows the Strict Pipe-Close protocol: closes all pipe FDs in the parent
 * immediately after spawning so readers receive EOF when writers exit,
 * preventing pipeline deadlocks before wait_pipeline() reaps all children.
 */
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

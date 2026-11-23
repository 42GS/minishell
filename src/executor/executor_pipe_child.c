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

/**
 * @file executor_pipe_child.c
 * @brief Pipeline child spawning, dup2 wiring, and Strict Pipe-Close cleanup.
 */

/**
 * @brief  Close every open descriptor in a flattened pipe FD array.
 * @param  pipes       Array of `pipe_count * 2` raw pipe descriptors.
 * @param  pipe_count  Number of pipes (`stage_count - 1`).
 *
 * Enforces the Strict Pipe-Close protocol in both parent and child stages so
 * no dangling write ends block downstream readers from seeing EOF.
 */
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

/**
 * @brief  Redirect STDIN/STDOUT of pipeline stage idx to adjacent pipe ends.
 * @param  pctx  Pipeline context holding pipe FDs and stage count.
 * @param  idx   Zero-based index of the current pipeline stage.
 * @return 0 on success, 1 if dup2() fails.
 */
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

/**
 * @brief  Configure pipe FDs and execute a single pipeline stage in a child.
 * @param  pctx  Pipeline context (heap arrays freed before exec/eval).
 * @param  idx   Zero-based stage index in pctx->cmds.
 * @param  ctx   Shell context marked in_subshell = true for isolation.
 *
 * Completes Strict Pipe-Close immediately after dup2(), resets signals to
 * SIG_STATE_CHILD, and for subshell stages closes sibling AST heredocs via
 * close_ast_heredocs() before recursing into cmd->left. Never returns.
 */
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

/**
 * @brief  Fork child processes for each flattened pipeline stage.
 * @param  pctx  Pipeline context receiving child PIDs in pctx->pids.
 * @param  ctx   Shell context passed to each forked stage.
 * @return Number of children successfully spawned (may be < pctx->count).
 *
 * Stops spawning on the first fork() error while returning the partial count
 * so the caller still closes all pipes and reaps already-forked children.
 */
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

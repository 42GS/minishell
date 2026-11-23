/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:35:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:35:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file executor.c
 * @brief Top-level AST traversal, simple commands, and subshell execution.
 */

/**
 * @brief  Restore STDIN/STDOUT from backup FDs after a parent-run builtin.
 * @param  ctx  Shell context holding saved_in, saved_out, has_backup_fds.
 *
 * Closes saved descriptors and resets backup state so subsequent commands
 * never inherit stale redirections or leaked FDs.
 */
static void	restore_std_fds(t_shell *ctx)
{
	if (!ctx->has_backup_fds)
		return ;
	if (ctx->saved_in >= 0)
	{
		if (dup2(ctx->saved_in, STDIN_FILENO) == -1)
			perror("minishell: dup2");
		close(ctx->saved_in);
	}
	if (ctx->saved_out >= 0)
	{
		if (dup2(ctx->saved_out, STDOUT_FILENO) == -1)
			perror("minishell: dup2");
		close(ctx->saved_out);
	}
	ctx->saved_in = -1;
	ctx->saved_out = -1;
	ctx->has_backup_fds = false;
}

/**
 * @brief  Execute a builtin directly in the current shell process.
 * @param  cmd  Command AST node whose argv[0] is a known builtin.
 * @param  ctx  Shell context updated by builtin side effects.
 * @return Exit status of the builtin, or 1 if dup/redirection fails.
 *
 * Bypasses dup/dup2 backup when !cmd->redirs to avoid unnecessary FD churn;
 * when redirections exist, backups guarantee parent stdio is always restored.
 */
static int	execute_single_builtin(t_ast_node *cmd, t_shell *ctx)
{
	int	status;

	if (!cmd->redirs)
		return (dispatch_builtin(cmd->argv, ctx, ctx->in_subshell));
	ctx->saved_in = dup(STDIN_FILENO);
	ctx->saved_out = dup(STDOUT_FILENO);
	ctx->has_backup_fds = true;
	if (ctx->saved_in == -1 || ctx->saved_out == -1)
	{
		restore_std_fds(ctx);
		return (1);
	}
	if (apply_redirections(cmd->redirs, ctx) != 0)
	{
		restore_std_fds(ctx);
		return (1);
	}
	status = dispatch_builtin(cmd->argv, ctx, ctx->in_subshell);
	restore_std_fds(ctx);
	return (status);
}

/**
 * @brief  Expand and run a simple command node, in-process or via fork.
 * @param  cmd  Simple command AST node.
 * @param  ctx  Shell execution context.
 * @return Normalized command exit status, or 1 on expansion/fork failure.
 *
 * Expands arguments first so builtins run in-process without forking.
 * External commands set SIG_STATE_EXECUTION before fork() so the parent
 * ignores SIGINT/SIGQUIT until wait_child_status() finishes reaping.
 */
static int	execute_simple_cmd(t_ast_node *cmd, t_shell *ctx)
{
	pid_t	pid;

	if (!expand_cmd_args(cmd, ctx))
		return (1);
	if (cmd->argv && cmd->argv[0] && is_builtin(cmd->argv[0]))
		return (execute_single_builtin(cmd, ctx));
	if (!ctx->in_subshell)
		set_signal_state(SIG_STATE_EXECUTION);
	pid = fork();
	if (pid == -1)
	{
		if (!ctx->in_subshell)
			set_signal_state(SIG_STATE_INTERACTIVE);
		perror("minishell: fork");
		return (1);
	}
	if (pid == 0)
	{
		set_signal_state(SIG_STATE_CHILD);
		exec_cmd_in_child(cmd, ctx);
	}
	return (wait_child_status(pid, ctx));
}

/**
 * @brief  Fork and execute a parenthesized subshell AST node.
 * @param  node  NODE_SUBSHELL AST node containing redirs and left subtree.
 * @param  ctx   Shell context; child sets in_subshell to isolate state.
 * @return Exit status of the subshell process, or 1 if fork fails.
 *
 * Closes sibling AST heredoc FDs via close_ast_heredocs() while keeping
 * node->left intact, preventing FD leaks into nested commands.
 */
static int	execute_subshell(t_ast_node *node, t_shell *ctx)
{
	pid_t	pid;
	int		status;

	if (!ctx->in_subshell)
		set_signal_state(SIG_STATE_EXECUTION);
	pid = fork();
	if (pid == -1)
	{
		if (!ctx->in_subshell)
			set_signal_state(SIG_STATE_INTERACTIVE);
		perror("minishell: fork");
		return (1);
	}
	if (pid == 0)
	{
		set_signal_state(SIG_STATE_CHILD);
		ctx->in_subshell = true;
		if (apply_redirections(node->redirs, ctx) != 0)
			exit_child_clean(ctx, NULL, NULL, 1);
		close_ast_heredocs(ctx->current_ast, node->left);
		status = execute_ast(node->left, ctx);
		exit_child_clean(ctx, NULL, NULL, status);
	}
	return (wait_child_status(pid, ctx));
}

/**
 * @brief  Recursively evaluate an AST node and record last_exit_status.
 * @param  node  Root of the AST subtree to execute (may be NULL).
 * @param  ctx   Shell context holding exit status and interrupt flags.
 * @return Exit status of the evaluated subtree (0 when node is NULL).
 *
 * Short-circuits && / || chains on !ctx->interrupted so a SIGINT-killed
 * child aborts the list, whereas `(exit 130) || true` leaves interrupted
 * false and still evaluates the right-hand branch.
 */
int	execute_ast(t_ast_node *node, t_shell *ctx)
{
	int	status;

	if (!node)
		return (0);
	if (node->type == NODE_CMD)
		status = execute_simple_cmd(node, ctx);
	else if (node->type == NODE_PIPE)
		status = execute_pipeline(node, ctx);
	else if (node->type == NODE_SUBSHELL)
		status = execute_subshell(node, ctx);
	else if (node->type == NODE_AND)
	{
		status = execute_ast(node->left, ctx);
		if (status == 0 && !ctx->interrupted)
			status = execute_ast(node->right, ctx);
	}
	else
	{
		status = execute_ast(node->left, ctx);
		if (status != 0 && !ctx->interrupted)
			status = execute_ast(node->right, ctx);
	}
	ctx->last_exit_status = status;
	return (status);
}

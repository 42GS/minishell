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

static void	restore_std_fds(t_shell *ctx)
{
	if (!ctx->has_backup_fds)
		return ;
	if (ctx->saved_in >= 0)
	{
		dup2(ctx->saved_in, STDIN_FILENO);
		close(ctx->saved_in);
	}
	if (ctx->saved_out >= 0)
	{
		dup2(ctx->saved_out, STDOUT_FILENO);
		close(ctx->saved_out);
	}
	ctx->saved_in = -1;
	ctx->saved_out = -1;
	ctx->has_backup_fds = false;
}

static int	execute_single_builtin(t_ast_node *cmd, t_shell *ctx)
{
	int	status;

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

static int	execute_simple_cmd(t_ast_node *cmd, t_shell *ctx)
{
	pid_t	pid;
	int		status;

	if (!expand_cmd_args(cmd, ctx))
		return (1);
	if (cmd->argv && cmd->argv[0] && is_builtin(cmd->argv[0]))
		return (execute_single_builtin(cmd, ctx));
	pid = fork();
	if (pid == -1)
	{
		perror("minishell: fork");
		return (1);
	}
	if (pid == 0)
		exec_cmd_in_child(cmd, ctx);
	if (waitpid(pid, &status, 0) == -1)
		return (1);
	return (normalize_wait_status(status));
}

static int	execute_subshell(t_ast_node *node, t_shell *ctx)
{
	pid_t	pid;
	int		status;

	pid = fork();
	if (pid == -1)
	{
		perror("minishell: fork");
		return (1);
	}
	if (pid == 0)
	{
		ctx->in_subshell = true;
		if (apply_redirections(node->redirs, ctx) != 0)
		{
			free_child_resources(ctx);
			exit(1);
		}
		status = execute_ast(node->left, ctx);
		free_child_resources(ctx);
		exit(status);
	}
	if (waitpid(pid, &status, 0) == -1)
		return (1);
	return (normalize_wait_status(status));
}

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
		if (status == 0)
			status = execute_ast(node->right, ctx);
	}
	else
	{
		status = execute_ast(node->left, ctx);
		if (status != 0)
			status = execute_ast(node->right, ctx);
	}
	ctx->last_exit_status = status;
	return (status);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_redir_stub.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:35:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:35:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	apply_redirections(t_redir *redirs, t_shell *ctx)
{
	(void)redirs;
	(void)ctx;
	return (0);
}

int	collect_heredocs(t_ast_node *node, t_shell *ctx)
{
	(void)node;
	(void)ctx;
	return (0);
}

static void	exit_child_clean(t_shell *ctx, char *path, char **envp, int code)
{
	free(path);
	free_str_array(envp);
	free_child_resources(ctx);
	exit(code);
}

void	exec_cmd_in_child(t_ast_node *cmd, t_shell *ctx)
{
	char	*path;
	char	**envp;
	int		err_code;

	if (apply_redirections(cmd->redirs, ctx) != 0)
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

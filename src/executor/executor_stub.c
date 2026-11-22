/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_stub.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	execute_ast(t_ast_node *node, t_shell *ctx)
{
	(void)node;
	(void)ctx;
	return (0);
}

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

char	*resolve_cmd_path(const char *cmd, t_shell *ctx, int *err_code)
{
	(void)cmd;
	(void)ctx;
	if (err_code)
		*err_code = 0;
	return (NULL);
}

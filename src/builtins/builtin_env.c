/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_env.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	print_env_entry(t_env_node *node)
{
	if (builtin_write_str(STDOUT_FILENO, node->key) != 0)
		return (1);
	if (builtin_write_str(STDOUT_FILENO, "=") != 0)
		return (1);
	return (builtin_write_endl(STDOUT_FILENO, node->value));
}

int	ft_env(char **argv, t_shell *ctx)
{
	t_env_node	*cur;

	if (argv[1])
	{
		ft_putstr_fd("env: ", STDERR_FILENO);
		ft_putstr_fd(argv[1], STDERR_FILENO);
		ft_putendl_fd(": No such file or directory", STDERR_FILENO);
		return (127);
	}
	cur = ctx->env_list;
	while (cur)
	{
		if (cur->value && print_env_entry(cur) != 0)
			return (1);
		cur = cur->next;
	}
	return (0);
}

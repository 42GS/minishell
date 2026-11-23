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

/**
 * @file builtin_env.c
 * @brief Implementation of the env builtin for printing exported values.
 */

/**
 * @brief  Prints a single KEY=VALUE entry to stdout with write error checks.
 * @param  node  Environment node containing non-NULL key and value.
 * @return 0 on success, 1 if any underlying write() fails.
 */
static int	print_env_entry(t_env_node *node)
{
	if (builtin_write_str(STDOUT_FILENO, node->key) != 0)
		return (1);
	if (builtin_write_str(STDOUT_FILENO, "=") != 0)
		return (1);
	return (builtin_write_endl(STDOUT_FILENO, node->value));
}

/**
 * @brief  Executes the env builtin command.
 *
 * Rejects positional arguments with status 127 (no option/command support)
 * and prints only variables that have an assigned value, skipping
 * value-less export-only entries while propagating write() failures.
 * @param  argv  Argument vector starting with "env".
 * @param  ctx   Shell state providing the env_list linked list.
 * @return 0 on success, 1 on write() error, 127 if an argument is given.
 */
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

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
		if (cur->value)
		{
			ft_putstr_fd(cur->key, STDOUT_FILENO);
			ft_putstr_fd("=", STDOUT_FILENO);
			ft_putendl_fd(cur->value, STDOUT_FILENO);
		}
		cur = cur->next;
	}
	return (0);
}

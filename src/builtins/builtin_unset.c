/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_unset.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file builtin_unset.c
 * @brief Implementation of the unset builtin command.
 */

/**
 * @brief  Executes the unset builtin command.
 *
 * Removes each named variable from ctx->env_list if present, ignoring
 * missing names and always succeeding with status 0.
 * @param  argv  Argument vector of variable names starting at argv[1].
 * @param  ctx   Shell state whose env_list is modified in place.
 * @return Always returns 0.
 */
int	ft_unset(char **argv, t_shell *ctx)
{
	int	i;

	i = 1;
	while (argv[i])
	{
		env_unset(&ctx->env_list, argv[i]);
		i++;
	}
	return (0);
}

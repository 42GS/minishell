/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins_stub.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

bool	is_builtin(const char *cmd_name)
{
	(void)cmd_name;
	return (false);
}

int	dispatch_builtin(char **argv, t_shell *ctx, bool in_child)
{
	(void)argv;
	(void)ctx;
	(void)in_child;
	return (0);
}

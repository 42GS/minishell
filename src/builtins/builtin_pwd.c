/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_pwd.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	ft_pwd(t_shell *ctx)
{
	char		*cwd;
	const char	*fallback;
	int			ret;

	cwd = getcwd(NULL, 0);
	if (cwd)
	{
		ret = builtin_write_endl(STDOUT_FILENO, cwd);
		free(cwd);
		return (ret);
	}
	fallback = ctx->cwd;
	if (!fallback)
		fallback = env_get(ctx->env_list, "PWD");
	if (fallback)
		return (builtin_write_endl(STDOUT_FILENO, fallback));
	perror("minishell: pwd");
	return (1);
}

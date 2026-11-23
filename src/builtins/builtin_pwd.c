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

/**
 * @file builtin_pwd.c
 * @brief Implementation of the pwd builtin with cached cwd/PWD fallback.
 */

/**
 * @brief  Executes the pwd builtin command.
 *
 * Queries getcwd() first for the physical current directory and propagates
 * any write() error via builtin_write_endl(). If getcwd() fails (e.g.,
 * removed working directory), falls back to ctx->cwd then the PWD env var
 * before reporting an error via perror().
 * @param  ctx  Shell state providing the cwd cache and env_list fallback.
 * @return 0 on success, 1 if no path is available or write() fails.
 */
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

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_cd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file builtin_cd.c
 * @brief Implementation of the cd builtin and PWD/OLDPWD synchronization.
 */

/**
 * @brief  Prints a formatted cd error message to stderr.
 * @param  msg  Diagnostic text (e.g., strerror or reason string).
 * @param  arg  Optional target path to include before msg, or NULL.
 * @return Always returns 1 as the builtin failure status.
 */
static int	print_cd_err(const char *msg, const char *arg)
{
	ft_putstr_fd("minishell: cd: ", STDERR_FILENO);
	if (arg)
	{
		ft_putstr_fd((char *)arg, STDERR_FILENO);
		ft_putstr_fd(": ", STDERR_FILENO);
	}
	ft_putendl_fd((char *)msg, STDERR_FILENO);
	return (1);
}

/**
 * @brief  Resolves the destination path for cd from arguments and env.
 *
 * Handles no-argument fallback to HOME, rejects extra arguments (> 1), and
 * expands "-" to OLDPWD while setting print_dir so the resolved path is
 * echoed on success.
 * @param  argv       Argument vector passed to cd.
 * @param  ctx        Shell state providing env_list lookups.
 * @param  print_dir  Output flag set true when target came from "-".
 * @return Target path string (borrowed), or NULL on missing env/extra args.
 */
static const char	*resolve_cd_target(char **argv, t_shell *ctx,
	bool *print_dir)
{
	const char	*target;

	*print_dir = false;
	if (!argv[1])
	{
		target = env_get(ctx->env_list, "HOME");
		if (!target)
			print_cd_err("HOME not set", NULL);
		return (target);
	}
	if (argv[2])
	{
		print_cd_err("too many arguments", NULL);
		return (NULL);
	}
	if (ft_strcmp(argv[1], "-") == 0)
	{
		target = env_get(ctx->env_list, "OLDPWD");
		if (!target)
			print_cd_err("OLDPWD not set", NULL);
		*print_dir = true;
		return (target);
	}
	return (argv[1]);
}

/**
 * @brief  Synchronizes OLDPWD, PWD, and ctx->cwd after a successful chdir().
 *
 * Preserves the previous PWD (falling back to ctx->cwd if PWD was unset) as
 * OLDPWD, then queries getcwd() to store the canonical path. If getcwd()
 * fails (e.g., unlinked parent), reports a warning and still prints target
 * when print_dir is set.
 * @param  ctx        Shell state whose env_list and cwd cache are updated.
 * @param  target     Original target path passed to chdir() as fallback.
 * @param  print_dir  True when cd "-" requires printing the new directory.
 */
static void	update_pwd_vars(t_shell *ctx, const char *target, bool print_dir)
{
	const char	*old_pwd;
	char		*new_pwd;

	old_pwd = env_get(ctx->env_list, "PWD");
	if (!old_pwd)
		old_pwd = ctx->cwd;
	if (old_pwd)
		env_set(&ctx->env_list, "OLDPWD", old_pwd);
	new_pwd = getcwd(NULL, 0);
	if (new_pwd)
	{
		env_set(&ctx->env_list, "PWD", new_pwd);
		free(ctx->cwd);
		ctx->cwd = new_pwd;
		if (print_dir)
			ft_putendl_fd(new_pwd, STDOUT_FILENO);
		return ;
	}
	perror("minishell: cd: error retrieving current directory: getcwd");
	if (print_dir)
		ft_putendl_fd((char *)target, STDOUT_FILENO);
}

/**
 * @brief  Executes the cd builtin command.
 *
 * Treats an empty target string ("") as a successful no-op like Bash, then
 * invokes chdir() and updates PWD/OLDPWD and ctx->cwd on success.
 * @param  argv  Argument vector (argv[1] optional target or "-").
 * @param  ctx   Shell state providing environment and cwd cache.
 * @return 0 on success, 1 on argument, lookup, or chdir() failure.
 */
int	ft_cd(char **argv, t_shell *ctx)
{
	const char	*target;
	bool		print_dir;

	target = resolve_cd_target(argv, ctx, &print_dir);
	if (!target)
		return (1);
	if (target[0] == '\0')
		return (0);
	if (chdir(target) != 0)
		return (print_cd_err(strerror(errno), target));
	update_pwd_vars(ctx, target, print_dir);
	return (0);
}

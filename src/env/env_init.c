/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static bool	parse_shlvl_val(const char *s, int *out)
{
	long	val;
	int		sign;

	if (!s || !*s)
		return (false);
	while (*s == ' ' || (*s >= '\t' && *s <= '\r'))
		s++;
	sign = 1;
	if (*s == '-' || *s == '+')
	{
		if (*s == '-')
			sign = -1;
		s++;
	}
	if (!ft_isdigit((unsigned char)*s))
		return (false);
	val = 0;
	while (ft_isdigit((unsigned char)*s))
	{
		if (val < 100000L)
			val = val * 10 + (*s - '0');
		s++;
	}
	*out = (int)(val * sign);
	return (*s == '\0');
}

static void	update_shlvl(t_shell *ctx)
{
	const char	*raw;
	int			lvl;
	char		*next_str;

	raw = env_get(ctx->env_list, "SHLVL");
	if (!parse_shlvl_val(raw, &lvl))
		lvl = 1;
	else if (lvl < 0)
		lvl = 0;
	else
		lvl++;
	if (lvl >= 1000)
	{
		ft_putstr_fd("minishell: warning: shell level too high, ",
			STDERR_FILENO);
		ft_putendl_fd("resetting to 1", STDERR_FILENO);
		lvl = 1;
	}
	next_str = ft_itoa(lvl);
	if (!next_str)
		return ;
	env_set(&ctx->env_list, "SHLVL", next_str);
	free(next_str);
}

void	init_env_defaults(t_shell *ctx)
{
	char		*cwd;
	t_env_node	*cur;
	bool		has_oldpwd;

	if (!env_get(ctx->env_list, "PWD"))
	{
		cwd = getcwd(NULL, 0);
		if (cwd)
		{
			env_set(&ctx->env_list, "PWD", cwd);
			free(cwd);
		}
	}
	has_oldpwd = false;
	cur = ctx->env_list;
	while (cur)
	{
		if (ft_strcmp(cur->key, "OLDPWD") == 0)
			has_oldpwd = true;
		cur = cur->next;
	}
	if (!has_oldpwd)
		env_set(&ctx->env_list, "OLDPWD", NULL);
	update_shlvl(ctx);
}

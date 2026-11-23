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

/**
 * @file env_init.c
 * @brief Startup environment defaults for PWD, OLDPWD, and SHLVL.
 */

/** @brief Advance @p s past leading or trailing lexer whitespace. */
static const char	*skip_shlvl_spaces(const char *s)
{
	while (is_lexer_space(*s))
		s++;
	return (s);
}

/** @brief Consume optional '+'/'-' sign and store polarity in @p *sign. */
static const char	*parse_shlvl_sign(const char *s, int *sign)
{
	*sign = 1;
	if (*s == '-' || *s == '+')
	{
		if (*s == '-')
			*sign = -1;
		s++;
	}
	return (s);
}

/**
 * @brief  Strictly parse @p s as a signed integer for SHLVL.
 *
 * Matches Bash behavior: allows surrounding spaces and an optional sign,
 * rejects empty strings or trailing non-digits, and caps accumulated
 * magnitude at 100000 to prevent signed overflow before the >=1000 reset.
 *
 * @param  s    Raw SHLVL string (may be NULL or empty).
 * @param  out  Receives the parsed integer value on success.
 * @return true if @p s is a valid integer string, false otherwise.
 */
static bool	parse_shlvl_val(const char *s, int *out)
{
	long	val;
	int		sign;

	if (!s || !*s)
		return (false);
	s = parse_shlvl_sign(skip_shlvl_spaces(s), &sign);
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
	return (*skip_shlvl_spaces(s) == '\0');
}

/**
 * @brief  Increment SHLVL on startup with Bash-compatible bounds checks.
 *
 * Unset or non-numeric SHLVL starts at 1, negative values reset to 0, and
 * incremented levels >= 1000 warn on stderr and reset to 1.
 *
 * @param  ctx  Shell context owning env_list.
 */
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

/**
 * @brief  Initialize missing PWD, OLDPWD placeholder, and SHLVL at startup.
 *
 * Seeds PWD via getcwd() if absent, inserts OLDPWD with value == NULL when
 * missing (so it appears in `export` but not `env`/child envp until `cd`
 * sets it, distinguishing NULL from ""), and increments SHLVL.
 *
 * @param  ctx  Shell context whose env_list has been cloned from envp.
 */
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

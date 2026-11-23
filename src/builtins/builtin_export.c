/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_export.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file builtin_export.c
 * @brief Implementation of the export builtin and identifier validation.
 */

/** @brief Prints exported env vars sorted by key (builtin_export_print.c). */
int	print_sorted_export(t_env_node *env);

/**
 * @brief  Validates a shell identifier prefix of length len.
 *
 * Requires a leading ASCII letter or '_' followed solely by alphanumeric
 * characters or '_', rejecting empty keys or invalid characters before '='.
 * @param  s    Start of the candidate identifier string.
 * @param  len  Byte length of the key portion before '=' or '+='.
 * @return true if valid POSIX shell identifier, false otherwise.
 */
static bool	is_valid_export_key(const char *s, size_t len)
{
	size_t	i;

	if (!s || len == 0)
		return (false);
	if (!ft_isalpha((unsigned char)s[0]) && s[0] != '_')
		return (false);
	i = 1;
	while (i < len)
	{
		if (!ft_isalnum((unsigned char)s[i]) && s[i] != '_')
			return (false);
		i++;
	}
	return (true);
}

/**
 * @brief  Prints an invalid export identifier error to stderr.
 * @param  arg  Offending export argument printed inside quotes.
 * @return Always returns 1 to mark the argument as failed.
 */
static int	print_export_invalid(const char *arg)
{
	ft_putstr_fd("minishell: export: `", STDERR_FILENO);
	ft_putstr_fd((char *)arg, STDERR_FILENO);
	ft_putendl_fd("': not a valid identifier", STDERR_FILENO);
	return (1);
}

/**
 * @brief  Appends add to an existing variable's value or creates it if unset.
 *
 * Implements the Bash "+=" append semantics for export KEY+=VALUE.
 * @param  ctx  Shell state containing env_list.
 * @param  key  Validated variable name.
 * @param  add  Suffix string to concatenate onto the current value.
 * @return 0 on success, negative/non-zero on allocation or env_set failure.
 */
static int	set_export_append(t_shell *ctx, const char *key, const char *add)
{
	const char	*old;
	char		*joined;
	int			ret;

	old = env_get(ctx->env_list, key);
	if (!old)
		return (env_set(&ctx->env_list, key, add));
	joined = ft_strjoin(old, add);
	if (!joined)
		return (-1);
	ret = env_set(&ctx->env_list, key, joined);
	free(joined);
	return (ret);
}

/**
 * @brief  Parses and applies a single export argument (KEY, KEY=V, KEY+=V).
 *
 * Detects "+=" before '=' to distinguish append from assignment, validates
 * the key slice, and stores NULL value for bare KEY so env omits it while
 * sorted export still displays it.
 * @param  arg  Single argument string from argv.
 * @param  ctx  Shell state whose env_list is updated.
 * @return 0 on success, 1 on invalid identifier or allocation failure.
 */
static int	handle_export_arg(const char *arg, t_shell *ctx)
{
	char	*eq;
	size_t	klen;
	bool	is_app;
	char	*key;
	int		ret;

	eq = ft_strchr(arg, '=');
	is_app = (eq && eq > arg && *(eq - 1) == '+');
	if (!eq)
		klen = ft_strlen(arg);
	else
		klen = (size_t)(eq - arg) - (size_t)is_app;
	if (!is_valid_export_key(arg, klen))
		return (print_export_invalid(arg));
	key = ft_substr(arg, 0, klen);
	if (!key)
		return (1);
	if (!eq)
		ret = env_set(&ctx->env_list, key, NULL);
	else if (is_app)
		ret = set_export_append(ctx, key, eq + 1);
	else
		ret = env_set(&ctx->env_list, key, eq + 1);
	free(key);
	return (ret != 0);
}

/**
 * @brief  Executes the export builtin command.
 *
 * With no arguments, delegates to print_sorted_export(). Otherwise iterates
 * through all arguments even if an invalid identifier occurs, returning 1
 * if any argument failed while still applying valid ones.
 * @param  argv  Argument vector starting with "export".
 * @param  ctx   Shell state providing env_list.
 * @return 0 if all arguments succeeded, 1 if any argument or print failed.
 */
int	ft_export(char **argv, t_shell *ctx)
{
	int	i;
	int	status;

	if (!argv[1])
		return (print_sorted_export(ctx->env_list));
	status = 0;
	i = 1;
	while (argv[i])
	{
		if (handle_export_arg(argv[i], ctx) != 0)
			status = 1;
		i++;
	}
	return (status);
}

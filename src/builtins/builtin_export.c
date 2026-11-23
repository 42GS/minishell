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

int	print_sorted_export(t_env_node *env);

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

static int	print_export_invalid(const char *arg)
{
	ft_putstr_fd("minishell: export: `", STDERR_FILENO);
	ft_putstr_fd((char *)arg, STDERR_FILENO);
	ft_putendl_fd("': not a valid identifier", STDERR_FILENO);
	return (1);
}

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

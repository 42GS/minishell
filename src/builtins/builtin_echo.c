/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_echo.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static bool	is_echo_n_flag(const char *arg)
{
	size_t	i;

	if (!arg || arg[0] != '-' || arg[1] != 'n')
		return (false);
	i = 2;
	while (arg[i])
	{
		if (arg[i] != 'n')
			return (false);
		i++;
	}
	return (true);
}

static int	write_str_safe(const char *s)
{
	size_t	len;

	len = ft_strlen(s);
	if (len == 0)
		return (0);
	if (write(STDOUT_FILENO, s, len) < 0)
		return (1);
	return (0);
}

int	ft_echo(char **argv)
{
	int		i;
	bool	no_nl;

	no_nl = false;
	i = 1;
	while (argv[i] && is_echo_n_flag(argv[i]))
	{
		no_nl = true;
		i++;
	}
	while (argv[i])
	{
		if (write_str_safe(argv[i]) != 0)
			return (1);
		if (argv[i + 1] && write(STDOUT_FILENO, " ", 1) < 0)
			return (1);
		i++;
	}
	if (!no_nl && write(STDOUT_FILENO, "\n", 1) < 0)
		return (1);
	return (0);
}

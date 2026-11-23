/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_exit.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <limits.h>
#include "minishell.h"

static bool	parse_exit_digits(const char *s, int sign, int *code)
{
	unsigned long long	acc;
	unsigned long long	lim;
	unsigned long long	digit;

	lim = (unsigned long long)LLONG_MAX + (unsigned long long)(sign == -1);
	acc = 0;
	while (ft_isdigit((unsigned char)*s))
	{
		digit = (unsigned long long)(*s - '0');
		if (acc > (lim - digit) / 10ULL)
			return (false);
		acc = acc * 10ULL + digit;
		s++;
	}
	while (is_lexer_space(*s))
		s++;
	if (*s != '\0')
		return (false);
	*code = (int)(unsigned char)((long long)(acc % 256ULL) * sign);
	return (true);
}

bool	parse_exit_code(const char *str, int *code)
{
	int	sign;

	if (!str || !code)
		return (false);
	while (is_lexer_space(*str))
		str++;
	sign = 1;
	if (*str == '+' || *str == '-')
	{
		if (*str == '-')
			sign = -1;
		str++;
	}
	if (!ft_isdigit((unsigned char)*str))
		return (false);
	return (parse_exit_digits(str, sign, code));
}

static void	exit_clean_shell(t_shell *ctx, bool in_child, int code)
{
	bool	interactive_parent;

	interactive_parent = (!in_child && !ctx->in_subshell
			&& ctx->is_interactive);
	free_child_resources(ctx);
	if (interactive_parent)
	{
		rl_clear_history();
		restore_terminal(&ctx->orig_term);
	}
	exit(code);
}

static void	print_exit_num_err(const char *arg)
{
	ft_putstr_fd("minishell: exit: ", STDERR_FILENO);
	ft_putstr_fd((char *)arg, STDERR_FILENO);
	ft_putendl_fd(": numeric argument required", STDERR_FILENO);
}

int	ft_exit(char **argv, t_shell *ctx, bool in_child)
{
	int	code;

	if (!in_child && !ctx->in_subshell && ctx->is_interactive)
		ft_putendl_fd("exit", STDERR_FILENO);
	if (!argv[1])
		exit_clean_shell(ctx, in_child,
			(int)(unsigned char)ctx->last_exit_status);
	if (!parse_exit_code(argv[1], &code))
	{
		print_exit_num_err(argv[1]);
		exit_clean_shell(ctx, in_child, 2);
	}
	if (argv[2])
	{
		ft_putendl_fd("minishell: exit: too many arguments", STDERR_FILENO);
		return (1);
	}
	exit_clean_shell(ctx, in_child, code);
	return (0);
}

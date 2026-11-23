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

/**
 * @file builtin_exit.c
 * @brief Implementation of the exit builtin and 64-bit exit code parsing.
 */

/**
 * @brief  Parses unsigned digits with signed 64-bit overflow detection.
 *
 * Bounds accumulation against LLONG_MAX for positive numbers and
 * LLONG_MAX + 1 (i.e. |LLONG_MIN|) for negative numbers, rejects trailing
 * non-whitespace characters, and folds the value modulo 256 into *code.
 * @param  s     Pointer to the first digit after optional sign.
 * @param  sign  1 for non-negative, -1 for negative.
 * @param  code  Output exit code truncated to 8-bit unsigned range.
 * @return true if valid 64-bit signed integer, false on overflow/garbage.
 */
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

/**
 * @brief  Parses an exit status string into an 8-bit exit code.
 *
 * Skips leading shell whitespace and an optional '+' or '-' sign before
 * delegating digit validation and LLONG_MAX/LLONG_MIN overflow checking.
 * @param  str   Argument string to parse.
 * @param  code  Output integer receiving the modulo-256 exit status.
 * @return true on valid numeric argument, false otherwise.
 */
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

/**
 * @brief  Frees shell resources, restores terminal if needed, and exits.
 *
 * Clears readline history and restores saved termios attributes only when
 * exiting from an interactive top-level shell (not child or subshell).
 * @param  ctx       Shell context to release before terminating.
 * @param  in_child  True if invoked inside a forked pipeline child.
 * @param  code      Final process exit code passed to exit().
 */
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

/** @brief Prints numeric argument required diagnostic for exit to stderr. */
static void	print_exit_num_err(const char *arg)
{
	ft_putstr_fd("minishell: exit: ", STDERR_FILENO);
	ft_putstr_fd((char *)arg, STDERR_FILENO);
	ft_putendl_fd(": numeric argument required", STDERR_FILENO);
}

/**
 * @brief  Executes the exit builtin command.
 *
 * Prints "exit" on stderr in interactive parent shells, exits with status 2
 * on non-numeric or overflowing arguments, and returns 1 without exiting
 * when given more than one argument after a valid numeric first argument.
 * @param  argv      Argument vector starting with "exit".
 * @param  ctx       Shell state providing last_exit_status and cleanup data.
 * @param  in_child  True if running in a forked child process.
 * @return 1 when too many arguments prevents exit; else does not return.
 */
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

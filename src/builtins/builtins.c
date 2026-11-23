/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file builtins.c
 * @brief Builtin command registry, dispatcher, and safe write helpers.
 */

/**
 * @brief  Checks whether a command name matches a shell builtin.
 *
 * Used by the executor to run state-mutating builtins in the parent process
 * instead of forking an external binary.
 * @param  cmd_name  Command name to test (NULL-safe).
 * @return true if cmd_name is one of the 7 builtins, false otherwise.
 */
bool	is_builtin(const char *cmd_name)
{
	if (!cmd_name)
		return (false);
	if (ft_strcmp(cmd_name, "echo") == 0)
		return (true);
	if (ft_strcmp(cmd_name, "cd") == 0)
		return (true);
	if (ft_strcmp(cmd_name, "pwd") == 0)
		return (true);
	if (ft_strcmp(cmd_name, "export") == 0)
		return (true);
	if (ft_strcmp(cmd_name, "unset") == 0)
		return (true);
	if (ft_strcmp(cmd_name, "env") == 0)
		return (true);
	if (ft_strcmp(cmd_name, "exit") == 0)
		return (true);
	return (false);
}

/**
 * @brief  Dispatches a builtin invocation to its handler function.
 * @param  argv      Argument vector where argv[0] is the builtin name.
 * @param  ctx       Mutable shell state (env list, cwd, exit status).
 * @param  in_child  True if running inside a forked pipeline child.
 * @return Exit status of the executed builtin, 0 if empty, 1 if unknown.
 */
int	dispatch_builtin(char **argv, t_shell *ctx, bool in_child)
{
	if (!argv || !argv[0])
		return (0);
	if (ft_strcmp(argv[0], "echo") == 0)
		return (ft_echo(argv));
	if (ft_strcmp(argv[0], "cd") == 0)
		return (ft_cd(argv, ctx));
	if (ft_strcmp(argv[0], "pwd") == 0)
		return (ft_pwd(ctx));
	if (ft_strcmp(argv[0], "export") == 0)
		return (ft_export(argv, ctx));
	if (ft_strcmp(argv[0], "unset") == 0)
		return (ft_unset(argv, ctx));
	if (ft_strcmp(argv[0], "env") == 0)
		return (ft_env(argv, ctx));
	if (ft_strcmp(argv[0], "exit") == 0)
		return (ft_exit(argv, ctx, in_child));
	return (1);
}

/**
 * @brief  Writes a full string to fd, handling partial write() calls.
 *
 * Loops until all bytes are written or write() fails (e.g., EPIPE/ENOSPC),
 * allowing builtins like pwd, env, and export to propagate write errors.
 * @param  fd  Destination file descriptor.
 * @param  s   String to write (NULL is treated as a no-op).
 * @return 0 on complete write, 1 on write() error.
 */
int	builtin_write_str(int fd, const char *s)
{
	size_t	len;
	ssize_t	written;

	if (!s)
		return (0);
	len = ft_strlen(s);
	while (len > 0)
	{
		written = write(fd, s, len);
		if (written < 0)
			return (1);
		s += written;
		len -= (size_t)written;
	}
	return (0);
}

/**
 * @brief  Writes a string followed by a newline with write() error checks.
 * @param  fd  Destination file descriptor.
 * @param  s   String to write before the trailing newline.
 * @return 0 on success, 1 if either write fails.
 */
int	builtin_write_endl(int fd, const char *s)
{
	if (builtin_write_str(fd, s) != 0)
		return (1);
	if (write(fd, "\n", 1) < 0)
		return (1);
	return (0);
}

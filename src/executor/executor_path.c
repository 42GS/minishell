/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_path.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:35:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 15:52:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file executor_path.c
 * @brief Command path resolution and POSIX 126/127 error precedence.
 */

/** @brief Print `minishell: <cmd>: <msg>` diagnostic line to STDERR. */
void	print_exec_error(const char *cmd, const char *msg)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd((char *)cmd, STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putendl_fd((char *)msg, STDERR_FILENO);
}

/**
 * @brief  Validate a slash-containing or PATH-less direct command path.
 * @param  cmd       Command path string to inspect.
 * @param  err_code  Receives 127 (ENOENT) or 126 (directory or EACCES).
 * @return Duplicated executable path on success, or NULL on error.
 *
 * Enforces POSIX exit-code precedence: missing file -> 127, directory or
 * non-executable file -> 126.
 */
static char	*check_direct_path(const char *cmd, int *err_code)
{
	struct stat	st;

	if (access(cmd, F_OK) != 0)
	{
		print_exec_error(cmd, strerror(errno));
		*err_code = 127;
		return (NULL);
	}
	if (stat(cmd, &st) == 0 && S_ISDIR(st.st_mode))
	{
		print_exec_error(cmd, "Is a directory");
		*err_code = 126;
		return (NULL);
	}
	if (access(cmd, X_OK) != 0)
	{
		print_exec_error(cmd, strerror(errno));
		*err_code = 126;
		return (NULL);
	}
	return (ft_strdup(cmd));
}

/**
 * @brief  Report final failure when no executable matched in PATH.
 * @param  denied    First non-executable regular file seen, or NULL.
 * @param  cmd       Original command name for "command not found".
 * @param  err_code  Receives 126 if denied is non-NULL, else 127.
 * @return Always NULL; takes ownership of and frees denied.
 *
 * Preserves POSIX 126 precedence over 127 when a matching file existed in
 * PATH without execute permission and no later entry succeeded.
 */
static char	*finish_path_miss(char *denied, const char *cmd, int *err_code)
{
	if (denied)
	{
		print_exec_error(denied, "Permission denied");
		free(denied);
		*err_code = 126;
		return (NULL);
	}
	print_exec_error(cmd, "command not found");
	*err_code = 127;
	return (NULL);
}

/**
 * @brief  Search colon-delimited PATH segments for an executable command.
 * @param  path_env  Non-empty PATH environment value.
 * @param  cmd       Bare command name (no slashes).
 * @param  err_code  Receives 1 on OOM, or 126/127 via finish_path_miss().
 * @return Heap-allocated executable path, or NULL if not found/denied.
 *
 * Continues scanning past permission-denied hits so a later runnable entry
 * wins, while remembering the first denied path for 126 reporting.
 */
static char	*search_in_paths(const char *path_env, const char *cmd,
	int *err_code)
{
	size_t	pos;
	bool	done;
	char	*cand;
	char	*denied;

	pos = 0;
	done = false;
	denied = NULL;
	while (!done)
	{
		cand = join_path_cmd(extract_path_dir(path_env, &pos, &done), cmd);
		if (!cand)
		{
			free(denied);
			*err_code = 1;
			return (NULL);
		}
		if (check_candidate(cand, &denied))
			return (cand);
	}
	return (finish_path_miss(denied, cmd, err_code));
}

/**
 * @brief  Resolve a command name to an executable path for execve().
 * @param  cmd       Command name or path from argv[0].
 * @param  ctx       Shell context supplying env_list for PATH lookup.
 * @param  err_code  Output status (0 on hit; 126, 127, or 1 on failure).
 * @return Heap-allocated path owned by caller, or NULL on failure.
 *
 * Rejects empty string, `.`, and `..` upfront with 127; treats unset or
 * empty PATH as a direct current-directory lookup per POSIX.
 */
char	*resolve_cmd_path(const char *cmd, t_shell *ctx, int *err_code)
{
	const char	*path_env;

	*err_code = 0;
	if (!cmd || !*cmd || ft_strcmp(cmd, ".") == 0
		|| ft_strcmp(cmd, "..") == 0)
	{
		if (!cmd || !*cmd)
			print_exec_error("", "command not found");
		else
			print_exec_error(cmd, "command not found");
		*err_code = 127;
		return (NULL);
	}
	if (ft_strchr(cmd, '/'))
		return (check_direct_path(cmd, err_code));
	path_env = env_get(ctx->env_list, "PATH");
	if (!path_env || !*path_env)
		return (check_direct_path(cmd, err_code));
	return (search_in_paths(path_env, cmd, err_code));
}

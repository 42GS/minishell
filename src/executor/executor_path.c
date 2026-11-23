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

void	print_exec_error(const char *cmd, const char *msg)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd((char *)cmd, STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putendl_fd((char *)msg, STDERR_FILENO);
}

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

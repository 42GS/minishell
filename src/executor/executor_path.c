/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_path.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:35:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:35:00 by yoshin           ###   ########.fr       */
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

static char	*join_path_cmd(const char *dir, const char *cmd)
{
	char	*slash;
	char	*full;

	slash = ft_strjoin(dir, "/");
	if (!slash)
		return (NULL);
	full = ft_strjoin(slash, cmd);
	free(slash);
	return (full);
}

static char	*search_in_paths(char **paths, const char *cmd, int *err_code)
{
	int		i;
	char	*candidate;

	i = 0;
	while (paths[i])
	{
		candidate = join_path_cmd(paths[i], cmd);
		if (!candidate)
		{
			*err_code = 1;
			return (NULL);
		}
		if (access(candidate, X_OK) == 0)
			return (candidate);
		free(candidate);
		i++;
	}
	print_exec_error(cmd, "command not found");
	*err_code = 127;
	return (NULL);
}

char	*resolve_cmd_path(const char *cmd, t_shell *ctx, int *err_code)
{
	const char	*path_env;
	char		**paths;
	char		*resolved;

	*err_code = 0;
	if (!cmd || !*cmd)
	{
		print_exec_error("", "command not found");
		*err_code = 127;
		return (NULL);
	}
	if (ft_strchr(cmd, '/'))
		return (check_direct_path(cmd, err_code));
	path_env = env_get(ctx->env_list, "PATH");
	if (!path_env || !*path_env)
		return (check_direct_path(cmd, err_code));
	paths = ft_split(path_env, ':');
	if (!paths)
	{
		*err_code = 1;
		return (NULL);
	}
	resolved = search_in_paths(paths, cmd, err_code);
	free_str_array(paths);
	return (resolved);
}

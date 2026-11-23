/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_path_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 15:52:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 15:52:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*extract_path_dir(const char *p, size_t *pos, bool *done)
{
	size_t	start;
	size_t	end;

	start = *pos;
	end = start;
	while (p[end] && p[end] != ':')
		end++;
	if (p[end] == ':')
		*pos = end + 1;
	else
		*done = true;
	if (end == start)
		return (ft_strdup("."));
	return (ft_substr(p, (unsigned int)start, end - start));
}

char	*join_path_cmd(char *dir, const char *cmd)
{
	char	*slash;
	char	*full;

	if (!dir)
		return (NULL);
	slash = ft_strjoin(dir, "/");
	free(dir);
	if (!slash)
		return (NULL);
	full = ft_strjoin(slash, cmd);
	free(slash);
	return (full);
}

int	check_candidate(char *cand, char **denied)
{
	struct stat	st;

	if (stat(cand, &st) == 0 && !S_ISDIR(st.st_mode))
	{
		if (access(cand, X_OK) == 0)
		{
			free(*denied);
			*denied = NULL;
			return (1);
		}
		if (!*denied)
		{
			*denied = cand;
			return (0);
		}
	}
	free(cand);
	return (0);
}

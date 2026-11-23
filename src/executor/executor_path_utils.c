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

/**
 * @file executor_path_utils.c
 * @brief POSIX PATH segment splitting and candidate validation helpers.
 */

/**
 * @brief  Extract the next colon-separated directory from PATH.
 * @param  p     PATH string being scanned.
 * @param  pos   In/out byte offset advanced past the delimiter.
 * @param  done  Set true when the NUL terminator is reached.
 * @return Heap-allocated directory string, or NULL on allocation failure.
 *
 * Per POSIX, an empty segment (leading, trailing, or `::`) denotes `.`
 * (the current working directory) rather than being skipped.
 */
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

/**
 * @brief  Build `<dir>/<cmd>` candidate path and unconditionally free dir.
 * @param  dir  Heap-allocated directory segment (may be NULL; always freed).
 * @param  cmd  Command name to append after `/`.
 * @return Newly allocated candidate path, or NULL on error.
 */
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

/**
 * @brief  Test whether a PATH candidate is a runnable non-directory file.
 * @param  cand    Candidate path (freed unless kept as return or *denied).
 * @param  denied  In/out pointer storing first non-X_OK file for 126 error.
 * @return 1 if cand is executable (clearing *denied), 0 otherwise.
 *
 * Transfers ownership of cand to *denied only for the first non-directory
 * permission miss so 126 takes precedence if no later PATH entry runs.
 */
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

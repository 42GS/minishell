/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_glob.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 20:58:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 20:58:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file expander_glob.c
 * @brief Quote-aware '*' wildcard pathname expansion in current directory.
 */

/**
 * @brief  Advance one step in iterative wildcard matching with backtrack.
 *
 * Uses @p idx as [pat_i, str_i, star_pat_i, star_str_i]. Only '*' chars
 * whose parallel @p mask byte is '1' act as wildcards, keeping quoted or
 * escaped asterisks literal.
 *
 * @param  pat   Expanded pattern string.
 * @param  mask  Parallel mask ('1' for unquoted '*', '0' for literal).
 * @param  str   Candidate filename being tested.
 * @param  idx   4-element cursor/backtrack state array.
 * @return 1 if matching progressed or backtracked, 0 on mismatch.
 */
static int	glob_advance(const char *pat, const char *mask, const char *str,
	size_t idx[4])
{
	if (pat[idx[0]] == '*' && mask[idx[0]] == '1')
	{
		idx[2] = idx[0];
		idx[3] = idx[1];
		idx[0]++;
		return (1);
	}
	if (pat[idx[0]] && pat[idx[0]] == str[idx[1]])
	{
		idx[0]++;
		idx[1]++;
		return (1);
	}
	if (idx[2] != (size_t)-1)
	{
		idx[0] = idx[2] + 1;
		idx[3]++;
		idx[1] = idx[3];
		return (1);
	}
	return (0);
}

/**
 * @brief  Match @p str against @p pat using the parallel wildcard @p mask.
 * @param  pat   Pattern string.
 * @param  mask  Parallel mask where '1' marks unquoted '*' wildcards.
 * @param  str   Candidate directory entry name.
 * @return true if @p str matches the entire pattern, false otherwise.
 */
static bool	glob_match(const char *pat, const char *mask, const char *str)
{
	size_t	idx[4];

	idx[0] = 0;
	idx[1] = 0;
	idx[2] = (size_t)-1;
	idx[3] = 0;
	while (str[idx[1]])
	{
		if (!glob_advance(pat, mask, str, idx))
			return (false);
	}
	while (pat[idx[0]] == '*' && mask[idx[0]] == '1')
		idx[0]++;
	return (pat[idx[0]] == '\0');
}

/**
 * @brief  Sort matched filenames in-place by ASCII order (POSIX C locale).
 * @param  arr  NULL-terminated array of matched filename strings.
 */
static void	sort_matches(char **arr)
{
	size_t	i;
	size_t	j;
	char	*tmp;

	if (!arr)
		return ;
	i = 0;
	while (arr[i])
	{
		j = i + 1;
		while (arr[j])
		{
			if (ft_strcmp(arr[i], arr[j]) > 0)
			{
				tmp = arr[i];
				arr[i] = arr[j];
				arr[j] = tmp;
			}
			j++;
		}
		i++;
	}
}

/**
 * @brief  Collect matching filenames from "." into @p *out.
 *
 * Skips "." and "..", suppresses hidden dotfiles unless @p pat explicitly
 * starts with '.', and leaves @p *out NULL when @p pat contains '/' (no
 * multi-segment path globbing) or opendir(".") fails.
 *
 * @param  pat   Pattern string.
 * @param  mask  Parallel wildcard mask aligned with @p pat.
 * @param  out   Receives NULL-terminated array of matches, or NULL.
 * @return 1 on success, 0 on allocation failure.
 */
static int	collect_dir_matches(const char *pat, const char *mask, char ***out)
{
	DIR				*dir;
	struct dirent	*ent;
	int				ok;

	*out = NULL;
	if (ft_strchr(pat, '/'))
		return (1);
	dir = opendir(".");
	if (!dir)
		return (1);
	ok = 1;
	ent = readdir(dir);
	while (ok && ent)
	{
		if (ft_strcmp(ent->d_name, ".") != 0
			&& ft_strcmp(ent->d_name, "..") != 0
			&& !(ent->d_name[0] == '.' && pat[0] != '.')
			&& glob_match(pat, mask, ent->d_name))
			ok = append_str_to_argv(out, ent->d_name);
		ent = readdir(dir);
	}
	closedir(dir);
	return (ok);
}

/**
 * @brief  Expand one field via '*' globbing and append results to @p *dst.
 *
 * Preserves the literal @p pat as a single field when no directory entries
 * match (standard POSIX non-nullglob behavior), or appends sorted matches.
 *
 * @param  pat   Field pattern string.
 * @param  mask  Parallel wildcard mask ('1' for unquoted '*').
 * @param  dst   Destination NULL-terminated field array.
 * @return 1 on success, 0 on allocation failure.
 */
int	expand_glob_field(const char *pat, const char *mask, char ***dst)
{
	char	**matches;
	size_t	i;
	int		ok;

	if (!collect_dir_matches(pat, mask, &matches))
	{
		free_str_array(matches);
		return (0);
	}
	if (!matches)
		return (append_str_to_argv(dst, pat));
	sort_matches(matches);
	i = 0;
	ok = 1;
	while (ok && matches[i])
	{
		ok = append_str_to_argv(dst, matches[i]);
		i++;
	}
	free_str_array(matches);
	return (ok);
}

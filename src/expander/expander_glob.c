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

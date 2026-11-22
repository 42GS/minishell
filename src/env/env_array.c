/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_array.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:35:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:35:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static size_t	count_exported_vars(t_env_node *env)
{
	size_t	count;

	count = 0;
	while (env)
	{
		if (env->value != NULL)
			count++;
		env = env->next;
	}
	return (count);
}

static char	*format_env_entry(const char *key, const char *value)
{
	char	*tmp;
	char	*entry;

	tmp = ft_strjoin(key, "=");
	if (!tmp)
		return (NULL);
	entry = ft_strjoin(tmp, value);
	free(tmp);
	return (entry);
}

char	**env_list_to_array(t_env_node *env)
{
	char	**arr;
	size_t	i;

	arr = (char **)malloc(sizeof(char *) * (count_exported_vars(env) + 1));
	if (!arr)
		return (NULL);
	i = 0;
	while (env)
	{
		if (env->value != NULL)
		{
			arr[i] = format_env_entry(env->key, env->value);
			if (!arr[i])
			{
				free_str_array(arr);
				return (NULL);
			}
			i++;
		}
		env = env->next;
	}
	arr[i] = NULL;
	return (arr);
}

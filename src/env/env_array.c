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

/**
 * @file env_array.c
 * @brief Serialization of environment list into execve-ready envp array.
 */

/**
 * @brief  Count environment nodes that have an assigned non-NULL value.
 *
 * Variables declared via bare `export VAR` have value == NULL; they stay
 * in the internal list for `export` display but are excluded from envp.
 *
 * @param  env  Head of the environment linked list.
 * @return Number of entries to serialize into execve's envp array.
 */
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

/**
 * @brief  Allocate a single "KEY=VALUE" environment string.
 * @param  key    Variable name.
 * @param  value  Variable value (may be "", must not be NULL).
 * @return Caller-owned "KEY=VALUE" string, or NULL on allocation failure.
 */
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

/**
 * @brief  Convert @p env into a NULL-terminated "KEY=VALUE" array for execve.
 *
 * Omits unassigned placeholders (value == NULL) such as initial OLDPWD or
 * bare `export VAR`, while preserving empty strings (value == "").
 *
 * @param  env  Head of the environment linked list.
 * @return Caller-owned string array (free with free_str_array), or NULL.
 */
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

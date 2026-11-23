/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_ops.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_env_node	*find_env_node(t_env_node *env, const char *key)
{
	while (env)
	{
		if (ft_strcmp(env->key, key) == 0)
			return (env);
		env = env->next;
	}
	return (NULL);
}

static t_env_node	*new_env_pair(const char *key, const char *value)
{
	t_env_node	*node;

	node = (t_env_node *)ft_calloc(1, sizeof(t_env_node));
	if (!node)
		return (NULL);
	node->key = ft_strdup(key);
	if (value)
		node->value = ft_strdup(value);
	if (!node->key || (value && !node->value))
	{
		free(node->key);
		free(node->value);
		free(node);
		return (NULL);
	}
	return (node);
}

static void	link_env_tail(t_env_node **env, t_env_node *node)
{
	t_env_node	*curr;

	if (!*env)
	{
		*env = node;
		return ;
	}
	curr = *env;
	while (curr->next)
		curr = curr->next;
	curr->next = node;
}

int	env_set(t_env_node **env, const char *key, const char *value)
{
	t_env_node	*node;
	char		*new_val;

	if (!env || !key)
		return (-1);
	node = find_env_node(*env, key);
	if (node)
	{
		if (!value)
			return (0);
		new_val = ft_strdup(value);
		if (!new_val)
			return (-1);
		free(node->value);
		node->value = new_val;
		return (0);
	}
	node = new_env_pair(key, value);
	if (!node)
		return (-1);
	link_env_tail(env, node);
	return (0);
}

int	env_unset(t_env_node **env, const char *key)
{
	t_env_node	*curr;
	t_env_node	*prev;

	if (!env || !*env || !key)
		return (0);
	curr = *env;
	prev = NULL;
	while (curr)
	{
		if (ft_strcmp(curr->key, key) == 0)
		{
			if (prev)
				prev->next = curr->next;
			else
				*env = curr->next;
			free(curr->key);
			free(curr->value);
			free(curr);
			return (0);
		}
		prev = curr;
		curr = curr->next;
	}
	return (0);
}

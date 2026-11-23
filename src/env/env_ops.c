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

/**
 * @file env_ops.c
 * @brief Environment variable insertion, update, and removal operations.
 */

/** @brief Locate the node matching @p key by exact name, or return NULL. */
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

/**
 * @brief  Allocate a new t_env_node duplicating @p key and optional @p value.
 * @param  key    Variable name (must not be NULL).
 * @param  value  Value string, or NULL for unassigned `export VAR` entries.
 * @return Newly allocated node, or NULL on allocation failure.
 */
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

/** @brief Link @p node at the tail of @p *env to preserve insertion order. */
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

/**
 * @brief  Insert or update @p key in the environment list @p *env.
 *
 * When @p key already exists and @p value is NULL (bare `export VAR`), the
 * existing value is intentionally preserved; passing "" overwrites it with
 * an empty string. New keys with @p value == NULL are created unassigned.
 *
 * @param  env    Address of the environment list head pointer.
 * @param  key    Variable name.
 * @param  value  New value string, "" for empty, or NULL for export-only.
 * @return 0 on success, -1 on invalid arguments or allocation failure.
 */
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

/**
 * @brief  Remove and free the node matching @p key from @p *env if present.
 * @param  env  Address of the environment list head pointer.
 * @param  key  Variable name to unset.
 * @return Always 0 (removing a non-existent key is a silent no-op).
 */
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

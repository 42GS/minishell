/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_core.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:35:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:35:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file env_core.c
 * @brief Environment linked-list initialization, teardown, and lookup.
 */

/**
 * @brief  Parse a raw "KEY=VALUE" or bare "KEY" string into a list node.
 *
 * Distinguishes "KEY=" (value == "") from bare "KEY" (value == NULL) so
 * empty values remain exported while unassigned names stay NULL.
 *
 * @param  entry  Raw environment entry string.
 * @return Newly allocated t_env_node, or NULL on allocation failure.
 */
static t_env_node	*create_env_node(const char *entry)
{
	t_env_node	*node;
	char		*eq;

	node = (t_env_node *)ft_calloc(1, sizeof(t_env_node));
	if (!node)
		return (NULL);
	eq = ft_strchr(entry, '=');
	if (eq)
	{
		node->key = ft_substr(entry, 0, (size_t)(eq - entry));
		node->value = ft_strdup(eq + 1);
	}
	else
		node->key = ft_strdup(entry);
	if (!node->key || (eq && !node->value))
	{
		free(node->key);
		free(node->value);
		free(node);
		return (NULL);
	}
	return (node);
}

/** @brief Append @p node to the list in O(1) using @p head and @p tail. */
static void	append_env_node(t_env_node **head, t_env_node **tail,
	t_env_node *node)
{
	if (!*head)
		*head = node;
	else
		(*tail)->next = node;
	*tail = node;
}

/**
 * @brief  Clone the host @p envp vector into a t_env_node linked list.
 * @param  envp  NULL-terminated host environment array (may be NULL/empty).
 * @return Head of the newly allocated list, or NULL if empty or on OOM.
 */
t_env_node	*init_env_list(char **envp)
{
	t_env_node	*head;
	t_env_node	*tail;
	t_env_node	*node;
	size_t		i;

	head = NULL;
	tail = NULL;
	i = 0;
	while (envp && envp[i])
	{
		node = create_env_node(envp[i]);
		if (!node)
		{
			free_env_list(&head);
			return (NULL);
		}
		append_env_node(&head, &tail, node);
		i++;
	}
	return (head);
}

/**
 * @brief  Free all nodes, keys, and values in @p *env_list and null it.
 * @param  env_list  Address of the list head pointer.
 */
void	free_env_list(t_env_node **env_list)
{
	t_env_node	*curr;
	t_env_node	*next;

	if (!env_list || !*env_list)
		return ;
	curr = *env_list;
	while (curr)
	{
		next = curr->next;
		free(curr->key);
		free(curr->value);
		free(curr);
		curr = next;
	}
	*env_list = NULL;
}

/**
 * @brief  Look up @p key in @p env and return its borrowed value pointer.
 * @param  env  Head of the environment list.
 * @param  key  Exact variable name to match.
 * @return Borrowed value string ("" if empty), or NULL if unset/unassigned.
 */
const char	*env_get(t_env_node *env, const char *key)
{
	if (!key)
		return (NULL);
	while (env)
	{
		if (ft_strcmp(env->key, key) == 0)
			return (env->value);
		env = env->next;
	}
	return (NULL);
}

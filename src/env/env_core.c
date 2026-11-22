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

static void	append_env_node(t_env_node **head, t_env_node **tail,
	t_env_node *node)
{
	if (!*head)
		*head = node;
	else
		(*tail)->next = node;
	*tail = node;
}

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

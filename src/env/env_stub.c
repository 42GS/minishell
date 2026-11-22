/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_stub.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_env_node	*init_env_list(char **envp)
{
	(void)envp;
	return (NULL);
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
	(void)env;
	(void)key;
	return (NULL);
}

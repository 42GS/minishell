/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_stub.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_token	*tokenize_input(const char *input, int *err_code)
{
	(void)input;
	if (err_code)
		*err_code = 0;
	return (NULL);
}

void	free_tokens(t_token **tokens)
{
	t_token	*curr;
	t_token	*next;

	if (!tokens || !*tokens)
		return ;
	curr = *tokens;
	while (curr)
	{
		next = curr->next;
		free(curr->value);
		free(curr);
		curr = next;
	}
	*tokens = NULL;
}

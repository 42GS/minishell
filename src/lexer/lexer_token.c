/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_token.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:50:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:50:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_token	*create_token(t_token_type type, char *value, int quoted)
{
	t_token	*tok;

	tok = (t_token *)malloc(sizeof(t_token));
	if (!tok)
		return (NULL);
	tok->type = type;
	tok->value = value;
	tok->quoted = quoted;
	tok->prev = NULL;
	tok->next = NULL;
	return (tok);
}

void	append_token(t_token **head, t_token *new_tok)
{
	t_token	*curr;

	if (!head || !new_tok)
		return ;
	if (!*head)
	{
		*head = new_tok;
		return ;
	}
	curr = *head;
	while (curr->next)
		curr = curr->next;
	curr->next = new_tok;
	new_tok->prev = curr;
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

void	*lexer_syntax_error(const char *msg, t_token **head, int *err_code)
{
	ft_putendl_fd((char *)msg, STDERR_FILENO);
	free_tokens(head);
	if (err_code)
		*err_code = 2;
	return (NULL);
}

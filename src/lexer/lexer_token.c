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

/**
 * @file lexer_token.c
 * @brief Token allocation, doubly-linked list helpers, and lexer errors.
 */

/**
 * @brief  Allocate a new token node taking ownership of @p value.
 * @param  type    Token kind (`TOK_WORD`, operator, or `TOK_EOF`).
 * @param  value   Heap-allocated lexeme string, or NULL for `TOK_EOF`.
 * @param  quoted  Bitmask of `QUOTE_SINGLE` / `QUOTE_DOUBLE` seen in token.
 * @return Newly allocated token, or NULL on malloc failure.
 */
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

/**
 * @brief  Append @p new_tok to the tail of doubly-linked list @p head.
 * @param  head     Address of the token list head pointer.
 * @param  new_tok  Token node to link at the end of the list.
 */
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

/**
 * @brief  Free an entire token list and null out @p tokens.
 * @param  tokens  Address of list head pointer; safe if NULL or empty.
 */
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

/**
 * @brief  Report a lexer syntax error, free partial tokens, and set status 2.
 * @param  msg       Diagnostic string printed to `STDERR_FILENO`.
 * @param  head      Token list head freed to prevent leaks on abort.
 * @param  err_code  Set to 2 (POSIX syntax error exit status) if non-NULL.
 * @return Always returns NULL for direct error propagation.
 */
void	*lexer_syntax_error(const char *msg, t_token **head, int *err_code)
{
	ft_putendl_fd((char *)msg, STDERR_FILENO);
	free_tokens(head);
	if (err_code)
		*err_code = 2;
	return (NULL);
}

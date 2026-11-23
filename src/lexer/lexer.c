/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:50:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:50:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file lexer.c
 * @brief Top-level lexical scanner turning raw input into a token list.
 */

/** @brief Return non-zero if @p c is unquoted shell whitespace. */
int	is_lexer_space(char c)
{
	return (c == ' ' || (c >= '\t' && c <= '\r'));
}

/**
 * @brief  Check whether @p s begins a shell operator token.
 * @param  s  Input substring at the current scan position.
 * @return 1 if `*s` starts an operator (`|`, `<`, `>`, `(`, `)`, `&`), else 0.
 *
 * Includes `&` so standalone `&` and `&&&` tokenize as `TOK_AMP` instead of
 * silently merging into adjacent words.
 */
int	is_operator_start(const char *s)
{
	if (!s || !*s)
		return (0);
	if (*s == '|' || *s == '<' || *s == '>' || *s == '(' || *s == ')'
		|| *s == '&')
		return (1);
	return (0);
}

/**
 * @brief  Extract one operator or word token and handle lexer errors.
 * @param  in    Full input string.
 * @param  i     In/out byte index into @p in.
 * @param  head  Token list head freed if extraction fails.
 * @param  err   Set to 2 on unclosed quote or 1 on allocation failure.
 * @return 1 on success, 0 on OOM, -1 on unclosed quote.
 */
static int	lexer_step(const char *in, size_t *i, t_token **head, int *err)
{
	int	ret;

	if (is_operator_start(&in[*i]))
		ret = extract_op_token(in, i, head);
	else
		ret = extract_word_token(in, i, head);
	if (ret == -1)
	{
		lexer_syntax_error("minishell: syntax error: unclosed quote", head,
			err);
		return (-1);
	}
	if (ret == 0)
	{
		free_tokens(head);
		if (err)
			*err = 1;
	}
	return (ret);
}

/**
 * @brief  Append the `TOK_EOF` sentinel so the parser has a uniform end.
 * @param  head      Token list head; freed if `TOK_EOF` allocation fails.
 * @param  err_code  Set to 1 on allocation failure.
 * @return `*head` on success, or NULL on allocation failure.
 */
static t_token	*append_eof(t_token **head, int *err_code)
{
	t_token	*eof_tok;

	eof_tok = create_token(TOK_EOF, NULL, QUOTE_NONE);
	if (!eof_tok)
	{
		free_tokens(head);
		if (err_code)
			*err_code = 1;
		return (NULL);
	}
	append_token(head, eof_tok);
	return (*head);
}

/**
 * @brief  Tokenize a shell input line into a `TOK_EOF`-terminated list.
 * @param  input     Raw input line (may be NULL or whitespace-only).
 * @param  err_code  Out status: 0 on success, 2 on unclosed quote, 1 on OOM.
 * @return Head of allocated token list, or NULL on error (list freed).
 */
t_token	*tokenize_input(const char *input, int *err_code)
{
	t_token	*head;
	size_t	i;
	int		ret;

	head = NULL;
	if (err_code)
		*err_code = 0;
	if (!input)
		return (append_eof(&head, err_code));
	i = 0;
	while (input[i])
	{
		while (input[i] && is_lexer_space(input[i]))
			i++;
		if (!input[i])
			break ;
		ret = lexer_step(input, &i, &head, err_code);
		if (ret <= 0)
			return (NULL);
	}
	return (append_eof(&head, err_code));
}

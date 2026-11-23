/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_op.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:50:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:50:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file lexer_op.c
 * @brief Longest-match operator tokenization for pipes, redirs, and logic.
 */

/**
 * @brief  Match a two-character operator at the start of @p in.
 * @param  in    Input slice starting at the current scan offset.
 * @param  type  Output token type when a 2-char operator matches.
 * @return 2 if a compound operator matched, 0 otherwise.
 *
 * Checked before single-character operators so `||`, `&&`, `<<`, and `>>`
 * are never split into separate single-character tokens.
 */
static size_t	match_two_char_op(const char *in, t_token_type *type)
{
	if (in[0] == '|' && in[1] == '|')
	{
		*type = TOK_OR_IF;
		return (2);
	}
	if (in[0] == '&' && in[1] == '&')
	{
		*type = TOK_AND_IF;
		return (2);
	}
	if (in[0] == '<' && in[1] == '<')
	{
		*type = TOK_HEREDOC;
		return (2);
	}
	if (in[0] == '>' && in[1] == '>')
	{
		*type = TOK_REDIR_APPEND;
		return (2);
	}
	return (0);
}

/**
 * @brief  Match a single-character operator on @p c.
 * @param  c     Character at the current scan offset.
 * @param  type  Output token type when a 1-char operator matches.
 * @return 1 if matched, 0 otherwise.
 *
 * Standalone `&` yields `TOK_AMP` so invalid `&` or `&&&` (`TOK_AND_IF` +
 * `TOK_AMP`) fails in the parser with syntax error status 2.
 */
static size_t	match_one_char_op(char c, t_token_type *type)
{
	if (c == '|')
		*type = TOK_PIPE;
	else if (c == '&')
		*type = TOK_AMP;
	else if (c == '<')
		*type = TOK_REDIR_IN;
	else if (c == '>')
		*type = TOK_REDIR_OUT;
	else if (c == '(')
		*type = TOK_LPAREN;
	else if (c == ')')
		*type = TOK_RPAREN;
	else
		return (0);
	return (1);
}

/**
 * @brief  Scan and append an operator token at offset @p i.
 * @param  in    Full input string.
 * @param  i     In/out scan index advanced by the matched operator length.
 * @param  head  Token list receiving the new operator token.
 * @return 1 on success, 0 on unmatched char or allocation failure.
 */
int	extract_op_token(const char *in, size_t *i, t_token **head)
{
	t_token_type	type;
	size_t			len;
	char			*val;
	t_token			*tok;

	len = match_two_char_op(&in[*i], &type);
	if (len == 0)
		len = match_one_char_op(in[*i], &type);
	if (len == 0)
		return (0);
	val = ft_substr(in, (unsigned int)(*i), len);
	if (!val)
		return (0);
	tok = create_token(type, val, QUOTE_NONE);
	if (!tok)
	{
		free(val);
		return (0);
	}
	append_token(head, tok);
	*i += len;
	return (1);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_word.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:50:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:50:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file lexer_word.c
 * @brief Quote-state DFA scanner for shell words and heredoc delimiters.
 */

/**
 * @brief  Advance the 3-state quote DFA on character @p c.
 * @param  c   Current character being scanned.
 * @param  st  In/out quote state (`GENERAL`, `SQUOTE`, or `DQUOTE`).
 * @return 1 if @p c toggled the quote state, 0 if treated as literal.
 *
 * Single quotes only toggle in `STATE_GENERAL`/`STATE_SQUOTE`, and double
 * quotes only in `STATE_GENERAL`/`STATE_DQUOTE`, keeping nested quotes raw.
 */
int	step_quote(char c, t_quote_state *st)
{
	if (*st == STATE_GENERAL && c == '\'')
		*st = STATE_SQUOTE;
	else if (*st == STATE_SQUOTE && c == '\'')
		*st = STATE_GENERAL;
	else if (*st == STATE_GENERAL && c == '"')
		*st = STATE_DQUOTE;
	else if (*st == STATE_DQUOTE && c == '"')
		*st = STATE_GENERAL;
	else
		return (0);
	return (1);
}

/**
 * @brief  Run one DFA step and record quote bitflags encountered.
 * @param  c       Current input character.
 * @param  state   In/out quote DFA state.
 * @param  quoted  Bitmask updated with `QUOTE_SINGLE` / `QUOTE_DOUBLE`.
 *
 * Recording which quote styles appeared preserves `quoted_delim` metadata
 * for heredoc delimiters without stripping quotes during lexing.
 */
static void	update_quote_state(char c, t_quote_state *state, int *quoted)
{
	if (!step_quote(c, state))
		return ;
	if (*state == STATE_SQUOTE)
		*quoted |= QUOTE_SINGLE;
	else if (*state == STATE_DQUOTE)
		*quoted |= QUOTE_DOUBLE;
}

/**
 * @brief  Scan until an unquoted delimiter ends the current word token.
 * @param  in      Full input string.
 * @param  i       In/out byte offset stopped at whitespace, op, or NUL.
 * @param  quoted  Output bitmask of quote styles seen in the word.
 * @return 1 when terminated in `STATE_GENERAL`, -1 if unclosed at EOF.
 */
static int	scan_word_end(const char *in, size_t *i, int *quoted)
{
	t_quote_state	state;

	state = STATE_GENERAL;
	*quoted = QUOTE_NONE;
	while (in[*i])
	{
		if (state == STATE_GENERAL
			&& (is_lexer_space(in[*i]) || is_operator_start(&in[*i])))
			break ;
		update_quote_state(in[*i], &state, quoted);
		(*i)++;
	}
	if (state != STATE_GENERAL)
		return (-1);
	return (1);
}

/**
 * @brief  Scan a word slice, allocate a `TOK_WORD` node, and append it.
 * @param  in    Full input string.
 * @param  i     In/out scan index advanced past the extracted word.
 * @param  head  Token list receiving the new `TOK_WORD` token.
 * @return 1 on success, 0 on OOM, -1 on unclosed quote.
 */
int	extract_word_token(const char *in, size_t *i, t_token **head)
{
	size_t	start;
	int		quoted;
	char	*word;
	t_token	*tok;

	start = *i;
	if (scan_word_end(in, i, &quoted) < 0)
		return (-1);
	word = ft_substr(in, (unsigned int)start, *i - start);
	if (!word)
		return (0);
	tok = create_token(TOK_WORD, word, quoted);
	if (!tok)
	{
		free(word);
		return (0);
	}
	append_token(head, tok);
	return (1);
}

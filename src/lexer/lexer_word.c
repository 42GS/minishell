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

static void	update_quote_state(char c, t_quote_state *state, int *quoted)
{
	if (*state == STATE_GENERAL && c == '\'')
	{
		*state = STATE_SQUOTE;
		*quoted |= QUOTE_SINGLE;
	}
	else if (*state == STATE_SQUOTE && c == '\'')
		*state = STATE_GENERAL;
	else if (*state == STATE_GENERAL && c == '"')
	{
		*state = STATE_DQUOTE;
		*quoted |= QUOTE_DOUBLE;
	}
	else if (*state == STATE_DQUOTE && c == '"')
		*state = STATE_GENERAL;
}

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

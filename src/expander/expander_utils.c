/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:48:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 13:55:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file expander_utils.c
 * @brief Quote-stepping and string-buffer helpers for word expansion.
 */

/**
 * @brief  Detect $"..." or $'...' prefix in unquoted state.
 *
 * Allows the expander to drop the leading '$' so the following quoted
 * segment is processed normally without leaving a literal '$'.
 *
 * @param  s   Input word string.
 * @param  i   Current index in @p s.
 * @param  st  Current quote state.
 * @return true if @p s[@p i] is '$' before a quote in STATE_GENERAL.
 */
bool	is_dollar_quote(const char *s, size_t i, t_quote_state st)
{
	return (st == STATE_GENERAL && s[i] == '$'
		&& (s[i + 1] == '"' || s[i + 1] == '\''));
}

/**
 * @brief  Append heap-allocated @p chunk to @p sb and free @p chunk.
 *
 * If @p chunk is NULL (upstream allocation failure), frees @p sb as well so
 * callers can propagate OOM in a single conditional check.
 *
 * @param  sb     Target string buffer.
 * @param  chunk  Heap string to append and free (may be NULL).
 * @return 1 on success, 0 on NULL @p chunk or buffer growth failure.
 */
int	sb_push_str_free(t_strbuf *sb, char *chunk)
{
	int	ok;

	if (!chunk)
	{
		sb_free(sb);
		return (0);
	}
	ok = sb_push_str(sb, chunk);
	free(chunk);
	return (ok);
}

/**
 * @brief  Remove syntactic quotes from @p raw without expanding '$'.
 *
 * Used for heredoc delimiters where quoting suppresses body expansion
 * but parameter substitution must not occur on the delimiter itself.
 *
 * @param  raw  Raw delimiter or word string.
 * @return Caller-owned unquoted string, or NULL on allocation failure.
 */
char	*strip_quotes_only(const char *raw)
{
	t_quote_state	st;
	size_t			i;
	t_strbuf		sb;

	ft_bzero(&sb, sizeof(t_strbuf));
	if (!sb_reserve(&sb, 0))
		return (NULL);
	st = STATE_GENERAL;
	i = 0;
	while (raw && raw[i])
	{
		if (step_quote(raw[i], &st))
			i++;
		else if (!sb_push_char(&sb, raw[i++]))
			return (NULL);
	}
	return (sb.data);
}

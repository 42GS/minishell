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

bool	is_dollar_quote(const char *s, size_t i, t_quote_state st)
{
	return (st == STATE_GENERAL && s[i] == '$'
		&& (s[i + 1] == '"' || s[i + 1] == '\''));
}

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

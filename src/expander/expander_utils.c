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

int	append_char_str(char **res, char c)
{
	char	buf[2];
	char	*joined;

	buf[0] = c;
	buf[1] = '\0';
	joined = ft_strjoin(*res, buf);
	free(*res);
	*res = joined;
	return (joined != NULL);
}

int	append_str_free(char **res, char *chunk)
{
	char	*joined;

	if (!chunk)
	{
		free(*res);
		*res = NULL;
		return (0);
	}
	joined = ft_strjoin(*res, chunk);
	free(*res);
	free(chunk);
	*res = joined;
	return (joined != NULL);
}

char	*strip_quotes_only(const char *raw)
{
	t_quote_state	st;
	size_t			i;
	char			*res;

	res = ft_strdup("");
	st = STATE_GENERAL;
	i = 0;
	while (res && raw && raw[i])
	{
		if (step_quote(raw[i], &st))
			i++;
		else if (!append_char_str(&res, raw[i++]))
			return (NULL);
	}
	return (res);
}

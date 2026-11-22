/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:48:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:48:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_var_char(char c, int first)
{
	if (first)
		return (ft_isalpha(c) || c == '_');
	return (ft_isalnum(c) || c == '_');
}

static char	*expand_dollar(const char *s, size_t *i, t_shell *ctx)
{
	size_t		start;
	char		*name;
	const char	*val;

	(*i)++;
	if (s[*i] == '?')
	{
		(*i)++;
		return (ft_itoa(ctx->last_exit_status));
	}
	if (!is_var_char(s[*i], 1))
		return (ft_strdup("$"));
	start = *i;
	while (s[*i] && is_var_char(s[*i], 0))
		(*i)++;
	name = ft_substr(s, (unsigned int)start, *i - start);
	if (!name)
		return (NULL);
	val = env_get(ctx->env_list, name);
	free(name);
	if (!val)
		return (ft_strdup(""));
	return (ft_strdup(val));
}

static int	step_quote(char c, t_quote_state *st)
{
	if (*st == STATE_GENERAL && c == '\'')
	{
		*st = STATE_SQUOTE;
		return (1);
	}
	if (*st == STATE_SQUOTE && c == '\'')
	{
		*st = STATE_GENERAL;
		return (1);
	}
	if (*st == STATE_GENERAL && c == '"')
	{
		*st = STATE_DQUOTE;
		return (1);
	}
	if (*st == STATE_DQUOTE && c == '"')
	{
		*st = STATE_GENERAL;
		return (1);
	}
	return (0);
}

char	*expand_word(const char *raw, t_shell *ctx)
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
		else if (raw[i] == '$' && st != STATE_SQUOTE)
		{
			if (!append_str_free(&res, expand_dollar(raw, &i, ctx)))
				return (NULL);
		}
		else if (!append_char_str(&res, raw[i++]))
			return (NULL);
	}
	return (res);
}

char	*expand_heredoc_line(const char *raw, t_shell *ctx)
{
	size_t	i;
	char	*res;

	res = ft_strdup("");
	i = 0;
	while (res && raw && raw[i])
	{
		if (raw[i] == '$')
		{
			if (!append_str_free(&res, expand_dollar(raw, &i, ctx)))
				return (NULL);
		}
		else if (!append_char_str(&res, raw[i++]))
			return (NULL);
	}
	return (res);
}

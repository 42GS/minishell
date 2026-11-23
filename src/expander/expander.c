/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:48:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 13:55:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_var_char(char c, int first)
{
	if (first)
		return (ft_isalpha(c) || c == '_');
	return (ft_isalnum(c) || c == '_');
}

static char	*expand_special_dollar(const char *s, size_t *i, t_shell *ctx)
{
	if (s[*i] == '?')
	{
		(*i)++;
		return (ft_itoa(ctx->last_exit_status));
	}
	(*i)++;
	if (s[*i - 1] == '0')
		return (ft_strdup("minishell"));
	return (ft_strdup(""));
}

char	*expand_dollar(const char *s, size_t *i, t_shell *ctx)
{
	size_t		start;
	char		*name;
	const char	*val;

	(*i)++;
	if (s[*i] == '?' || ft_isdigit((unsigned char)s[*i]))
		return (expand_special_dollar(s, i, ctx));
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

char	*expand_word(const char *raw, t_shell *ctx)
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
		if (step_quote(raw[i], &st) || is_dollar_quote(raw, i, st))
			i++;
		else if (raw[i] == '$' && st != STATE_SQUOTE)
		{
			if (!sb_push_str_free(&sb, expand_dollar(raw, &i, ctx)))
				return (NULL);
		}
		else if (!sb_push_char(&sb, raw[i++]))
			return (NULL);
	}
	return (sb.data);
}

char	*expand_heredoc_line(const char *raw, t_shell *ctx)
{
	size_t		i;
	t_strbuf	sb;

	ft_bzero(&sb, sizeof(t_strbuf));
	if (!sb_reserve(&sb, 0))
		return (NULL);
	i = 0;
	while (raw && raw[i])
	{
		if (raw[i] == '$')
		{
			if (!sb_push_str_free(&sb, expand_dollar(raw, &i, ctx)))
				return (NULL);
		}
		else if (!sb_push_char(&sb, raw[i++]))
			return (NULL);
	}
	return (sb.data);
}

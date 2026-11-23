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

/**
 * @file expander.c
 * @brief Core parameter ($VAR, $?, $0) and quote-stripping word expansion.
 */

/** @brief Return non-zero if @p c is valid in a POSIX variable name. */
static int	is_var_char(char c, int first)
{
	if (first)
		return (ft_isalpha(c) || c == '_');
	return (ft_isalnum(c) || c == '_');
}

/**
 * @brief  Expand special single-character parameters ($?, $0, $1..$9).
 * @param  s    Input string positioned right after '$'.
 * @param  i    Cursor advanced past the special parameter character.
 * @param  ctx  Shell state providing last_exit_status for $?.
 * @return Newly allocated expansion string, or NULL on OOM.
 */
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

/**
 * @brief  Parse and expand a '$' parameter reference starting at @p *i.
 *
 * Advances @p *i past '$' and the variable token. Preserves a literal "$"
 * when not followed by a valid name start, digit, or '?'. Unset variables
 * expand to an empty string rather than NULL.
 *
 * @param  s    Source string containing '$'.
 * @param  i    In/out index initially pointing at '$'.
 * @param  ctx  Shell context holding env_list and last_exit_status.
 * @return Caller-owned expanded string, or NULL on allocation failure.
 */
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

/**
 * @brief  Expand variables and strip quotes into a single string.
 *
 * Performs quote tracking, $"..." / $'...' prefix removal, and '$'
 * expansion outside single quotes without IFS splitting or globbing.
 * Used where a single word must stay intact (e.g., export assignments).
 *
 * @param  raw  Raw token string to expand.
 * @param  ctx  Shell context for variable lookup.
 * @return Caller-owned expanded string, or NULL on allocation failure.
 */
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

/**
 * @brief  Expand '$' variables in an unquoted heredoc body line.
 *
 * Single and double quotes inside heredoc bodies are literal characters
 * in POSIX shells and therefore never suppress '$' expansion or get
 * stripped here.
 *
 * @param  raw  Heredoc line to expand.
 * @param  ctx  Shell context for variable lookup.
 * @return Caller-owned expanded line, or NULL on allocation failure.
 */
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

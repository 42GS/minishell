/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_split.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 13:55:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 13:55:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	flush_cur_field(t_exp_buf *b)
{
	if (!b->cur.data)
		return (1);
	if (!append_str_to_argv(&b->fields, b->cur.data))
		return (0);
	sb_free(&b->cur);
	return (1);
}

static int	push_char_to_cur(t_exp_buf *b, char c)
{
	if (c == '\0')
		return (sb_reserve(&b->cur, 0));
	return (sb_push_char(&b->cur, c));
}

static int	feed_split_chunk(t_exp_buf *b, char *chunk)
{
	size_t	k;
	int		ok;

	k = 0;
	ok = 1;
	while (ok && chunk[k])
	{
		if (chunk[k] == ' ' || chunk[k] == '\t' || chunk[k] == '\n')
			ok = flush_cur_field(b);
		else
			ok = push_char_to_cur(b, chunk[k]);
		k++;
	}
	free(chunk);
	return (ok);
}

static int	step_expand_dollar(const char *raw, size_t *i, t_exp_buf *b,
	t_shell *ctx)
{
	char	*chunk;
	char	next;
	bool	split;

	if (is_dollar_quote(raw, *i, b->st))
	{
		(*i)++;
		return (1);
	}
	next = raw[*i + 1];
	split = (b->st == STATE_GENERAL && (next == '?'
				|| ft_isdigit((unsigned char)next)
				|| ft_isalpha((unsigned char)next) || next == '_'));
	chunk = expand_dollar(raw, i, ctx);
	if (!chunk)
		return (0);
	if (split)
		return (feed_split_chunk(b, chunk));
	return (sb_push_str_free(&b->cur, chunk));
}

char	**expand_word_fields(const char *raw, t_shell *ctx, int *err)
{
	t_exp_buf	b;
	size_t		i;
	int			ok;

	ft_bzero(&b, sizeof(t_exp_buf));
	*err = 0;
	i = 0;
	ok = 1;
	while (ok && raw && raw[i])
	{
		if (step_quote(raw[i], &b.st) && ++i)
			ok = push_char_to_cur(&b, '\0');
		else if (raw[i] == '$' && b.st != STATE_SQUOTE)
			ok = step_expand_dollar(raw, &i, &b, ctx);
		else
			ok = push_char_to_cur(&b, raw[i++]);
	}
	if (ok && flush_cur_field(&b))
		return (b.fields);
	sb_free(&b.cur);
	free_str_array(b.fields);
	*err = 1;
	return (NULL);
}

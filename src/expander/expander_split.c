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
	int	ok;

	if (!b->cur.data)
		return (1);
	if (b->has_wild)
		ok = expand_glob_field(b->cur.data, b->mask.data, &b->fields);
	else
		ok = append_str_to_argv(&b->fields, b->cur.data);
	sb_free(&b->cur);
	sb_free(&b->mask);
	b->has_wild = false;
	return (ok);
}

static int	push_char_to_cur(t_exp_buf *b, char c, bool wild)
{
	char	m;

	if (c == '\0')
		return (sb_reserve(&b->cur, 0) && sb_reserve(&b->mask, 0));
	m = '0';
	if (wild && c == '*')
	{
		m = '1';
		b->has_wild = true;
	}
	return (sb_push_char(&b->cur, c) && sb_push_char(&b->mask, m));
}

static int	feed_chunk(t_exp_buf *b, char *chunk, bool split)
{
	size_t	k;
	int		ok;

	ok = (split || push_char_to_cur(b, '\0', false));
	k = 0;
	while (ok && chunk[k])
	{
		if (split && (chunk[k] == ' ' || chunk[k] == '\t' || chunk[k] == '\n'))
			ok = flush_cur_field(b);
		else
			ok = push_char_to_cur(b, chunk[k], split);
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
	return (feed_chunk(b, chunk, split));
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
			ok = push_char_to_cur(&b, '\0', false);
		else if (raw[i] == '$' && b.st != STATE_SQUOTE)
			ok = step_expand_dollar(raw, &i, &b, ctx);
		else
			ok = push_char_to_cur(&b, raw[i++], b.st == STATE_GENERAL);
	}
	if (ok && flush_cur_field(&b))
		return (b.fields);
	sb_free(&b.cur);
	sb_free(&b.mask);
	free_str_array(b.fields);
	*err = 1;
	return (NULL);
}

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

/**
 * @file expander_split.c
 * @brief Quote-aware IFS word splitting and parallel wildcard mask builder.
 */

/**
 * @brief  Finalize the accumulated field in @p b and reset its buffers.
 *
 * Skips flushing when b->cur.data is NULL (no characters or quoted empty
 * string encountered). Dispatches to expand_glob_field() when unquoted '*'
 * was recorded, otherwise appends the literal field directly.
 *
 * @param  b  Expansion buffer state holding cur, mask, and output fields.
 * @return 1 on success, 0 on allocation failure.
 */
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

/**
 * @brief  Append @p c and its parallel wildcard flag ('1'/'0') to @p b.
 *
 * Passing @p c == '\0' allocates empty non-NULL buffers in cur and mask so
 * quoted empty strings ("" or '') are preserved as an explicit empty field
 * rather than being elided like unquoted empty expansions.
 *
 * @param  b     Expansion buffer state.
 * @param  c     Character to append, or '\0' to materialize an empty field.
 * @param  wild  True if @p c came from an unquoted context (enables '*').
 * @return 1 on success, 0 on allocation failure.
 */
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

/**
 * @brief  Feed an expanded '$' @p chunk into @p b and free @p chunk.
 *
 * When @p split is true (unquoted variable), space/tab/newline split fields
 * and '*' characters are marked wild ('1'). When @p split is false (inside
 * double quotes), an empty buffer is reserved so "$EMPTY" yields "".
 *
 * @param  b      Expansion buffer state.
 * @param  chunk  Heap-allocated expansion string (always freed).
 * @param  split  Whether IFS splitting and wildcard marking apply.
 * @return 1 on success, 0 on allocation failure.
 */
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

/**
 * @brief  Expand a '$' sequence during field-splitting traversal.
 *
 * Strips '$' in $"..."/$'...' prefixes. Enables IFS splitting and wildcard
 * tracking only in STATE_GENERAL for valid parameter names, digits, or '?'.
 *
 * @param  raw  Original unexpanded word.
 * @param  i    In/out cursor pointing at '$'.
 * @param  b    Expansion buffer state holding current quote state.
 * @param  ctx  Shell context for variable lookup.
 * @return 1 on success, 0 on allocation failure.
 */
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

/**
 * @brief  Expand @p raw into IFS-split and glob-expanded fields.
 *
 * Maintains a parallel wildcard mask alongside the character buffer inside
 * t_exp_buf so quoted '*' never globs while unquoted '*' (literal or from
 * unquoted $VAR) does. Sets @p *err to 1 and returns NULL on OOM.
 *
 * @param  raw  Raw word to expand.
 * @param  ctx  Shell context for variable lookup.
 * @param  err  Set to 0 on success, 1 on allocation failure.
 * @return NULL-terminated array of fields (may be NULL if empty), or NULL.
 */
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

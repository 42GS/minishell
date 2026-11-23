/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strbuf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 17:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 17:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/**
 * @file ft_strbuf.c
 * @brief Dynamically growing string buffer implemented without `realloc`.
 */

/**
 * @brief  Compute next buffer capacity >= `need` via geometric doubling.
 *
 * Starts at 32 bytes and doubles so repeated appends remain amortized
 * O(1) even without `realloc` (disallowed in 42 projects).
 *
 * @param  cur   Current capacity in bytes (0 for uninitialized buffer).
 * @param  need  Minimum required byte capacity including NUL terminator.
 * @return Grown capacity >= `need`.
 */
static size_t	calc_cap(size_t cur, size_t need)
{
	if (cur == 0)
		cur = 32;
	while (cur < need)
		cur *= 2;
	return (cur);
}

/**
 * @brief  Free `sb->data` and reset buffer metadata to empty zero state.
 *
 * Resetting fields prevents dangling pointers on reuse or repeat cleanup.
 *
 * @param  sb  String buffer to release (must be non-NULL).
 */
void	sb_free(t_strbuf *sb)
{
	free(sb->data);
	sb->data = NULL;
	sb->len = 0;
	sb->cap = 0;
}

/**
 * @brief  Ensure `sb` holds `need` extra bytes plus a NUL terminator.
 *
 * Because `realloc` is disallowed, growth allocates a new buffer via
 * `malloc`, copies `sb->len` bytes, and frees the old buffer. On OOM,
 * `sb` itself is freed via `sb_free` so callers need no cleanup on `0`.
 *
 * @param  sb    Target zero-initialized or active string buffer.
 * @param  need  Additional payload bytes required beyond `sb->len`.
 * @return `1` on success, `0` on `malloc` failure (`sb` freed).
 */
int	sb_reserve(t_strbuf *sb, size_t need)
{
	size_t	new_cap;
	char	*buf;
	size_t	i;

	if (sb->data && sb->len + need + 1 <= sb->cap)
		return (1);
	new_cap = calc_cap(sb->cap, sb->len + need + 1);
	buf = (char *)malloc(new_cap);
	if (!buf)
	{
		sb_free(sb);
		return (0);
	}
	i = 0;
	while (i < sb->len)
	{
		buf[i] = sb->data[i];
		i++;
	}
	buf[sb->len] = '\0';
	free(sb->data);
	sb->data = buf;
	sb->cap = new_cap;
	return (1);
}

/**
 * @brief  Append single character `c` while keeping `sb->data` NUL-ended.
 *
 * @param  sb  Target string buffer (freed automatically on OOM).
 * @param  c   Character byte to append.
 * @return `1` on success, `0` on allocation failure (`sb` freed).
 */
int	sb_push_char(t_strbuf *sb, char c)
{
	if (!sb_reserve(sb, 1))
		return (0);
	sb->data[sb->len] = c;
	sb->len++;
	sb->data[sb->len] = '\0';
	return (1);
}

/**
 * @brief  Append C-string `s` to `sb`, treating `NULL` as a safe no-op.
 *
 * Keeps `sb->data` NUL-terminated; caller retains ownership of `sb->data`
 * on success and must call `sb_free` (or `free`) when done.
 *
 * @param  sb  Target string buffer (freed automatically on OOM).
 * @param  s   String to append, or `NULL` (no-op returning `1`).
 * @return `1` on success, `0` on allocation failure (`sb` freed).
 */
int	sb_push_str(t_strbuf *sb, const char *s)
{
	size_t	add;
	size_t	i;

	if (!s)
		return (1);
	add = ft_strlen(s);
	if (!sb_reserve(sb, add))
		return (0);
	i = 0;
	while (i < add)
	{
		sb->data[sb->len + i] = s[i];
		i++;
	}
	sb->len += add;
	sb->data[sb->len] = '\0';
	return (1);
}

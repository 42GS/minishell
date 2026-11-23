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

static size_t	calc_cap(size_t cur, size_t need)
{
	if (cur == 0)
		cur = 32;
	while (cur < need)
		cur *= 2;
	return (cur);
}

void	sb_free(t_strbuf *sb)
{
	free(sb->data);
	sb->data = NULL;
	sb->len = 0;
	sb->cap = 0;
}

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

int	sb_push_char(t_strbuf *sb, char c)
{
	if (!sb_reserve(sb, 1))
		return (0);
	sb->data[sb->len] = c;
	sb->len++;
	sb->data[sb->len] = '\0';
	return (1);
}

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

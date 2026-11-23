/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_substr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/**
 * @file ft_substr.c
 * @brief Bounds-clamped substring extraction.
 */

/**
 * @brief  Allocate a substring of `s` starting at `start` up to `len`.
 *
 * Clamps `len` to `slen - start` so `malloc` never over-allocates beyond
 * the actual remaining characters, and returns `""` if `start >= slen`.
 *
 * @param  s      Source string (returns `NULL` if `NULL`).
 * @param  start  Zero-based start index within `s`.
 * @param  len    Maximum number of bytes to copy.
 * @return Caller-owned substring (`free` required), or `NULL` on `NULL`
 *         input or allocation failure.
 */
char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	char	*sub;
	size_t	slen;
	size_t	i;

	if (!s)
		return (NULL);
	slen = ft_strlen(s);
	if (start >= slen)
		return (ft_strdup(""));
	if (len > slen - start)
		len = slen - start;
	sub = (char *)malloc(sizeof(char) * (len + 1));
	if (!sub)
		return (NULL);
	i = 0;
	while (i < len)
	{
		sub[i] = s[start + i];
		i++;
	}
	sub[i] = '\0';
	return (sub);
}

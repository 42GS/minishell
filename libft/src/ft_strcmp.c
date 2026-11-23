/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strcmp.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/**
 * @file ft_strcmp.c
 * @brief Unsigned-byte lexicographical string comparison utilities.
 */

/**
 * @brief  Compare `s1` and `s2` lexicographically using `unsigned char`.
 *
 * Unsigned comparison avoids negative differences for bytes >= 0x80 on
 * platforms where `char` is signed.
 *
 * @param  s1  First non-NULL NUL-terminated string.
 * @param  s2  Second non-NULL NUL-terminated string.
 * @return Negative, zero, or positive integer for `<`, `==`, or `>`.
 */
int	ft_strcmp(const char *s1, const char *s2)
{
	size_t	i;

	i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

/**
 * @brief  Compare at most `n` bytes of `s1` and `s2` as `unsigned char`.
 *
 * Stops at the first mismatch, NUL terminator in both strings, or `n`.
 *
 * @param  s1  First non-NULL string.
 * @param  s2  Second non-NULL string.
 * @param  n   Maximum byte count to compare.
 * @return `0` if prefixes match (or `n == 0`), else signed byte difference.
 */
int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n && (s1[i] || s2[i]))
	{
		if (s1[i] != s2[i])
			return ((unsigned char)s1[i] - (unsigned char)s2[i]);
		i++;
	}
	return (0);
}

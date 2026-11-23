/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/**
 * @file ft_strchr.c
 * @brief Forward character search in a C-string.
 */

/**
 * @brief  Locate the first occurrence of `(char)c` in `s`.
 *
 * Explicitly checks the terminating NUL after the loop so callers can
 * search for `'\0'` per POSIX `strchr` semantics.
 *
 * @param  s  Non-NULL NUL-terminated string to scan.
 * @param  c  Character value (converted to `char`) to find.
 * @return Pointer into `s` at the first match, or `NULL` if absent.
 */
char	*ft_strchr(const char *s, int c)
{
	while (*s)
	{
		if (*s == (char)c)
			return ((char *)s);
		s++;
	}
	if ((char)c == '\0')
		return ((char *)s);
	return (NULL);
}

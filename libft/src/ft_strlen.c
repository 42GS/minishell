/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlen.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/**
 * @file ft_strlen.c
 * @brief NULL-safe string length helper.
 */

/**
 * @brief  Return length of `s` in bytes, treating `NULL` as length `0`.
 *
 * Returning `0` on `NULL` avoids crashes in defensive string helpers
 * (unlike standard `strlen`).
 *
 * @param  s  NUL-terminated string, or `NULL`.
 * @return Byte length excluding NUL, or `0` if `s` is `NULL`.
 */
size_t	ft_strlen(const char *s)
{
	size_t	i;

	if (!s)
		return (0);
	i = 0;
	while (s[i])
		i++;
	return (i);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/**
 * @file ft_calloc.c
 * @brief Zero-initializing memory allocation utilities.
 */

/**
 * @brief  Zero `n` bytes starting at `s` via byte-wise writes.
 *
 * Uses `unsigned char *` to avoid alignment and strict-aliasing issues.
 *
 * @param  s  Destination buffer (must be valid when `n > 0`).
 * @param  n  Number of bytes to clear (0 is a safe no-op).
 */
void	ft_bzero(void *s, size_t n)
{
	unsigned char	*ptr;
	size_t			i;

	ptr = (unsigned char *)s;
	i = 0;
	while (i < n)
	{
		ptr[i] = 0;
		i++;
	}
}

/**
 * @brief  Allocate zeroed array memory with multiplication overflow guard.
 *
 * Rejects `count * size` wraparound before calling `malloc` so an
 * attacker- or input-driven size cannot yield an undersized buffer.
 *
 * @param  count  Number of elements to allocate.
 * @param  size   Byte size of each element.
 * @return Caller-owned zeroed buffer (`free` required), or `NULL` on
 *         overflow or `malloc` failure.
 */
void	*ft_calloc(size_t count, size_t size)
{
	void	*ptr;

	if (count != 0 && size > ((size_t)-1) / count)
		return (NULL);
	ptr = malloc(count * size);
	if (!ptr)
		return (NULL);
	ft_bzero(ptr, count * size);
	return (ptr);
}

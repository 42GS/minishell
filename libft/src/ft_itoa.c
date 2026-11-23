/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/**
 * @file ft_itoa.c
 * @brief Signed integer to decimal string conversion.
 */

/**
 * @brief  Count characters needed for `n`, including a leading sign.
 *
 * Accepts `long` so `INT_MIN` can be negated without signed overflow.
 *
 * @param  n  Widened integer value to measure.
 * @return Character count excluding the terminating NUL byte.
 */
static int	count_digits(long n)
{
	int	len;

	len = 1;
	if (n < 0)
	{
		len++;
		n = -n;
	}
	while (n >= 10)
	{
		n /= 10;
		len++;
	}
	return (len);
}

/**
 * @brief  Convert signed integer `n` into a newly allocated string.
 *
 * Widens `n` to `long` so `-INT_MIN` never triggers signed overflow UB,
 * then fills digits right-to-left into an exact-sized buffer.
 *
 * @param  n  Integer value to format in base 10.
 * @return Caller-owned NUL-terminated string (`free` required), or `NULL`
 *         on allocation failure.
 */
char	*ft_itoa(int n)
{
	long	nb;
	int		len;
	char	*str;

	nb = (long)n;
	len = count_digits(nb);
	str = (char *)malloc(sizeof(char) * (len + 1));
	if (!str)
		return (NULL);
	str[len] = '\0';
	if (nb < 0)
	{
		str[0] = '-';
		nb = -nb;
	}
	if (nb == 0)
		str[0] = '0';
	while (nb > 0)
	{
		str[--len] = (nb % 10) + '0';
		nb /= 10;
	}
	return (str);
}

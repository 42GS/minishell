/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_put_fd.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/**
 * @file ft_put_fd.c
 * @brief File-descriptor string output utilities.
 */

/**
 * @brief  Write string `s` to `fd`, ignoring `NULL` or invalid descriptors.
 *
 * Guards prevent `ft_strlen` or `write` errors on uninitialized inputs.
 *
 * @param  s   NUL-terminated string to write (no-op if `NULL`).
 * @param  fd  Target file descriptor (no-op if `< 0`).
 */
void	ft_putstr_fd(char *s, int fd)
{
	if (!s || fd < 0)
		return ;
	write(fd, s, ft_strlen(s));
}

/**
 * @brief  Write string `s` followed by a newline to `fd`.
 *
 * Guards `NULL` and negative `fd`; writes `\n` directly to avoid a
 * temporary heap allocation for error/diagnostic output.
 *
 * @param  s   NUL-terminated string to write (no-op if `NULL`).
 * @param  fd  Target file descriptor (no-op if `< 0`).
 */
void	ft_putendl_fd(char *s, int fd)
{
	if (!s || fd < 0)
		return ;
	write(fd, s, ft_strlen(s));
	write(fd, "\n", 1);
}

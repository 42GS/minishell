/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIBFT_H
# define LIBFT_H

# include <stdlib.h>
# include <unistd.h>

/**
 * @file libft.h
 * @brief Core string, memory, I/O, and dynamic string-buffer utilities.
 */

/**
 * @struct s_strbuf
 * @brief  Growable NUL-terminated byte buffer minimizing reallocations.
 *
 * Must be zero-initialized before first use. Callers may take ownership of
 * data directly on completion or release it via sb_free().
 *
 * Key fields:
 * - data: Heap buffer (NULL until first sb_reserve/push).
 * - len: Current string length excluding NUL terminator.
 * - cap: Allocated byte capacity.
 */
typedef struct s_strbuf
{
	char	*data;
	size_t	len;
	size_t	cap;
}	t_strbuf;

/**
 * @brief  Free sb->data and reset len and cap to zero.
 *
 * @param  sb  String buffer to clear (must be non-NULL).
 */
void	sb_free(t_strbuf *sb);

/**
 * @brief  Ensure capacity for need extra bytes plus NUL via 2x growth.
 *
 * Frees sb via sb_free() if allocation fails so callers avoid cleanup leaks.
 *
 * @param  sb    Target string buffer.
 * @param  need  Additional bytes required beyond sb->len.
 * @return 1 on success, 0 on malloc failure (sb freed).
 */
int		sb_reserve(t_strbuf *sb, size_t need);

/**
 * @brief  Append one byte c and trailing NUL to sb.
 *
 * @param  sb  Target string buffer.
 * @param  c   Byte to append.
 * @return 1 on success, 0 on allocation failure (sb freed).
 */
int		sb_push_char(t_strbuf *sb, char c);

/**
 * @brief  Append NUL-terminated string s to sb (NULL is a safe no-op).
 *
 * @param  sb  Target string buffer.
 * @param  s   Source string to append (may be NULL).
 * @return 1 on success or NULL s, 0 on allocation failure (sb freed).
 */
int		sb_push_str(t_strbuf *sb, const char *s);

/** @brief Return length of s in bytes (0 if s is NULL). */
size_t	ft_strlen(const char *s);

/**
 * @brief  Duplicate s1 into a newly allocated string.
 *
 * @param  s1  Source string (NULL returns NULL).
 * @return Caller-owned duplicate, or NULL on NULL input or OOM.
 */
char	*ft_strdup(const char *s1);

/**
 * @brief  Allocate a substring of s starting at start up to len bytes.
 *
 * @param  s      Source string (NULL returns NULL).
 * @param  start  Starting byte offset (>= strlen(s) yields empty "").
 * @param  len    Maximum bytes to copy.
 * @return Caller-owned substring, or NULL on NULL input or OOM.
 */
char	*ft_substr(char const *s, unsigned int start, size_t len);

/**
 * @brief  Concatenate s1 and s2 into a newly allocated string.
 *
 * @param  s1  Prefix string (must be non-NULL).
 * @param  s2  Suffix string (must be non-NULL).
 * @return Caller-owned concatenation, or NULL on NULL input or OOM.
 */
char	*ft_strjoin(char const *s1, char const *s2);

/** @brief Compare s1 and s2 lexicographically as unsigned chars. */
int		ft_strcmp(const char *s1, const char *s2);

/** @brief Compare up to n bytes of s1 and s2 as unsigned chars. */
int		ft_strncmp(const char *s1, const char *s2, size_t n);

/**
 * @brief  Locate first occurrence of (char)c in s (including trailing '\\0').
 *
 * @param  s  NUL-terminated string to scan (must be non-NULL).
 * @param  c  Byte value converted to char.
 * @return Pointer into s at match, or NULL if absent.
 */
char	*ft_strchr(const char *s, int c);

/**
 * @brief  Allocate zeroed array memory with multiplication overflow guard.
 *
 * @param  count  Element count.
 * @param  size   Byte size per element.
 * @return Caller-owned zeroed buffer, or NULL on overflow or OOM.
 */
void	*ft_calloc(size_t count, size_t size);

/** @brief Zero n bytes starting at s (n == 0 is a safe no-op). */
void	ft_bzero(void *s, size_t n);

/** @brief Write s to fd, ignoring NULL s or negative fd. */
void	ft_putstr_fd(char *s, int fd);

/** @brief Write s followed by '\\n' to fd, ignoring NULL s or negative fd. */
void	ft_putendl_fd(char *s, int fd);

/** @brief Return non-zero if c is an ASCII alphabetic letter. */
int		ft_isalpha(int c);

/** @brief Return non-zero if c is an ASCII decimal digit. */
int		ft_isdigit(int c);

/** @brief Return non-zero if c is an ASCII letter or decimal digit. */
int		ft_isalnum(int c);

/**
 * @brief  Convert signed int n (including INT_MIN) to decimal string.
 *
 * @param  n  Integer to format.
 * @return Caller-owned string, or NULL on allocation failure.
 */
char	*ft_itoa(int n);

/**
 * @brief  Split s by delimiter c into a NULL-terminated string array.
 *
 * @param  s  Source string (NULL returns NULL).
 * @param  c  Delimiter byte.
 * @return Caller-owned NULL-terminated array (free_str_array), or NULL.
 */
char	**ft_split(char const *s, char c);

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026-11-22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

/**
 * @file ft_split.c
 * @brief Delimiter-based string splitting into a NULL-terminated array.
 */

/**
 * @brief  Count non-empty tokens separated by delimiter `c` in `s`.
 *
 * Pre-counting lets `ft_split` size the outer pointer array once without
 * reallocation.
 *
 * @param  s  Non-NULL input string to scan.
 * @param  c  Delimiter byte.
 * @return Number of non-empty substrings.
 */
static size_t	count_words(char const *s, char c)
{
	size_t	count;
	size_t	i;

	count = 0;
	i = 0;
	while (s[i])
	{
		while (s[i] && s[i] == c)
			i++;
		if (s[i])
			count++;
		while (s[i] && s[i] != c)
			i++;
	}
	return (count);
}

/**
 * @brief  Roll back `n` allocated token strings and `arr` on OOM failure.
 *
 * @param  arr  Partially populated array to free.
 * @param  n    Number of valid entries already stored in `arr`.
 * @return Always `0` for direct propagation of failure status.
 */
static int	free_split(char **arr, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		free(arr[i]);
		i++;
	}
	free(arr);
	return (0);
}

/**
 * @brief  Extract tokens from `s` into `arr` and NULL-terminate `arr`.
 *
 * Frees all previously allocated entries via `free_split` if any
 * `ft_substr` allocation fails so partial results never leak.
 *
 * @param  arr  Pre-allocated array of size `count_words(s, c) + 1`.
 * @param  s    Non-NULL input string.
 * @param  c    Delimiter byte.
 * @return `1` on success, `0` on allocation failure (`arr` freed).
 */
static int	fill_words(char **arr, char const *s, char c)
{
	size_t	i;
	size_t	j;
	size_t	start;

	i = 0;
	j = 0;
	while (s[i])
	{
		while (s[i] && s[i] == c)
			i++;
		start = i;
		while (s[i] && s[i] != c)
			i++;
		if (i > start)
		{
			arr[j] = ft_substr(s, start, i - start);
			if (!arr[j])
				return (free_split(arr, j));
			j++;
		}
	}
	arr[j] = NULL;
	return (1);
}

/**
 * @brief  Split `s` on delimiter `c` into a NULL-terminated string array.
 *
 * Consecutive delimiters are skipped, producing no empty tokens.
 *
 * @param  s  Input string (returns `NULL` if `NULL`).
 * @param  c  Delimiter character.
 * @return Caller-owned NULL-terminated `char **` (must free each string
 *         and the array), or `NULL` on `NULL` input or OOM.
 */
char	**ft_split(char const *s, char c)
{
	char	**arr;

	if (!s)
		return (NULL);
	arr = (char **)malloc(sizeof(char *) * (count_words(s, c) + 1));
	if (!arr)
		return (NULL);
	if (!fill_words(arr, s, c))
		return (NULL);
	return (arr);
}

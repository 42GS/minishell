/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_export_print.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:08:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 00:08:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file builtin_export_print.c
 * @brief ASCII sorting and escaped declaration printing for bare export.
 */

/** @brief Counts nodes in the environment linked list for array sizing. */
static size_t	count_env_nodes(t_env_node *env)
{
	size_t	count;

	count = 0;
	while (env)
	{
		count++;
		env = env->next;
	}
	return (count);
}

/**
 * @brief  Sorts an array of environment node pointers in ASCII key order.
 *
 * Uses lexicographical ft_strcmp() comparison to match POSIX/C locale sort
 * order without mutating the underlying env_list linked list.
 * @param  arr    Array of borrowed t_env_node pointers to sort in place.
 * @param  count  Number of elements in arr.
 */
static void	sort_env_nodes(t_env_node **arr, size_t count)
{
	size_t		i;
	size_t		j;
	t_env_node	*tmp;

	i = 0;
	while (i < count)
	{
		j = i + 1;
		while (j < count)
		{
			if (ft_strcmp(arr[i]->key, arr[j]->key) > 0)
			{
				tmp = arr[i];
				arr[i] = arr[j];
				arr[j] = tmp;
			}
			j++;
		}
		i++;
	}
}

/**
 * @brief  Writes an exported value with shell-metacharacter escaping.
 *
 * Escapes '"', '$', and '\' with a leading backslash so the printed
 * declare -x output remains safely reusable as shell input, checking each
 * write() for I/O errors.
 * @param  val  Variable value string to escape and write to stdout.
 * @return 0 on success, 1 if any write() fails.
 */
static int	print_escaped_value(const char *val)
{
	size_t	i;

	i = 0;
	while (val[i])
	{
		if (val[i] == '"' || val[i] == '$' || val[i] == '\\')
		{
			if (write(STDOUT_FILENO, "\\", 1) < 0)
				return (1);
		}
		if (write(STDOUT_FILENO, &val[i], 1) < 0)
			return (1);
		i++;
	}
	return (0);
}

/**
 * @brief  Formats one environment node as a "declare -x" line.
 *
 * Prints key="escaped_value" when value is non-NULL, or bare key without
 * '=' when value is NULL, propagating any write() error immediately.
 * @param  node  Environment node to format and write to stdout.
 * @return 0 on success, 1 on write() failure.
 */
static int	print_export_node(t_env_node *node)
{
	if (builtin_write_str(STDOUT_FILENO, "declare -x ") != 0)
		return (1);
	if (builtin_write_str(STDOUT_FILENO, node->key) != 0)
		return (1);
	if (node->value)
	{
		if (builtin_write_str(STDOUT_FILENO, "=\"") != 0)
			return (1);
		if (print_escaped_value(node->value) != 0)
			return (1);
		return (builtin_write_endl(STDOUT_FILENO, "\""));
	}
	return (builtin_write_endl(STDOUT_FILENO, ""));
}

/**
 * @brief  Prints all exported variables sorted in ASCII order by key.
 *
 * Allocates a temporary pointer array to sort without altering env_list
 * order, prints each entry, and stops early if any write() fails.
 * @param  env  Head of the shell environment list.
 * @return 0 on success, 1 on malloc or write() failure.
 */
int	print_sorted_export(t_env_node *env)
{
	t_env_node	**arr;
	size_t		count;
	size_t		i;
	int			ret;

	count = count_env_nodes(env);
	if (count == 0)
		return (0);
	arr = (t_env_node **)malloc(sizeof(t_env_node *) * count);
	if (!arr)
		return (1);
	i = 0;
	while (env)
	{
		arr[i++] = env;
		env = env->next;
	}
	sort_env_nodes(arr, count);
	i = 0;
	ret = 0;
	while (i < count && ret == 0)
		ret = print_export_node(arr[i++]);
	free(arr);
	return (ret);
}

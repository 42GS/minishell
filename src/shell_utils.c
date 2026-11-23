/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shell_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:40:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:40:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file shell_utils.c
 * @brief Resource teardown for parent/child shells and raw stdin line reader.
 */

/**
 * @brief  Free a NULL-terminated array of heap strings and the array itself.
 * @param  arr  String vector to release (no-op if NULL).
 */
void	free_str_array(char **arr)
{
	size_t	i;

	if (!arr)
		return ;
	i = 0;
	while (arr[i])
	{
		free(arr[i]);
		i++;
	}
	free(arr);
}

/**
 * @brief  Release all heap buffers and backup FDs owned by @p ctx.
 * @param  ctx  Shell context (safe if NULL).
 *
 * Closes `saved_in` / `saved_out` when `has_backup_fds` is set, and frees
 * `line`, `current_ast`, `current_tokens`, `env_list`, and `cwd` so child
 * processes and parent shutdown leave zero reachable leaks or open FDs.
 */
void	free_child_resources(t_shell *ctx)
{
	if (!ctx)
		return ;
	if (ctx->has_backup_fds)
	{
		if (ctx->saved_in >= 0)
			close(ctx->saved_in);
		if (ctx->saved_out >= 0)
			close(ctx->saved_out);
		ctx->has_backup_fds = false;
	}
	free(ctx->line);
	ctx->line = NULL;
	free_ast(&ctx->current_ast);
	free_tokens(&ctx->current_tokens);
	free_env_list(&ctx->env_list);
	if (ctx->cwd)
	{
		free(ctx->cwd);
		ctx->cwd = NULL;
	}
}

/**
 * @brief  Free execve temporaries and shell resources, then exit in a child.
 * @param  ctx   Shell context freed via `free_child_resources`.
 * @param  path  Resolved executable path to free (may be NULL).
 * @param  envp  Serialized environment vector to free (may be NULL).
 * @param  code  Exit status passed to `exit(3)`.
 */
void	exit_child_clean(t_shell *ctx, char *path, char **envp, int code)
{
	free(path);
	free_str_array(envp);
	free_child_resources(ctx);
	exit(code);
}

/**
 * @brief  Read one newline-terminated line from `STDIN_FILENO` byte-by-byte.
 * @return Heap-allocated line without trailing newline, or NULL on EOF/OOM.
 *
 * Used in non-interactive mode instead of `readline` so piped or scripted
 * inputs avoid prompt output and terminal line-editing overhead.
 */
char	*read_stdin_line(void)
{
	t_strbuf	sb;
	char		c;
	ssize_t		rd;

	ft_bzero(&sb, sizeof(t_strbuf));
	while (1)
	{
		rd = read(STDIN_FILENO, &c, 1);
		if (rd <= 0)
			break ;
		if (c == '\n')
		{
			if (!sb_reserve(&sb, 0))
				return (NULL);
			return (sb.data);
		}
		if (!sb_push_char(&sb, c))
			return (NULL);
	}
	return (sb.data);
}

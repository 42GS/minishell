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

void	exit_child_clean(t_shell *ctx, char *path, char **envp, int code)
{
	free(path);
	free_str_array(envp);
	free_child_resources(ctx);
	exit(code);
}

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

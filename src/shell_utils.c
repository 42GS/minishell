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
	free_ast(&ctx->current_ast);
	free_tokens(&ctx->current_tokens);
	free_env_list(&ctx->env_list);
	if (ctx->cwd)
	{
		free(ctx->cwd);
		ctx->cwd = NULL;
	}
}

static char	*append_char_line(char *line, char c, size_t len)
{
	char	*new_line;
	size_t	i;

	new_line = (char *)malloc(sizeof(char) * (len + 2));
	if (!new_line)
	{
		free(line);
		return (NULL);
	}
	i = 0;
	while (i < len)
	{
		new_line[i] = line[i];
		i++;
	}
	new_line[len] = c;
	new_line[len + 1] = '\0';
	free(line);
	return (new_line);
}

char	*read_stdin_line(void)
{
	char	*line;
	char	c;
	ssize_t	rd;
	size_t	len;

	line = NULL;
	len = 0;
	while (1)
	{
		rd = read(STDIN_FILENO, &c, 1);
		if (rd <= 0)
			break ;
		if (c == '\n')
		{
			if (!line)
				return (ft_strdup(""));
			return (line);
		}
		line = append_char_line(line, c, len++);
		if (!line)
			return (NULL);
	}
	return (line);
}

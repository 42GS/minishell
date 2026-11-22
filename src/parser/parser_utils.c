/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:58:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:58:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_redir_token(t_token_type type)
{
	return (type == TOK_REDIR_IN || type == TOK_REDIR_OUT
		|| type == TOK_REDIR_APPEND || type == TOK_HEREDOC);
}

t_redir_type	get_redir_type(t_token_type type)
{
	if (type == TOK_REDIR_IN)
		return (REDIR_IN);
	if (type == TOK_REDIR_OUT)
		return (REDIR_OUT);
	if (type == TOK_REDIR_APPEND)
		return (REDIR_APPEND);
	return (REDIR_HEREDOC);
}

void	*parser_syntax_error(t_token *tok, int *err)
{
	const char	*tok_str;

	if (!tok || tok->type == TOK_EOF)
		tok_str = "newline";
	else
		tok_str = tok->value;
	ft_putstr_fd("minishell: syntax error near unexpected token `",
		STDERR_FILENO);
	ft_putstr_fd((char *)tok_str, STDERR_FILENO);
	ft_putendl_fd("'", STDERR_FILENO);
	if (err)
		*err = 2;
	return (NULL);
}

static size_t	arr_len(char **arr)
{
	size_t	len;

	len = 0;
	while (arr && arr[len])
		len++;
	return (len);
}

int	append_str_to_argv(char ***argv, const char *str)
{
	char	**new_arr;
	size_t	len;
	size_t	i;

	len = arr_len(*argv);
	new_arr = (char **)malloc(sizeof(char *) * (len + 2));
	if (!new_arr)
		return (0);
	i = 0;
	while (i < len)
	{
		new_arr[i] = (*argv)[i];
		i++;
	}
	new_arr[len] = ft_strdup(str);
	if (!new_arr[len])
	{
		free(new_arr);
		return (0);
	}
	new_arr[len + 1] = NULL;
	free(*argv);
	*argv = new_arr;
	return (1);
}

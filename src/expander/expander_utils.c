/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:48:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:48:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	append_char_str(char **res, char c)
{
	char	buf[2];
	char	*joined;

	buf[0] = c;
	buf[1] = '\0';
	joined = ft_strjoin(*res, buf);
	free(*res);
	*res = joined;
	return (joined != NULL);
}

int	append_str_free(char **res, char *chunk)
{
	char	*joined;

	if (!chunk)
	{
		free(*res);
		*res = NULL;
		return (0);
	}
	joined = ft_strjoin(*res, chunk);
	free(*res);
	free(chunk);
	*res = joined;
	return (joined != NULL);
}

char	*strip_quotes_only(const char *raw)
{
	t_quote_state	st;
	size_t			i;
	char			*res;

	res = ft_strdup("");
	st = STATE_GENERAL;
	i = 0;
	while (res && raw && raw[i])
	{
		if (st == STATE_GENERAL && raw[i] == '\'')
			st = STATE_SQUOTE;
		else if (st == STATE_SQUOTE && raw[i] == '\'')
			st = STATE_GENERAL;
		else if (st == STATE_GENERAL && raw[i] == '"')
			st = STATE_DQUOTE;
		else if (st == STATE_DQUOTE && raw[i] == '"')
			st = STATE_GENERAL;
		else if (!append_char_str(&res, raw[i]))
			return (NULL);
		i++;
	}
	return (res);
}

static int	expand_one_arg(char ***new_argv, const char *raw, t_shell *ctx)
{
	int		had_q;
	size_t	i;
	char	*exp;
	int		ok;

	had_q = 0;
	i = 0;
	while (raw[i])
	{
		if (raw[i] == '\'' || raw[i] == '"')
			had_q = 1;
		i++;
	}
	exp = expand_word(raw, ctx);
	if (!exp)
		return (0);
	ok = 1;
	if (had_q || exp[0] != '\0')
		ok = append_str_to_argv(new_argv, exp);
	free(exp);
	return (ok);
}

int	expand_cmd_args(t_ast_node *cmd, t_shell *ctx)
{
	char	**new_argv;
	int		i;

	if (!cmd || cmd->is_expanded)
		return (1);
	cmd->is_expanded = true;
	if (!cmd->argv)
		return (1);
	new_argv = NULL;
	i = 0;
	while (cmd->argv[i])
	{
		if (!expand_one_arg(&new_argv, cmd->argv[i], ctx))
		{
			free_str_array(new_argv);
			return (0);
		}
		i++;
	}
	free_str_array(cmd->argv);
	cmd->argv = new_argv;
	return (1);
}

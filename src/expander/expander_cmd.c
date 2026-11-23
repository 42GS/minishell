/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_cmd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 13:55:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 13:55:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static bool	is_export_assign_arg(const char *raw)
{
	size_t	i;

	if (!raw || (!ft_isalpha((unsigned char)raw[0]) && raw[0] != '_'))
		return (false);
	i = 1;
	while (raw[i] && (ft_isalnum((unsigned char)raw[i]) || raw[i] == '_'))
		i++;
	if (raw[i] == '+' && raw[i + 1] == '=')
		return (true);
	return (raw[i] == '=');
}

static int	append_fields_to_argv(char ***dst, char **fields)
{
	int	i;

	if (!fields)
		return (1);
	i = 0;
	while (fields[i])
	{
		if (!append_str_to_argv(dst, fields[i]))
		{
			free_str_array(fields);
			return (0);
		}
		i++;
	}
	free_str_array(fields);
	return (1);
}

static int	expand_one_cmd_arg(char ***new_argv, const char *raw, t_shell *ctx)
{
	char	*single;
	char	**fields;
	int		err;

	if (*new_argv && (*new_argv)[0]
		&& ft_strcmp((*new_argv)[0], "export") == 0
		&& is_export_assign_arg(raw))
	{
		single = expand_word(raw, ctx);
		if (!single)
			return (0);
		err = !append_str_to_argv(new_argv, single);
		free(single);
		return (!err);
	}
	fields = expand_word_fields(raw, ctx, &err);
	if (err)
		return (0);
	return (append_fields_to_argv(new_argv, fields));
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
		if (!expand_one_cmd_arg(&new_argv, cmd->argv[i], ctx))
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

char	*expand_redir_target(const char *raw, t_shell *ctx)
{
	char	**fields;
	char	*target;
	int		err;

	fields = expand_word_fields(raw, ctx, &err);
	if (err)
		return (NULL);
	if (!fields || !fields[0] || fields[1])
	{
		print_exec_error(raw, "ambiguous redirect");
		free_str_array(fields);
		return (NULL);
	}
	target = ft_strdup(fields[0]);
	free_str_array(fields);
	return (target);
}

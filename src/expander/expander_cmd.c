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

/**
 * @file expander_cmd.c
 * @brief Command argv and redirection target expansion with Bash semantics.
 */

/**
 * @brief  Check if a raw word has NAME=VALUE or NAME+=VALUE export syntax.
 *
 * Checked on the unexpanded raw argument so quotes on the left-hand side
 * (e.g., "A=1") fail the identifier check and undergo normal splitting.
 *
 * @param  raw  Unexpanded command argument.
 * @return true if @p raw begins with a valid identifier followed by = or +=.
 */
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

/**
 * @brief  Append expanded @p fields to @p *dst and always free @p fields.
 * @param  dst     Destination NULL-terminated argv pointer.
 * @param  fields  Heap array of expanded fields (consumed and freed).
 * @return 1 on success, 0 on allocation failure.
 */
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

/**
 * @brief  Expand one raw command argument into @p *new_argv.
 *
 * Enforces Bash's export assignment exception: once argv[0] is "export",
 * arguments matching NAME=VALUE or NAME+=VALUE are expanded via
 * expand_word() to suppress IFS word splitting and pathname globbing.
 *
 * @param  new_argv  In/out expanded argv vector being built.
 * @param  raw       Raw argument string from the AST.
 * @param  ctx       Shell context.
 * @return 1 on success, 0 on allocation failure.
 */
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

/**
 * @brief  Expand all argv words of a command AST node in place.
 *
 * Guards with cmd->is_expanded so re-visiting a node never double-expands
 * previously substituted literals or wildcards.
 *
 * @param  cmd  Command AST node whose argv is replaced on success.
 * @param  ctx  Shell context.
 * @return 1 on success, 0 on allocation failure (original argv preserved).
 */
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

/**
 * @brief  Expand a redirection target and detect ambiguous redirects.
 *
 * POSIX requires redirection targets to expand to exactly one field; zero
 * fields (empty unquoted var) or multiple fields (IFS split or multi-match
 * glob) trigger an "ambiguous redirect" diagnostic and return NULL.
 *
 * @param  raw  Raw redirection filename word.
 * @param  ctx  Shell context.
 * @return Caller-owned target path, or NULL on OOM or ambiguous redirect.
 */
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

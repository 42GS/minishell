/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_cmd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:58:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:58:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_ast_node	*parse_tokens(t_token *tokens, int *err_code)
{
	t_token		*curr;
	t_ast_node	*root;

	if (err_code)
		*err_code = 0;
	if (!tokens || tokens->type == TOK_EOF)
		return (NULL);
	curr = tokens;
	root = parse_and_or(&curr, err_code);
	if (!root)
		return (NULL);
	if (curr && curr->type != TOK_EOF)
	{
		free_ast(&root);
		return (parser_syntax_error(curr, err_code));
	}
	return (root);
}

static int	handle_cmd_redir(t_token **curr, t_ast_node *node, int *err)
{
	t_token_type	op_type;
	t_redir			*redir;

	op_type = (*curr)->type;
	*curr = (*curr)->next;
	if (!*curr || (*curr)->type != TOK_WORD)
	{
		parser_syntax_error(*curr, err);
		return (0);
	}
	redir = create_redir_node(op_type, *curr);
	if (!redir)
	{
		if (err)
			*err = 1;
		return (0);
	}
	append_redir(&node->redirs, redir);
	*curr = (*curr)->next;
	return (1);
}

static int	handle_cmd_word(t_token **curr, t_ast_node *node, int *err)
{
	if (!append_str_to_argv(&node->argv, (*curr)->value))
	{
		if (err)
			*err = 1;
		return (0);
	}
	*curr = (*curr)->next;
	return (1);
}

t_ast_node	*parse_simple_command(t_token **curr, int *err)
{
	t_ast_node	*node;
	int			ok;

	node = create_ast_node(NODE_CMD, NULL, NULL);
	if (!node)
	{
		if (err)
			*err = 1;
		return (NULL);
	}
	while (*curr && ((*curr)->type == TOK_WORD
			|| is_redir_token((*curr)->type)))
	{
		if ((*curr)->type == TOK_WORD)
			ok = handle_cmd_word(curr, node, err);
		else
			ok = handle_cmd_redir(curr, node, err);
		if (!ok)
		{
			free_ast(&node);
			return (NULL);
		}
	}
	return (node);
}

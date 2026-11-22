/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:58:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:58:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	parse_redir_list(t_token **curr, t_redir **redirs, int *err)
{
	t_token_type	op_type;
	t_redir			*node;

	while (*curr && is_redir_token((*curr)->type))
	{
		op_type = (*curr)->type;
		*curr = (*curr)->next;
		if (!*curr || (*curr)->type != TOK_WORD)
		{
			parser_syntax_error(*curr, err);
			return (0);
		}
		node = create_redir_node(op_type, *curr);
		if (!node)
		{
			*err = 1;
			return (0);
		}
		append_redir(redirs, node);
		*curr = (*curr)->next;
	}
	return (1);
}

static t_ast_node	*parse_subshell(t_token **curr, int *err)
{
	t_ast_node	*sub;
	t_ast_node	*node;

	*curr = (*curr)->next;
	sub = parse_and_or(curr, err);
	if (!sub)
		return (NULL);
	if (!*curr || (*curr)->type != TOK_RPAREN)
	{
		free_ast(&sub);
		return (parser_syntax_error(*curr, err));
	}
	*curr = (*curr)->next;
	node = join_ast_nodes(NODE_SUBSHELL, NULL, sub, err);
	if (!node)
		return (NULL);
	node->left = node->right;
	node->right = NULL;
	if (!parse_redir_list(curr, &node->redirs, err))
	{
		free_ast(&node);
		return (NULL);
	}
	return (node);
}

static t_ast_node	*parse_command(t_token **curr, int *err)
{
	if (!*curr)
		return (parser_syntax_error(NULL, err));
	if ((*curr)->type == TOK_LPAREN)
		return (parse_subshell(curr, err));
	if ((*curr)->type == TOK_WORD || is_redir_token((*curr)->type))
		return (parse_simple_command(curr, err));
	return (parser_syntax_error(*curr, err));
}

static t_ast_node	*parse_pipeline(t_token **curr, int *err)
{
	t_ast_node	*left;
	t_ast_node	*right;

	left = parse_command(curr, err);
	if (!left)
		return (NULL);
	while (*curr && (*curr)->type == TOK_PIPE)
	{
		*curr = (*curr)->next;
		right = parse_command(curr, err);
		left = join_ast_nodes(NODE_PIPE, left, right, err);
		if (!left)
			return (NULL);
	}
	return (left);
}

t_ast_node	*parse_and_or(t_token **curr, int *err)
{
	t_ast_node	*left;
	t_ast_node	*right;
	t_node_type	type;

	left = parse_pipeline(curr, err);
	if (!left)
		return (NULL);
	while (*curr && ((*curr)->type == TOK_AND_IF
			|| (*curr)->type == TOK_OR_IF))
	{
		type = NODE_AND;
		if ((*curr)->type == TOK_OR_IF)
			type = NODE_OR;
		*curr = (*curr)->next;
		right = parse_pipeline(curr, err);
		left = join_ast_nodes(type, left, right, err);
		if (!left)
			return (NULL);
	}
	return (left);
}

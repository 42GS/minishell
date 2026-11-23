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

/**
 * @file parser.c
 * @brief Recursive-descent parser enforcing shell operator precedence.
 */

/**
 * @brief  Parse zero or more trailing redirections into @p redirs.
 * @param  curr    In/out token cursor advanced past parsed redirections.
 * @param  redirs  Destination redirection list head.
 * @param  err     Set to 2 on syntax error or 1 on allocation failure.
 * @return 1 on success, 0 on error.
 */
static int	parse_redir_list(t_token **curr, t_redir **redirs, int *err)
{
	while (*curr && is_redir_token((*curr)->type))
	{
		if (!parse_single_redir(curr, redirs, err))
			return (0);
	}
	return (1);
}

/**
 * @brief  Parse a parenthesized subshell `( list ) [redirs]`.
 * @param  curr  Token cursor initially pointing to `TOK_LPAREN`.
 * @param  err   Set to 2 on missing `)` / bad redir or 1 on OOM.
 * @return `NODE_SUBSHELL` AST node, or NULL on error (partial AST freed).
 */
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

/**
 * @brief  Parse a primary command unit: subshell or simple command.
 * @param  curr  Token cursor at the start of the command unit.
 * @param  err   Set to 2 on unexpected token (`TOK_AMP`, `)`, `|`, EOF).
 * @return Parsed AST node, or NULL on syntax/allocation error.
 */
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

/**
 * @brief  Parse a left-associative pipeline (`cmd ( '|' cmd )*`).
 * @param  curr  In/out token cursor.
 * @param  err   Propagates syntax error 2 or allocation error 1.
 * @return Root of the pipeline subtree, or NULL on error (no leaks).
 */
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

/**
 * @brief  Parse left-associative `&&` / `||` lists (lowest precedence).
 * @param  curr  In/out token cursor.
 * @param  err   Propagates syntax error 2 or allocation error 1.
 * @return AST root of the parsed list, or NULL on error.
 *
 * Enforces shell precedence: `(...)` and simple commands with redirections
 * bind tighter than `|`, which binds tighter than `&&` and `||`.
 */
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

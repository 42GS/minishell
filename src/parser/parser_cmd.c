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

/**
 * @file parser_cmd.c
 * @brief Top-level parse entry point and simple command/redir builders.
 */

/**
 * @brief  Parse a `TOK_EOF`-terminated token list into an AST.
 * @param  tokens    Head of token stream from `tokenize_input`.
 * @param  err_code  Out status: 0 on success/empty, 2 on syntax, 1 on OOM.
 * @return Root AST node, or NULL on empty input or error (AST freed).
 *
 * Rejects trailing unconsumed tokens (e.g. an unmatched `)`) with syntax
 * error 2 instead of silently ignoring the remainder of the line.
 */
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

/**
 * @brief  Parse one redirection operator and its mandatory word target.
 * @param  curr    In/out token cursor starting on a redirection operator.
 * @param  redirs  Destination redirection list tail-appended on success.
 * @param  err     Set to 2 if target is missing/non-word, or 1 on OOM.
 * @return 1 on success, 0 on syntax or allocation error.
 */
int	parse_single_redir(t_token **curr, t_redir **redirs, int *err)
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
	append_redir(redirs, redir);
	*curr = (*curr)->next;
	return (1);
}

/**
 * @brief  Append one `TOK_WORD` string to @p node->argv and advance @p curr.
 * @param  curr  In/out token cursor pointing at a `TOK_WORD`.
 * @param  node  `NODE_CMD` accumulating command arguments.
 * @param  err   Set to 1 on allocation failure.
 * @return 1 on success, 0 on OOM.
 */
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

/**
 * @brief  Parse interleaved command words and redirections into `NODE_CMD`.
 * @param  curr  In/out token cursor starting on a word or redirection.
 * @param  err   Set to 2 on redirection syntax error or 1 on OOM.
 * @return Allocated `NODE_CMD`, or NULL with partial node freed on error.
 */
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
			ok = parse_single_redir(curr, &node->redirs, err);
		if (!ok)
		{
			free_ast(&node);
			return (NULL);
		}
	}
	return (node);
}

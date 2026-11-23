/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_ast.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:58:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:58:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file parser_ast.c
 * @brief AST and redirection node constructors and recursive cleanup.
 */

/**
 * @brief  Zero-allocate an AST node so child/argv/redir pointers start NULL.
 * @param  type   Node kind (`NODE_CMD`, `NODE_PIPE`, `NODE_AND`, etc.).
 * @param  left   Left subtree (or inner subtree for `NODE_SUBSHELL`).
 * @param  right  Right subtree for binary operators, or NULL.
 * @return Allocated AST node, or NULL on allocation failure.
 */
t_ast_node	*create_ast_node(t_node_type type, t_ast_node *left,
	t_ast_node *right)
{
	t_ast_node	*node;

	node = (t_ast_node *)ft_calloc(1, sizeof(t_ast_node));
	if (!node)
		return (NULL);
	node->type = type;
	node->left = left;
	node->right = right;
	return (node);
}

/**
 * @brief  Combine two subtrees under a new @p type parent, freeing on error.
 * @param  type   Parent operator kind (`NODE_PIPE`, `NODE_AND`, etc.).
 * @param  left   Left subtree (ownership transferred).
 * @param  right  Right subtree (ownership transferred; may be NULL on error).
 * @param  err    Set to 1 if parent allocation fails (not set if !right).
 * @return New parent node, or NULL with both @p left and @p right freed.
 */
t_ast_node	*join_ast_nodes(t_node_type type, t_ast_node *left,
	t_ast_node *right, int *err)
{
	t_ast_node	*parent;

	if (!right)
	{
		free_ast(&left);
		return (NULL);
	}
	parent = create_ast_node(type, left, right);
	if (!parent)
	{
		free_ast(&left);
		free_ast(&right);
		if (err)
			*err = 1;
		return (NULL);
	}
	return (parent);
}

/**
 * @brief  Allocate a redirection node from an operator and target token.
 * @param  tok_type    Lexer redirection token (`TOK_REDIR_*` or heredoc).
 * @param  target_tok  `TOK_WORD` whose value and quote mask are copied.
 * @return Allocated `t_redir` with `heredoc_fd = -1`, or NULL on OOM.
 *
 * Sets `quoted_delim` whenever `target_tok->quoted != QUOTE_NONE` so heredoc
 * processing skips `$VAR` expansion for quoted delimiters.
 */
t_redir	*create_redir_node(t_token_type tok_type, t_token *target_tok)
{
	t_redir	*redir;

	redir = (t_redir *)malloc(sizeof(t_redir));
	if (!redir)
		return (NULL);
	redir->type = get_redir_type(tok_type);
	redir->target = ft_strdup(target_tok->value);
	if (!redir->target)
	{
		free(redir);
		return (NULL);
	}
	redir->heredoc_fd = -1;
	redir->quoted_delim = (target_tok->quoted != QUOTE_NONE);
	redir->next = NULL;
	return (redir);
}

/**
 * @brief  Append @p new_redir to the tail of @p head to keep order.
 * @param  head       Address of the redirection list head.
 * @param  new_redir  Redirection node appended at the end.
 */
void	append_redir(t_redir **head, t_redir *new_redir)
{
	t_redir	*curr;

	if (!head || !new_redir)
		return ;
	if (!*head)
	{
		*head = new_redir;
		return ;
	}
	curr = *head;
	while (curr->next)
		curr = curr->next;
	curr->next = new_redir;
}

/**
 * @brief  Recursively free an AST, its redirections, and open heredoc FDs.
 * @param  node  Address of AST root pointer; nulled after teardown.
 *
 * Closes any `heredoc_fd >= 0` left open on syntax/abort paths so failed or
 * unexecuted pipelines never leak pipe read descriptors.
 */
void	free_ast(t_ast_node **node)
{
	t_redir	*redir;
	t_redir	*next;

	if (!node || !*node)
		return ;
	free_ast(&(*node)->left);
	free_ast(&(*node)->right);
	free_str_array((*node)->argv);
	redir = (*node)->redirs;
	while (redir)
	{
		next = redir->next;
		free(redir->target);
		if (redir->heredoc_fd >= 0)
			close(redir->heredoc_fd);
		free(redir);
		redir = next;
	}
	free(*node);
	*node = NULL;
}

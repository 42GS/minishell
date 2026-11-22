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

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_stub.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_ast_node	*parse_tokens(t_token *tokens, int *err_code)
{
	(void)tokens;
	if (err_code)
		*err_code = 0;
	return (NULL);
}

static void	free_redirs(t_redir *redir)
{
	t_redir	*next;

	while (redir)
	{
		next = redir->next;
		free(redir->target);
		if (redir->heredoc_fd >= 0)
			close(redir->heredoc_fd);
		free(redir);
		redir = next;
	}
}

void	free_ast(t_ast_node **node)
{
	if (!node || !*node)
		return ;
	free_ast(&(*node)->left);
	free_ast(&(*node)->right);
	free_str_array((*node)->argv);
	free_redirs((*node)->redirs);
	free(*node);
	*node = NULL;
}

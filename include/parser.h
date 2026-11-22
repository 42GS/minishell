/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:58:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:58:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_H
# define PARSER_H

# include "lexer.h"

typedef enum e_node_type
{
	NODE_CMD,
	NODE_PIPE,
	NODE_AND,
	NODE_OR,
	NODE_SUBSHELL
}	t_node_type;

typedef enum e_redir_type
{
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	REDIR_HEREDOC
}	t_redir_type;

typedef struct s_redir
{
	t_redir_type	type;
	char			*target;
	int				heredoc_fd;
	bool			quoted_delim;
	struct s_redir	*next;
}	t_redir;

typedef struct s_ast_node
{
	t_node_type			type;
	struct s_ast_node	*left;
	struct s_ast_node	*right;
	char				**argv;
	t_redir				*redirs;
}	t_ast_node;

t_ast_node		*parse_tokens(t_token *tokens, int *err_code);
void			free_ast(t_ast_node **node);

t_ast_node		*parse_and_or(t_token **curr, int *err);
t_ast_node		*parse_simple_command(t_token **curr, int *err);
t_ast_node		*create_ast_node(t_node_type type, t_ast_node *left,
					t_ast_node *right);
t_ast_node		*join_ast_nodes(t_node_type type, t_ast_node *left,
					t_ast_node *right, int *err);
t_redir			*create_redir_node(t_token_type tok_type, t_token *target_tok);
void			append_redir(t_redir **head, t_redir *new_redir);
int				is_redir_token(t_token_type type);
t_redir_type	get_redir_type(t_token_type type);
void			*parser_syntax_error(t_token *tok, int *err);
int				append_str_to_argv(char ***argv, const char *str);

#endif

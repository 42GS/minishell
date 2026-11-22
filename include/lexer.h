/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:50:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:50:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_H
# define LEXER_H

# include <stdbool.h>
# include <stddef.h>

# define QUOTE_NONE		0
# define QUOTE_SINGLE	1
# define QUOTE_DOUBLE	2

typedef enum e_token_type
{
	TOK_WORD,
	TOK_PIPE,
	TOK_REDIR_IN,
	TOK_REDIR_OUT,
	TOK_REDIR_APPEND,
	TOK_HEREDOC,
	TOK_AND_IF,
	TOK_OR_IF,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_EOF
}	t_token_type;

typedef enum e_quote_state
{
	STATE_GENERAL,
	STATE_SQUOTE,
	STATE_DQUOTE
}	t_quote_state;

typedef struct s_token
{
	t_token_type	type;
	char			*value;
	int				quoted;
	struct s_token	*prev;
	struct s_token	*next;
}	t_token;

t_token	*tokenize_input(const char *input, int *err_code);
void	free_tokens(t_token **tokens);

int		is_lexer_space(char c);
int		is_operator_start(const char *s);
int		extract_word_token(const char *in, size_t *i, t_token **head);
int		extract_op_token(const char *in, size_t *i, t_token **head);
t_token	*create_token(t_token_type type, char *value, int quoted);
void	append_token(t_token **head, t_token *new_tok);
void	*lexer_syntax_error(const char *msg, t_token **head, int *err_code);

#endif

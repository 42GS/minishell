/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_H
# define LEXER_H

# include <stdbool.h>

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
	t_token_type		type;
	char			*value;
	struct s_token	*prev;
	struct s_token	*next;
}	t_token;

t_token	*tokenize_input(const char *input, int *err_code);
void	free_tokens(t_token **tokens);

#endif

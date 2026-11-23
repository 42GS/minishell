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

/**
 * @file lexer.h
 * @brief Lexical analysis, quote state machine, and token list definitions.
 */

/**
 * @brief Bitmask flags recording which quote styles appeared in a word.
 *
 * Used by the parser to detect quoted heredoc delimiters without stripping
 * raw quotes prematurely before expansion.
 */
# define QUOTE_NONE		0
# define QUOTE_SINGLE	1
# define QUOTE_DOUBLE	2

/**
 * @enum  e_token_type
 * @brief Terminal token classes emitted by tokenize_input().
 *
 * TOK_AMP is lexed as a distinct operator so stray '&' triggers a syntax
 * error instead of merging into an adjacent word. Every non-error stream
 * ends with TOK_EOF.
 */
typedef enum e_token_type
{
	TOK_WORD,
	TOK_PIPE,
	TOK_REDIR_IN,
	TOK_REDIR_OUT,
	TOK_REDIR_APPEND,
	TOK_HEREDOC,
	TOK_AND_IF,
	TOK_AMP,
	TOK_OR_IF,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_EOF
}	t_token_type;

/**
 * @enum  e_quote_state
 * @brief Quote-nesting FSM shared by the lexer and variable expander.
 */
typedef enum e_quote_state
{
	STATE_GENERAL,
	STATE_SQUOTE,
	STATE_DQUOTE
}	t_quote_state;

/**
 * @struct s_token
 * @brief  Doubly-linked lexical token node owned by the token list.
 *
 * Key fields:
 * - type: Classification from e_token_type.
 * - value: Heap-allocated raw slice (NULL for TOK_EOF), freed by free_tokens.
 * - quoted: Bitwise OR of QUOTE_* flags observed while scanning the word.
 * - prev, next: Doubly-linked list links.
 */
typedef struct s_token
{
	t_token_type	type;
	char			*value;
	int				quoted;
	struct s_token	*prev;
	struct s_token	*next;
}	t_token;

/**
 * @brief  Lex raw command input into a TOK_EOF-terminated token list.
 *
 * Keeps raw quotes intact in TOK_WORD values so expansion can distinguish
 * literal vs expandable spans later.
 *
 * @param  input     Raw input string (NULL yields a single TOK_EOF list).
 * @param  err_code  Optional out status: 0 on success, 2 on unclosed quote,
 *                   1 on allocation failure.
 * @return Caller-owned token list (free via free_tokens), or NULL on error.
 */
t_token	*tokenize_input(const char *input, int *err_code);

/**
 * @brief  Free an entire token list and its owned strings, nulling *tokens.
 *
 * @param  tokens  Address of list head pointer (safe no-op if NULL).
 */
void	free_tokens(t_token **tokens);

/** @brief Return non-zero if c is shell whitespace (' ' or '\t'..'\r'). */
int		is_lexer_space(char c);

/**
 * @brief  Advance quote FSM when c opens or closes an active quote.
 *
 * @param  c   Current character being scanned.
 * @param  st  In/out quote state updated on quote boundaries.
 * @return 1 if c toggled the quote state, 0 if c is literal content.
 */
int		step_quote(char c, t_quote_state *st);

/** @brief Return 1 if *s starts a shell operator (|, <, >, (, ), &). */
int		is_operator_start(const char *s);

/**
 * @brief  Scan a word token preserving embedded quotes and append to head.
 *
 * @param  in    Source input line.
 * @param  i     In/out scan cursor advanced past the scanned word.
 * @param  head  Token list head receiving the new TOK_WORD node.
 * @return 1 on success, -1 on unclosed quote, 0 on allocation failure.
 */
int		extract_word_token(const char *in, size_t *i, t_token **head);

/**
 * @brief  Scan a 2-char or 1-char operator at *i and append to head.
 *
 * @param  in    Source input line.
 * @param  i     In/out scan cursor advanced by operator length.
 * @param  head  Token list head receiving the new operator node.
 * @return 1 on success, 0 on unrecognized operator or allocation failure.
 */
int		extract_op_token(const char *in, size_t *i, t_token **head);

/**
 * @brief  Allocate a token node taking ownership of value.
 *
 * On allocation failure, value is NOT freed by create_token(); caller must
 * release value on NULL return.
 *
 * @param  type    Token classification.
 * @param  value   Heap-allocated lexeme owned by the token (NULL for EOF).
 * @param  quoted  QUOTE_* bitmask for word tokens.
 * @return Newly allocated token node, or NULL on malloc failure.
 */
t_token	*create_token(t_token_type type, char *value, int quoted);

/**
 * @brief  Append new_tok to the tail of doubly-linked list *head.
 *
 * @param  head     Address of list head pointer.
 * @param  new_tok  Detached token node to append.
 */
void	append_token(t_token **head, t_token *new_tok);

/**
 * @brief  Print a lexer syntax error, free *head, and set *err_code to 2.
 *
 * @param  msg       Error text written to STDERR_FILENO.
 * @param  head      Token list freed and nulled before returning.
 * @param  err_code  Optional output error code set to 2.
 * @return Always returns NULL for concise error propagation.
 */
void	*lexer_syntax_error(const char *msg, t_token **head, int *err_code);

#endif

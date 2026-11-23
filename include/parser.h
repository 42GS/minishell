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

/**
 * @file parser.h
 * @brief Recursive-descent parser and Abstract Syntax Tree definitions.
 */

/**
 * @enum  e_node_type
 * @brief AST node kinds ordered across precedence levels (cmd/subshell,
 *        pipe, and/or).
 */
typedef enum e_node_type
{
	NODE_CMD,
	NODE_PIPE,
	NODE_AND,
	NODE_OR,
	NODE_SUBSHELL
}	t_node_type;

/**
 * @enum  e_redir_type
 * @brief I/O redirection kinds attached to commands and subshells.
 */
typedef enum e_redir_type
{
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	REDIR_HEREDOC
}	t_redir_type;

/**
 * @struct s_redir
 * @brief  Single redirection descriptor applied in left-to-right order.
 *
 * Key fields:
 * - type: Redirection direction/mode from e_redir_type.
 * - target: Heap-allocated raw filename or heredoc delimiter string.
 * - heredoc_fd: Pre-collected read FD (-1 until collect_heredocs() runs).
 *   Closed once applied or when free_ast() tears down the tree.
 * - quoted_delim: True if the heredoc delimiter contained quotes, disabling
 *   parameter expansion inside the heredoc body.
 * - next: Next redirection in evaluation order.
 */
typedef struct s_redir
{
	t_redir_type	type;
	char			*target;
	int				heredoc_fd;
	bool			quoted_delim;
	struct s_redir	*next;
}	t_redir;

/**
 * @struct s_ast_node
 * @brief  Binary AST node owned by the parsed command tree.
 *
 * Key fields:
 * - type: Node role (NODE_CMD, NODE_PIPE, NODE_AND, NODE_OR, NODE_SUBSHELL).
 * - left, right: Child subtrees (NODE_SUBSHELL stores inner tree in left
 *   and keeps right as NULL).
 * - argv: Owned NULL-terminated raw argument array for NODE_CMD.
 * - redirs: Owned linked list of redirections for NODE_CMD or NODE_SUBSHELL.
 * - is_expanded: Prevents duplicate expansion if a node is revisited.
 */
typedef struct s_ast_node
{
	t_node_type			type;
	struct s_ast_node	*left;
	struct s_ast_node	*right;
	char				**argv;
	t_redir				*redirs;
	bool				is_expanded;
}	t_ast_node;

/**
 * @brief  Parse a TOK_EOF-terminated token list into an AST.
 *
 * Rejects trailing tokens after the root expression as syntax errors.
 *
 * @param  tokens    Token list produced by tokenize_input().
 * @param  err_code  Optional out status: 0 on success/empty, 2 on syntax
 *                   error, 1 on allocation failure.
 * @return Caller-owned root AST (free via free_ast), or NULL if empty/error.
 */
t_ast_node		*parse_tokens(t_token *tokens, int *err_code);

/**
 * @brief  Recursively free an AST, its argv/redirs, and open heredoc FDs.
 *
 * @param  node  Address of AST root pointer (nulled on return; NULL-safe).
 */
void			free_ast(t_ast_node **node);

/**
 * @brief  Parse left-associative && and || chains over pipelines.
 *
 * @param  curr  In/out token cursor advanced past the parsed expression.
 * @param  err   Optional out error code (2 syntax, 1 malloc).
 * @return Subtree root, or NULL on syntax or allocation failure.
 */
t_ast_node		*parse_and_or(t_token **curr, int *err);

/**
 * @brief  Parse interleaved command words and redirections into NODE_CMD.
 *
 * @param  curr  In/out token cursor advanced past the simple command.
 * @param  err   Optional out error code on failure.
 * @return Newly allocated NODE_CMD, or NULL on failure.
 */
t_ast_node		*parse_simple_command(t_token **curr, int *err);

/**
 * @brief  Consume one redirection operator and its TOK_WORD target.
 *
 * @param  curr    In/out token cursor starting on a redirection operator.
 * @param  redirs  Redirection list tail-appended on success.
 * @param  err     Optional out error code (2 missing/invalid target, 1 OOM).
 * @return 1 on success, 0 on syntax or allocation failure.
 */
int				parse_single_redir(t_token **curr, t_redir **redirs, int *err);

/**
 * @brief  Allocate a zero-initialized AST node with the given children.
 *
 * @param  type   AST node type.
 * @param  left   Left child subtree (may be NULL).
 * @param  right  Right child subtree (may be NULL).
 * @return Newly allocated node, or NULL on allocation failure.
 */
t_ast_node		*create_ast_node(t_node_type type, t_ast_node *left,
					t_ast_node *right);

/**
 * @brief  Join left and right subtrees under a binary parent node.
 *
 * Frees left (and right if parent allocation fails) on error so callers do
 * not leak already-parsed left branches during unwinding.
 *
 * @param  type   Parent operator type (e.g. NODE_PIPE, NODE_AND).
 * @param  left   Owned left subtree.
 * @param  right  Owned right subtree (NULL triggers left cleanup).
 * @param  err    Optional out error code set to 1 on allocation failure.
 * @return Parent AST node, or NULL on failure.
 */
t_ast_node		*join_ast_nodes(t_node_type type, t_ast_node *left,
					t_ast_node *right, int *err);

/**
 * @brief  Allocate a redirection node from an operator and target token.
 *
 * @param  tok_type    Redirection operator token type.
 * @param  target_tok  TOK_WORD providing target string and quote flags.
 * @return Caller-owned redirection node, or NULL on malloc failure.
 */
t_redir			*create_redir_node(t_token_type tok_type, t_token *target_tok);

/**
 * @brief  Append new_redir to *head to preserve left-to-right eval order.
 *
 * @param  head       Address of redirection list head pointer.
 * @param  new_redir  Detached redirection node to append.
 */
void			append_redir(t_redir **head, t_redir *new_redir);

/** @brief Return non-zero if type is <, >, >>, or <<. */
int				is_redir_token(t_token_type type);

/** @brief Map a redirection token type to its t_redir_type enum value. */
t_redir_type	get_redir_type(t_token_type type);

/**
 * @brief  Emit unexpected-token syntax error to stderr and set *err to 2.
 *
 * @param  tok  Offending token (NULL or TOK_EOF prints "newline").
 * @param  err  Optional output error code set to 2.
 * @return Always returns NULL for concise parser error returns.
 */
void			*parser_syntax_error(t_token *tok, int *err);

/**
 * @brief  Append a duplicated string to a NULL-terminated argv array.
 *
 * Leaves *argv untouched if allocation fails so partial arrays stay valid.
 *
 * @param  argv  Address of NULL-terminated string vector (may start NULL).
 * @param  str   String duplicated into the new tail slot.
 * @return 1 on success, 0 on allocation failure.
 */
int				append_str_to_argv(char ***argv, const char *str);

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "minishell.h"

volatile sig_atomic_t	g_signal_num = 0;

void	free_str_array(char **arr)
{
	size_t	i;

	if (!arr)
		return ;
	i = 0;
	while (arr[i])
		free(arr[i++]);
	free(arr);
}

static void	test_empty_and_spaces(void)
{
	int		err;
	t_token	*tok;

	tok = tokenize_input("", &err);
	assert(err == 0 && tok != NULL && tok->type == TOK_EOF && tok->next == NULL);
	free_tokens(&tok);
	tok = tokenize_input("   \t\n  ", &err);
	assert(err == 0 && tok != NULL && tok->type == TOK_EOF && tok->next == NULL);
	free_tokens(&tok);
}

static void	test_operators_and_simple_words(void)
{
	int		err;
	t_token	*tok;
	t_token	*cur;

	tok = tokenize_input("cat<in|grep a>>out", &err);
	assert(err == 0 && tok != NULL);
	cur = tok;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "cat") == 0);
	cur = cur->next;
	assert(cur->type == TOK_REDIR_IN && strcmp(cur->value, "<") == 0);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "in") == 0);
	cur = cur->next;
	assert(cur->type == TOK_PIPE && strcmp(cur->value, "|") == 0);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "grep") == 0);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "a") == 0);
	cur = cur->next;
	assert(cur->type == TOK_REDIR_APPEND && strcmp(cur->value, ">>") == 0);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "out") == 0);
	cur = cur->next;
	assert(cur->type == TOK_EOF && cur->next == NULL);
	free_tokens(&tok);
}

static void	test_bonus_ops_and_single_ampersand(void)
{
	int		err;
	t_token	*tok;
	t_token	*cur;

	tok = tokenize_input("(a && b) || echo x&y", &err);
	assert(err == 0 && tok != NULL);
	cur = tok;
	assert(cur->type == TOK_LPAREN);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "a") == 0);
	cur = cur->next;
	assert(cur->type == TOK_AND_IF);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "b") == 0);
	cur = cur->next;
	assert(cur->type == TOK_RPAREN);
	cur = cur->next;
	assert(cur->type == TOK_OR_IF);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "echo") == 0);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "x") == 0);
	cur = cur->next;
	assert(cur->type == TOK_AMP && strcmp(cur->value, "&") == 0);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "y") == 0);
	cur = cur->next;
	assert(cur->type == TOK_EOF);
	free_tokens(&tok);
	tok = tokenize_input("echo a &&& echo b", &err);
	assert(err == 0 && tok != NULL);
	cur = tok->next->next;
	assert(cur->type == TOK_AND_IF && strcmp(cur->value, "&&") == 0);
	cur = cur->next;
	assert(cur->type == TOK_AMP && strcmp(cur->value, "&") == 0);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "echo") == 0);
	free_tokens(&tok);
	tok = tokenize_input("echo 'a&b' \"c&d\"", &err);
	assert(err == 0 && tok != NULL);
	cur = tok->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "'a&b'") == 0);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "\"c&d\"") == 0);
	assert(cur->next->type == TOK_EOF);
	free_tokens(&tok);
}

static void	test_quotes_and_heredoc(void)
{
	int		err;
	t_token	*tok;
	t_token	*cur;

	tok = tokenize_input("echo \"$USER | >\" '' << 'EOF'", &err);
	assert(err == 0 && tok != NULL);
	cur = tok;
	assert(cur->type == TOK_WORD && cur->quoted == QUOTE_NONE);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "\"$USER | >\"") == 0);
	assert(cur->quoted == QUOTE_DOUBLE);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "''") == 0);
	assert(cur->quoted == QUOTE_SINGLE);
	cur = cur->next;
	assert(cur->type == TOK_HEREDOC);
	cur = cur->next;
	assert(cur->type == TOK_WORD && strcmp(cur->value, "'EOF'") == 0);
	assert(cur->quoted == QUOTE_SINGLE);
	free_tokens(&tok);
}

static void	test_unclosed_quotes_leak_free(void)
{
	int		err;
	t_token	*tok;

	tok = tokenize_input("echo \"hello world", &err);
	assert(tok == NULL && err == 2);
	tok = tokenize_input("cat | 'unclosed", &err);
	assert(tok == NULL && err == 2);
}

int	main(void)
{
	printf("[UNIT] Running Lexer DFA unit tests...\n");
	test_empty_and_spaces();
	test_operators_and_simple_words();
	test_bonus_ops_and_single_ampersand();
	test_quotes_and_heredoc();
	test_unclosed_quotes_leak_free();
	printf("[UNIT] All Lexer DFA unit tests PASSED!\n");
	return (0);
}

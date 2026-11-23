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

static t_ast_node	*lex_and_parse(const char *line, int *err)
{
	t_token		*toks;
	t_ast_node	*ast;

	toks = tokenize_input(line, err);
	if (!toks)
		return (NULL);
	ast = parse_tokens(toks, err);
	free_tokens(&toks);
	return (ast);
}

static void	test_empty_and_mixed_simple_cmd(void)
{
	int			err;
	t_ast_node	*ast;

	ast = lex_and_parse("   \t  ", &err);
	assert(ast == NULL && err == 0);
	ast = lex_and_parse("> out.txt echo -n << 'EOF' hello", &err);
	assert(ast != NULL && err == 0 && ast->type == NODE_CMD);
	assert(strcmp(ast->argv[0], "echo") == 0);
	assert(strcmp(ast->argv[1], "-n") == 0);
	assert(strcmp(ast->argv[2], "hello") == 0);
	assert(ast->argv[3] == NULL);
	assert(ast->redirs->type == REDIR_OUT);
	assert(strcmp(ast->redirs->target, "out.txt") == 0);
	assert(ast->redirs->next->type == REDIR_HEREDOC);
	assert(ast->redirs->next->quoted_delim == true);
	free_ast(&ast);
	ast = lex_and_parse("< in.txt > out.txt", &err);
	assert(ast != NULL && err == 0 && ast->type == NODE_CMD);
	assert(ast->argv == NULL && ast->redirs != NULL);
	free_ast(&ast);
}

static void	test_precedence_and_associativity(void)
{
	int			err;
	t_ast_node	*ast;

	ast = lex_and_parse("a | b | c", &err);
	assert(ast != NULL && err == 0 && ast->type == NODE_PIPE);
	assert(ast->left->type == NODE_PIPE);
	assert(strcmp(ast->left->left->argv[0], "a") == 0);
	assert(strcmp(ast->left->right->argv[0], "b") == 0);
	assert(strcmp(ast->right->argv[0], "c") == 0);
	free_ast(&ast);
	ast = lex_and_parse("a && b | c || d", &err);
	assert(ast != NULL && err == 0 && ast->type == NODE_OR);
	assert(ast->left->type == NODE_AND);
	assert(strcmp(ast->left->left->argv[0], "a") == 0);
	assert(ast->left->right->type == NODE_PIPE);
	assert(strcmp(ast->right->argv[0], "d") == 0);
	free_ast(&ast);
}

static void	test_subshell_and_redirs(void)
{
	int			err;
	t_ast_node	*ast;

	ast = lex_and_parse("(ls && pwd) > sub.log", &err);
	assert(ast != NULL && err == 0 && ast->type == NODE_SUBSHELL);
	assert(ast->right == NULL);
	assert(ast->left->type == NODE_AND);
	assert(ast->redirs != NULL && ast->redirs->type == REDIR_OUT);
	assert(strcmp(ast->redirs->target, "sub.log") == 0);
	free_ast(&ast);
}

static void	test_syntax_errors_leak_free(void)
{
	const char	*bad_cases[] = {
		"| ls", "ls | | grep a", "ls &&", "cat <",
		"cat > | ls", "( )", "(ls && pwd", "ls )", "(ls) foo",
		"echo a &&& echo b", "echo a & b", "&", "echo x&y", NULL
	};
	int			i;
	int			err;
	t_ast_node	*ast;

	i = 0;
	while (bad_cases[i])
	{
		ast = lex_and_parse(bad_cases[i], &err);
		assert(ast == NULL && err == 2);
		i++;
	}
	assert(lex_and_parse("echo a &&& echo b", NULL) == NULL);
	assert(lex_and_parse("(ls) >", NULL) == NULL);
	ast = lex_and_parse("echo 'a&b' \"c&d\"", &err);
	assert(ast != NULL && err == 0);
	free_ast(&ast);
}

int	main(void)
{
	printf("[UNIT] Running Recursive Descent Parser unit tests...\n");
	test_empty_and_mixed_simple_cmd();
	test_precedence_and_associativity();
	test_subshell_and_redirs();
	test_syntax_errors_leak_free();
	printf("[UNIT] All Parser & AST unit tests PASSED!\n");
	return (0);
}

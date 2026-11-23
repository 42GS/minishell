#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "minishell.h"

volatile sig_atomic_t	g_signal_num = 0;

static void	test_expander_quotes_and_vars(t_shell *ctx)
{
	char	*out;

	ctx->last_exit_status = 42;
	out = expand_word("\"'$MS_UNIT_V'\"", ctx);
	assert(out != NULL && strcmp(out, "'hello'") == 0);
	free(out);
	out = expand_word("'\"$MS_UNIT_V\"'", ctx);
	assert(out != NULL && strcmp(out, "\"$MS_UNIT_V\"") == 0);
	free(out);
	out = expand_word("code=$?/$NO_SUCH/$", ctx);
	assert(out != NULL && strcmp(out, "code=42//$") == 0);
	free(out);
	out = expand_heredoc_line("\"$MS_UNIT_V\" '$?'", ctx);
	assert(out != NULL && strcmp(out, "\"hello\" '42'") == 0);
	free(out);
}

static void	test_file_redirections(t_shell *ctx)
{
	int		saved_out;
	t_redir	r1;
	t_redir	r2;

	saved_out = dup(STDOUT_FILENO);
	r1.type = REDIR_OUT;
	r1.target = "/tmp/ms_unit_redir1.txt";
	r1.heredoc_fd = -1;
	r1.next = &r2;
	r2.type = REDIR_OUT;
	r2.target = "/tmp/ms_unit_redir2.txt";
	r2.heredoc_fd = -1;
	r2.next = NULL;
	assert(apply_redirections(&r1, ctx) == 0);
	ft_putendl_fd("final_target", STDOUT_FILENO);
	dup2(saved_out, STDOUT_FILENO);
	close(saved_out);
	r1.type = REDIR_IN;
	r1.target = "/tmp/nonexistent_ms_unit_999";
	r1.next = NULL;
	assert(apply_redirections(&r1, ctx) == 1);
	unlink("/tmp/ms_unit_redir1.txt");
	unlink("/tmp/ms_unit_redir2.txt");
}

static void	test_heredoc_collection(t_shell *ctx)
{
	int		pfd[2];
	int		saved_in;
	t_token	*toks;
	char	buf[64];
	ssize_t	n;
	int		err;

	toks = tokenize_input("cat << EOF", &err);
	ctx->current_ast = parse_tokens(toks, &err);
	free_tokens(&toks);
	assert(ctx->current_ast != NULL);
	assert(pipe(pfd) == 0);
	ft_putstr_fd("val=$MS_UNIT_V\nEOF\n", pfd[1]);
	close(pfd[1]);
	saved_in = dup(STDIN_FILENO);
	dup2(pfd[0], STDIN_FILENO);
	close(pfd[0]);
	assert(collect_heredocs(ctx->current_ast, ctx) == 1);
	dup2(saved_in, STDIN_FILENO);
	close(saved_in);
	assert(ctx->current_ast->redirs->heredoc_fd >= 0);
	n = read(ctx->current_ast->redirs->heredoc_fd, buf, sizeof(buf) - 1);
	assert(n > 0);
	buf[n] = '\0';
	assert(strcmp(buf, "val=hello\n") == 0);
	free_ast(&ctx->current_ast);
}

int	main(void)
{
	t_shell	ctx;
	char	*envp[2];

	envp[0] = "MS_UNIT_V=hello";
	envp[1] = NULL;
	ft_bzero(&ctx, sizeof(t_shell));
	ctx.env_list = init_env_list(envp);
	printf("[UNIT] Running Expander, Redirection & Heredoc unit tests...\n");
	test_expander_quotes_and_vars(&ctx);
	test_file_redirections(&ctx);
	test_heredoc_collection(&ctx);
	free_child_resources(&ctx);
	printf("[UNIT] All Redirection & Heredoc unit tests PASSED!\n");
	return (0);
}

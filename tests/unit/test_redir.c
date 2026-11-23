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

static void	test_heredoc_excl_no_clobber(t_shell *ctx)
{
	int			guard_fd;
	int			pfd[2];
	int			saved_in;
	t_token		*toks;
	char		buf[32];
	ssize_t		n;
	int			err;

	guard_fd = open("/tmp/.minishell_hd_0", O_WRONLY | O_CREAT | O_TRUNC, 0600);
	assert(guard_fd >= 0);
	assert(write(guard_fd, "keep_me\n", 8) == 8);
	close(guard_fd);
	toks = tokenize_input("cat << EOF", &err);
	ctx->current_ast = parse_tokens(toks, &err);
	free_tokens(&toks);
	assert(ctx->current_ast != NULL);
	assert(pipe(pfd) == 0);
	ft_putstr_fd("ok\nEOF\n", pfd[1]);
	close(pfd[1]);
	saved_in = dup(STDIN_FILENO);
	dup2(pfd[0], STDIN_FILENO);
	close(pfd[0]);
	assert(collect_heredocs(ctx->current_ast, ctx) == 1);
	dup2(saved_in, STDIN_FILENO);
	close(saved_in);
	free_ast(&ctx->current_ast);
	guard_fd = open("/tmp/.minishell_hd_0", O_RDONLY);
	assert(guard_fd >= 0);
	n = read(guard_fd, buf, sizeof(buf) - 1);
	close(guard_fd);
	unlink("/tmp/.minishell_hd_0");
	assert(n == 8 && memcmp(buf, "keep_me\n", 8) == 0);
}

static void	test_word_splitting_and_ambig(t_shell *ctx)
{
	char	**f;
	char	*out;
	int		err;

	assert(env_set(&ctx->env_list, "MS_WS", "a   b") == 0);
	assert(env_set(&ctx->env_list, "MS_SP", "   ") == 0);
	f = expand_word_fields("$MS_WS", ctx, &err);
	assert(!err && f && strcmp(f[0], "a") == 0
		&& strcmp(f[1], "b") == 0 && f[2] == NULL);
	free_str_array(f);
	f = expand_word_fields("\"$MS_WS\"", ctx, &err);
	assert(!err && f && strcmp(f[0], "a   b") == 0 && f[1] == NULL);
	free_str_array(f);
	f = expand_word_fields("pre$MS_WS\"post\"", ctx, &err);
	assert(!err && f && strcmp(f[0], "prea") == 0
		&& strcmp(f[1], "bpost") == 0 && f[2] == NULL);
	free_str_array(f);
	f = expand_word_fields("$MS_SP", ctx, &err);
	assert(!err && f == NULL);
	f = expand_word_fields("\"\"$MS_SP", ctx, &err);
	assert(!err && f && strcmp(f[0], "") == 0 && f[1] == NULL);
	free_str_array(f);
	f = expand_word_fields("$MS_SP\"\"", ctx, &err);
	assert(!err && f && strcmp(f[0], "") == 0 && f[1] == NULL);
	free_str_array(f);
	out = expand_word("$\"hello\" $'world' \"$\"\"a\" $1abc", ctx);
	assert(out && strcmp(out, "hello world $a abc") == 0);
	free(out);
	assert(expand_redir_target("$MS_NO_SUCH", ctx) == NULL);
	assert(expand_redir_target("$MS_WS", ctx) == NULL);
	out = expand_redir_target("\"$MS_WS\"", ctx);
	assert(out && strcmp(out, "a   b") == 0);
	free(out);
}

static void	test_expander_long_strbuf(t_shell *ctx)
{
	char	raw[4200];
	char	*stripped;
	char	*expanded;
	size_t	i;

	raw[0] = '"';
	i = 1;
	while (i < 4097)
		raw[i++] = 'x';
	raw[i++] = '"';
	raw[i] = '\0';
	stripped = strip_quotes_only(raw);
	assert(stripped != NULL && strlen(stripped) == 4096);
	free(stripped);
	expanded = expand_word(raw, ctx);
	assert(expanded != NULL && strlen(expanded) == 4096);
	free(expanded);
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
	test_expander_long_strbuf(&ctx);
	test_file_redirections(&ctx);
	test_heredoc_collection(&ctx);
	test_heredoc_excl_no_clobber(&ctx);
	test_word_splitting_and_ambig(&ctx);
	free_child_resources(&ctx);
	printf("[UNIT] All Redirection & Heredoc unit tests PASSED!\n");
	return (0);
}

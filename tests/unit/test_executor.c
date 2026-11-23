#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "minishell.h"

volatile sig_atomic_t	g_signal_num = 0;

static int	run_line_ctx(const char *line, t_shell *ctx)
{
	int	err;

	err = 0;
	ctx->current_tokens = tokenize_input(line, &err);
	if (!ctx->current_tokens)
		return (err);
	ctx->current_ast = parse_tokens(ctx->current_tokens, &err);
	free_tokens(&ctx->current_tokens);
	if (!ctx->current_ast)
		return (err);
	err = execute_ast(ctx->current_ast, ctx);
	free_ast(&ctx->current_ast);
	return (err);
}

static void	test_path_and_status_codes(t_shell *ctx)
{
	int	rc;

	rc = run_line_ctx("true", ctx);
	assert(rc == 0);
	rc = run_line_ctx("false", ctx);
	assert(rc == 1);
	rc = run_line_ctx("nonexistent_cmd_xyz_42", ctx);
	assert(rc == 127);
	rc = run_line_ctx("/tmp", ctx);
	assert(rc == 126);
	rc = run_line_ctx("/nonexistent/bin/foo", ctx);
	assert(rc == 127);
}

static void	test_pipeline_status_propagation(t_shell *ctx)
{
	int	rc;

	rc = run_line_ctx("ls /nonexistent_dir_42 | true", ctx);
	assert(rc == 0);
	rc = run_line_ctx("true | false", ctx);
	assert(rc == 1);
	rc = run_line_ctx("true | nonexistent_right_cmd_42", ctx);
	assert(rc == 127);
}

static void	test_multi_stage_pipe_no_hang(t_shell *ctx)
{
	int	rc;

	rc = run_line_ctx("/bin/echo hello | cat | cat | wc -l", ctx);
	assert(rc == 0);
	rc = run_line_ctx("true && false || true", ctx);
	assert(rc == 0);
}

static void	test_path_edge_cases(t_shell *ctx)
{
	char	*orig_path;
	int		fd;

	orig_path = ft_strdup(env_get(ctx->env_list, "PATH"));
	assert(run_line_ctx("..", ctx) == 127);
	mkdir("/tmp/ms_u_p1", 0755);
	mkdir("/tmp/ms_u_p2", 0755);
	mkdir("/tmp/ms_u_p1/shadow_u", 0755);
	fd = open("/tmp/ms_u_p1/noexec_u", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	assert(fd >= 0);
	close(fd);
	fd = open("/tmp/ms_u_p2/shadow_u", O_WRONLY | O_CREAT | O_TRUNC, 0755);
	assert(fd >= 0);
	assert(write(fd, "#!/bin/sh\nexit 0\n", 17) == 17);
	close(fd);
	assert(env_set(&ctx->env_list, "PATH", "/tmp/ms_u_p1") == 0);
	assert(run_line_ctx("shadow_u", ctx) == 127);
	assert(run_line_ctx("noexec_u", ctx) == 126);
	assert(env_set(&ctx->env_list, "PATH", "/tmp/ms_u_p1:/tmp/ms_u_p2") == 0);
	assert(run_line_ctx("shadow_u", ctx) == 0);
	env_set(&ctx->env_list, "PATH", orig_path);
	free(orig_path);
	unlink("/tmp/ms_u_p1/noexec_u");
	rmdir("/tmp/ms_u_p1/shadow_u");
	unlink("/tmp/ms_u_p2/shadow_u");
	rmdir("/tmp/ms_u_p1");
	rmdir("/tmp/ms_u_p2");
}

int	main(int argc, char **argv, char **envp)
{
	t_shell	ctx;

	(void)argc;
	(void)argv;
	ft_bzero(&ctx, sizeof(t_shell));
	ctx.env_list = init_env_list(envp);
	printf("[UNIT] Running Execution & Pipeline Engine unit tests...\n");
	test_path_and_status_codes(&ctx);
	test_pipeline_status_propagation(&ctx);
	test_multi_stage_pipe_no_hang(&ctx);
	test_path_edge_cases(&ctx);
	free_child_resources(&ctx);
	printf("[UNIT] All Executor & Pipeline unit tests PASSED!\n");
	return (0);
}

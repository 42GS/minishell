#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "minishell.h"

volatile sig_atomic_t	g_signal_num = 0;

static void	test_env_ops_and_defaults(void)
{
	t_shell		ctx;
	char		*envp[2];
	const char	*val;

	envp[0] = "SHLVL=4";
	envp[1] = NULL;
	ft_bzero(&ctx, sizeof(t_shell));
	ctx.env_list = init_env_list(envp);
	init_env_defaults(&ctx);
	val = env_get(ctx.env_list, "SHLVL");
	assert(val != NULL && strcmp(val, "5") == 0);
	assert(env_get(ctx.env_list, "PWD") != NULL);
	assert(env_set(&ctx.env_list, "FOO", "bar") == 0);
	assert(strcmp(env_get(ctx.env_list, "FOO"), "bar") == 0);
	assert(env_set(&ctx.env_list, "FOO", NULL) == 0);
	assert(strcmp(env_get(ctx.env_list, "FOO"), "bar") == 0);
	assert(env_set(&ctx.env_list, "FOO", "baz") == 0);
	assert(strcmp(env_get(ctx.env_list, "FOO"), "baz") == 0);
	assert(env_unset(&ctx.env_list, "FOO") == 0);
	assert(env_get(ctx.env_list, "FOO") == NULL);
	free_child_resources(&ctx);
}

static void	test_export_and_unset_builtins(void)
{
	t_shell	ctx;
	char	*exp_args1[4];
	char	*exp_args2[3];
	char	*unset_args[3];

	ft_bzero(&ctx, sizeof(t_shell));
	exp_args1[0] = "export";
	exp_args1[1] = "VALID_A=hello";
	exp_args1[2] = "1BAD=nope";
	exp_args1[3] = NULL;
	assert(ft_export(exp_args1, &ctx) == 1);
	assert(strcmp(env_get(ctx.env_list, "VALID_A"), "hello") == 0);
	exp_args2[0] = "export";
	exp_args2[1] = "VALID_A+=_world";
	exp_args2[2] = NULL;
	assert(ft_export(exp_args2, &ctx) == 0);
	assert(strcmp(env_get(ctx.env_list, "VALID_A"), "hello_world") == 0);
	unset_args[0] = "unset";
	unset_args[1] = "VALID_A";
	unset_args[2] = NULL;
	assert(ft_unset(unset_args, &ctx) == 0);
	assert(env_get(ctx.env_list, "VALID_A") == NULL);
	free_child_resources(&ctx);
}

static void	test_cd_and_pwd_builtins(void)
{
	t_shell	ctx;
	char	*cd_args[3];
	char	*orig_dir;

	ft_bzero(&ctx, sizeof(t_shell));
	orig_dir = getcwd(NULL, 0);
	assert(orig_dir != NULL);
	env_set(&ctx.env_list, "PWD", orig_dir);
	cd_args[0] = "cd";
	cd_args[1] = "/tmp";
	cd_args[2] = NULL;
	assert(ft_cd(cd_args, &ctx) == 0);
	assert(strcmp(env_get(ctx.env_list, "OLDPWD"), orig_dir) == 0);
	cd_args[1] = orig_dir;
	assert(ft_cd(cd_args, &ctx) == 0);
	free(orig_dir);
	free_child_resources(&ctx);
}

static void	test_exit_64bit_and_dispatch(void)
{
	t_shell	ctx;
	char	*exit_many[4];
	int		code;
	pid_t	pid;
	int		status;

	assert(parse_exit_code("0", &code) && code == 0);
	assert(parse_exit_code("  +259  ", &code) && code == 3);
	assert(parse_exit_code("-1", &code) && code == 255);
	assert(parse_exit_code("9223372036854775807", &code) && code == 255);
	assert(parse_exit_code("-9223372036854775808", &code) && code == 0);
	assert(!parse_exit_code("9223372036854775808", &code));
	assert(!parse_exit_code("-9223372036854775809", &code));
	assert(!parse_exit_code("42abc", &code));
	assert(!parse_exit_code("1 2", &code));
	assert(!parse_exit_code("--1", &code));
	assert(!parse_exit_code("", &code));
	ft_bzero(&ctx, sizeof(t_shell));
	exit_many[0] = "exit";
	exit_many[1] = "1";
	exit_many[2] = "2";
	exit_many[3] = NULL;
	assert(ft_exit(exit_many, &ctx, false) == 1);
	pid = fork();
	assert(pid >= 0);
	if (pid == 0)
	{
		exit_many[2] = NULL;
		exit_many[1] = "259";
		ft_exit(exit_many, &ctx, false);
	}
	assert(waitpid(pid, &status, 0) == pid);
	assert(WIFEXITED(status) && WEXITSTATUS(status) == 3);
}

int	main(void)
{
	printf("[UNIT] Running 7 Builtins & Env unit tests...\n");
	test_env_ops_and_defaults();
	test_export_and_unset_builtins();
	test_cd_and_pwd_builtins();
	test_exit_64bit_and_dispatch();
	printf("[UNIT] All Builtins & Env unit tests PASSED!\n");
	return (0);
}

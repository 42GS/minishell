#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include "minishell.h"

volatile sig_atomic_t	g_signal_num = 0;

static void	test_sync_signal_status(void)
{
	t_shell	ctx;

	ft_bzero(&ctx, sizeof(t_shell));
	g_signal_num = SIGINT;
	sync_signal_status(&ctx);
	assert(ctx.last_exit_status == 130);
	assert(g_signal_num == 0);
	g_signal_num = SIGQUIT;
	sync_signal_status(&ctx);
	assert(ctx.last_exit_status == 131);
	assert(g_signal_num == 0);
}

static void	test_signal_state_handlers(void)
{
	struct sigaction	sa_int;
	struct sigaction	sa_quit;

	set_signal_state(SIG_STATE_INTERACTIVE);
	assert(sigaction(SIGINT, NULL, &sa_int) == 0);
	assert(sigaction(SIGQUIT, NULL, &sa_quit) == 0);
	assert(sa_int.sa_handler != SIG_IGN && sa_int.sa_handler != SIG_DFL);
	assert(sa_quit.sa_handler == SIG_IGN);
	set_signal_state(SIG_STATE_EXECUTION);
	assert(sigaction(SIGINT, NULL, &sa_int) == 0);
	assert(sigaction(SIGQUIT, NULL, &sa_quit) == 0);
	assert(sa_int.sa_handler == SIG_IGN);
	assert(sa_quit.sa_handler == SIG_IGN);
	set_signal_state(SIG_STATE_CHILD);
	assert(sigaction(SIGINT, NULL, &sa_int) == 0);
	assert(sigaction(SIGQUIT, NULL, &sa_quit) == 0);
	assert(sa_int.sa_handler == SIG_DFL);
	assert(sa_quit.sa_handler == SIG_DFL);
	set_signal_state(SIG_STATE_HEREDOC);
	assert(sigaction(SIGINT, NULL, &sa_int) == 0);
	assert(sigaction(SIGQUIT, NULL, &sa_quit) == 0);
	assert(sa_int.sa_handler == SIG_DFL);
	assert(sa_quit.sa_handler == SIG_IGN);
}

static void	test_wait_child_signal_codes(void)
{
	t_shell	ctx;
	pid_t	pid;
	int		rc;

	ft_bzero(&ctx, sizeof(t_shell));
	ctx.in_subshell = true;
	pid = fork();
	assert(pid >= 0);
	if (pid == 0)
	{
		set_signal_state(SIG_STATE_CHILD);
		pause();
		exit(0);
	}
	usleep(20000);
	assert(kill(pid, SIGINT) == 0);
	rc = wait_child_status(pid, &ctx);
	assert(rc == 130);
	pid = fork();
	assert(pid >= 0);
	if (pid == 0)
	{
		set_signal_state(SIG_STATE_CHILD);
		pause();
		exit(0);
	}
	usleep(20000);
	assert(kill(pid, SIGQUIT) == 0);
	rc = wait_child_status(pid, &ctx);
	assert(rc == 131);
}

int	main(void)
{
	printf("[UNIT] Running Signal Dispatcher & Wait Status unit tests...\n");
	test_sync_signal_status();
	test_signal_state_handlers();
	test_wait_child_signal_codes();
	set_signal_state(SIG_STATE_CHILD);
	printf("[UNIT] All Signal unit tests PASSED!\n");
	return (0);
}

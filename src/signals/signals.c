/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 12:15:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 12:15:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file signals.c
 * @brief 4-state signal FSM and async-signal-safe g_signal_num tracking.
 */

/**
 * @brief  Install @p handler for @p signo via sigaction with zero flags.
 *
 * Leaves sa_flags at 0 (omitting SA_RESTART) so blocking readline/read calls
 * can be interrupted cleanly on signal delivery without restarting.
 *
 * @param  signo    Signal number (e.g., SIGINT or SIGQUIT).
 * @param  handler  Signal handler function, SIG_IGN, or SIG_DFL.
 */
static void	set_sig_handler(int signo, void (*handler)(int))
{
	struct sigaction	sa;

	ft_bzero(&sa, sizeof(struct sigaction));
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(signo, &sa, NULL);
}

/**
 * @brief  Interactive SIGINT handler for the readline prompt state.
 *
 * Stores @p signo in volatile sig_atomic_t g_signal_num (single-global
 * subject constraint) rather than mutating shell state inside the handler,
 * then clears and redisplays a fresh readline prompt on the next line.
 *
 * @param  signo  Delivered signal number (SIGINT).
 */
static void	handle_sigint_interactive(int signo)
{
	g_signal_num = signo;
	write(STDOUT_FILENO, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

/**
 * @brief  Transition SIGINT and SIGQUIT dispositions across the 4 FSM states.
 *
 * - SIG_STATE_INTERACTIVE: catch SIGINT at prompt, ignore SIGQUIT.
 * - SIG_STATE_EXECUTION: parent ignores both while waiting on children.
 * - SIG_STATE_CHILD: restore SIG_DFL for both before execve or subshell.
 * - SIG_STATE_HEREDOC: SIGINT uses SIG_DFL to abort heredoc child, SIGQUIT
 *   remains ignored.
 *
 * @param  state  Target signal disposition state.
 */
void	set_signal_state(t_sig_state state)
{
	if (state == SIG_STATE_INTERACTIVE)
	{
		set_sig_handler(SIGINT, handle_sigint_interactive);
		set_sig_handler(SIGQUIT, SIG_IGN);
	}
	else if (state == SIG_STATE_EXECUTION)
	{
		set_sig_handler(SIGINT, SIG_IGN);
		set_sig_handler(SIGQUIT, SIG_IGN);
	}
	else if (state == SIG_STATE_CHILD)
	{
		set_sig_handler(SIGINT, SIG_DFL);
		set_sig_handler(SIGQUIT, SIG_DFL);
	}
	else if (state == SIG_STATE_HEREDOC)
	{
		set_sig_handler(SIGINT, SIG_DFL);
		set_sig_handler(SIGQUIT, SIG_IGN);
	}
}

/**
 * @brief  Consume g_signal_num in normal execution and update exit status.
 *
 * Reads and resets the volatile sig_atomic_t flag outside signal context,
 * mapping SIGINT to 130 and SIGQUIT to 131 in ctx->last_exit_status.
 *
 * @param  ctx  Shell context whose last_exit_status is synchronized.
 */
void	sync_signal_status(t_shell *ctx)
{
	if (!ctx)
		return ;
	if (g_signal_num == SIGINT)
	{
		ctx->last_exit_status = 130;
		g_signal_num = 0;
	}
	else if (g_signal_num == SIGQUIT)
	{
		ctx->last_exit_status = 131;
		g_signal_num = 0;
	}
}

/**
 * @brief  Print Bash-style feedback when a foreground child dies by signal.
 *
 * Emits a newline on SIGINT or "Quit (core dumped)" on SIGQUIT when
 * waiting from the top-level shell; suppressed when @p in_subshell is true
 * so nested pipelines do not print duplicate diagnostics.
 *
 * @param  status       Raw waitpid status word from the child process.
 * @param  in_subshell  True if executing inside a forked subshell.
 */
void	report_child_signal(int status, bool in_subshell)
{
	if (in_subshell || !WIFSIGNALED(status))
		return ;
	if (WTERMSIG(status) == SIGINT)
		write(STDERR_FILENO, "\n", 1);
	else if (WTERMSIG(status) == SIGQUIT)
		ft_putendl_fd("Quit (core dumped)", STDERR_FILENO);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor_wait.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 12:15:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 12:15:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file executor_wait.c
 * @brief Child reaping with EINTR retry loops and exit-status normalization.
 */

/**
 * @brief  Wait for pid while retrying whenever waitpid() fails with EINTR.
 * @param  pid     Child PID to reap.
 * @param  status  Output raw wait status from waitpid().
 * @return 0 on success, -1 on non-EINTR waitpid() error.
 *
 * Retrying on EINTR ensures a caught signal never abandons reaping and
 * leaves zombie processes behind.
 */
static int	waitpid_eintr(pid_t pid, int *status)
{
	while (waitpid(pid, status, 0) == -1)
	{
		if (errno != EINTR)
			return (-1);
	}
	return (0);
}

/**
 * @brief  Convert raw waitpid status into POSIX shell exit status.
 * @param  status  Raw wait status from waitpid().
 * @return Exit code (0-255), 128 + signal on signal kill, or 1 fallback.
 */
int	normalize_wait_status(int status)
{
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	if (WIFSIGNALED(status))
		return (128 + WTERMSIG(status));
	return (1);
}

/**
 * @brief  Reap a single foreground child and restore interactive signals.
 * @param  pid  PID of the foreground child process.
 * @param  ctx  Shell context updated with ctx->interrupted on SIGINT.
 * @return Normalized child exit status, or 1 if waitpid() fails.
 */
int	wait_child_status(pid_t pid, t_shell *ctx)
{
	int	status;

	if (waitpid_eintr(pid, &status) == -1)
	{
		if (!ctx->in_subshell)
			set_signal_state(SIG_STATE_INTERACTIVE);
		return (1);
	}
	if (!ctx->in_subshell)
		set_signal_state(SIG_STATE_INTERACTIVE);
	report_child_signal(status, ctx->in_subshell);
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
		ctx->interrupted = true;
	return (normalize_wait_status(status));
}

/**
 * @brief  Record SIGINT/SIGQUIT termination from a pipeline stage.
 * @param  status      Raw wait status of one reaped pipeline child.
 * @param  sig_status  Out status updated for SIGINT or SIGQUIT kills.
 * @param  ctx         Shell context; sets interrupted = true on SIGINT.
 */
static void	record_pipe_sig(int status, int *sig_status, t_shell *ctx)
{
	int	sig;

	if (!WIFSIGNALED(status))
		return ;
	sig = WTERMSIG(status);
	if (sig == SIGINT)
		ctx->interrupted = true;
	if (sig == SIGINT || sig == SIGQUIT)
		*sig_status = status;
}

/**
 * @brief  Reap every spawned pipeline child and return the last stage status.
 * @param  pids   Array of count child PIDs in pipeline order.
 * @param  count  Number of children spawned by spawn_pipe_children().
 * @param  ctx    Shell context for signal restoration and interrupt state.
 * @return Normalized exit status of the final stage (or 1 on wait error).
 *
 * Uses waitpid_eintr() on every child so no zombies remain and terminal signal
 * diagnostics print only after all pipeline stages have finished.
 */
int	wait_pipeline(pid_t *pids, int count, t_shell *ctx)
{
	int	i;
	int	status;
	int	last_status;
	int	sig_status;

	last_status = 0;
	sig_status = 0;
	i = 0;
	while (i < count)
	{
		if (waitpid_eintr(pids[i], &status) == -1)
			last_status = 1;
		else
		{
			if (i == count - 1)
				last_status = normalize_wait_status(status);
			record_pipe_sig(status, &sig_status, ctx);
		}
		i++;
	}
	if (!ctx->in_subshell)
		set_signal_state(SIG_STATE_INTERACTIVE);
	if (sig_status != 0)
		report_child_signal(sig_status, ctx->in_subshell);
	return (last_status);
}

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

static int	waitpid_eintr(pid_t pid, int *status)
{
	while (waitpid(pid, status, 0) == -1)
	{
		if (errno != EINTR)
			return (-1);
	}
	return (0);
}

int	normalize_wait_status(int status)
{
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	if (WIFSIGNALED(status))
		return (128 + WTERMSIG(status));
	return (1);
}

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

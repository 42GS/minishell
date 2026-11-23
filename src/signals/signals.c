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

static void	set_sig_handler(int signo, void (*handler)(int))
{
	struct sigaction	sa;

	ft_bzero(&sa, sizeof(struct sigaction));
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(signo, &sa, NULL);
}

static void	handle_sigint_interactive(int signo)
{
	g_signal_num = signo;
	write(STDOUT_FILENO, "\n", 1);
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
}

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

void	report_child_signal(int status, bool in_subshell)
{
	if (in_subshell || !WIFSIGNALED(status))
		return ;
	if (WTERMSIG(status) == SIGINT)
		write(STDERR_FILENO, "\n", 1);
	else if (WTERMSIG(status) == SIGQUIT)
		ft_putendl_fd("Quit (core dumped)", STDERR_FILENO);
}

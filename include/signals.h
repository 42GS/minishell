/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 12:15:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALS_H
# define SIGNALS_H

# include <stdbool.h>
# include <signal.h>
# include <termios.h>

/**
 * @file signals.h
 * @brief Signal state machine and TTY ECHOCTL management interfaces.
 */

/** @brief Forward declaration avoiding circular include with minishell.h. */
typedef struct s_shell	t_shell;

/**
 * @enum  e_sig_state
 * @brief Shell execution phases dictating SIGINT/SIGQUIT dispositions.
 *
 * Key states:
 * - SIG_STATE_INTERACTIVE: Parent at prompt; SIGINT resets readline, SIGQUIT
 *   is ignored.
 * - SIG_STATE_EXECUTION: Parent waiting on children; both SIGINT and SIGQUIT
 *   are ignored so only children react.
 * - SIG_STATE_CHILD: Forked command child; restores SIG_DFL for both.
 * - SIG_STATE_HEREDOC: Forked heredoc reader; SIGINT is SIG_DFL, SIGQUIT is
 *   ignored.
 */
typedef enum e_sig_state
{
	SIG_STATE_INTERACTIVE,
	SIG_STATE_EXECUTION,
	SIG_STATE_CHILD,
	SIG_STATE_HEREDOC
}	t_sig_state;

/**
 * @brief  Install SIGINT and SIGQUIT handlers for the requested shell phase.
 *
 * Uses sigaction() with empty mask and sa_flags = 0 so readline/read can be
 * interrupted cleanly without automatic SA_RESTART surprises.
 *
 * @param  state  Target signal disposition phase.
 */
void	set_signal_state(t_sig_state state);

/**
 * @brief  Drain g_signal_num into ctx->last_exit_status (130 or 131).
 *
 * Keeps async-signal handlers minimal by deferring struct mutation to safe
 * synchronous checkpoints in the main loop and heredoc failure path.
 *
 * @param  ctx  Shell context updated if g_signal_num is non-zero (NULL-safe).
 */
void	sync_signal_status(t_shell *ctx);

/**
 * @brief  Save TTY termios into *orig_term and clear ECHOCTL.
 *
 * Prevents literal "^C"/"^\\" echoes on interactive readline prompts.
 * Safe no-op when STDIN_FILENO is not a TTY or orig_term is NULL.
 *
 * @param  orig_term  Snapshot buffer populated for later restore_terminal().
 */
void	disable_echoctl(struct termios *orig_term);

/**
 * @brief  Restore terminal attributes captured by disable_echoctl().
 *
 * @param  orig_term  Saved termios state (no-op if NULL or non-TTY).
 */
void	restore_terminal(const struct termios *orig_term);

/**
 * @brief  Print bash-compatible signal termination notice for foreground jobs.
 *
 * Prints newline on SIGINT and "Quit (core dumped)" on SIGQUIT unless inside
 * a subshell/pipeline child where only the top-level shell should report.
 *
 * @param  status       Raw waitpid() child status.
 * @param  in_subshell  True in subshells to suppress duplicate notice.
 */
void	report_child_signal(int status, bool in_subshell);

#endif

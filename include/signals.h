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

typedef struct s_shell	t_shell;

typedef enum e_sig_state
{
	SIG_STATE_INTERACTIVE,
	SIG_STATE_EXECUTION,
	SIG_STATE_CHILD,
	SIG_STATE_HEREDOC
}	t_sig_state;

void	set_signal_state(t_sig_state state);
void	sync_signal_status(t_shell *ctx);
void	disable_echoctl(struct termios *orig_term);
void	restore_terminal(const struct termios *orig_term);
void	report_child_signal(int status, bool in_subshell);

#endif

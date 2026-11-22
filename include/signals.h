/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALS_H
# define SIGNALS_H

# include <signal.h>
# include <termios.h>

typedef enum e_sig_state
{
	SIG_STATE_INTERACTIVE,
	SIG_STATE_EXECUTION,
	SIG_STATE_HEREDOC
}	t_sig_state;

void	set_signal_state(t_sig_state state);
void	disable_echoctl(struct termios *orig_term);
void	restore_terminal(const struct termios *orig_term);

#endif

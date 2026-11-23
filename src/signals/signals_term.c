/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals_term.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 12:15:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/23 12:15:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file signals_term.c
 * @brief Terminal ECHOCTL suppression and restoration around interactive I/O.
 */

#ifndef ECHOCTL
# define ECHOCTL 0
#endif

/**
 * @brief  Save stdin termios attributes and disable ECHOCTL on a TTY.
 *
 * Prevents the terminal driver from echoing "^C" or "^\\" control glyphs
 * when Ctrl-C / Ctrl-\ are pressed at the interactive prompt, matching Bash.
 * Safe no-op when stdin is not a TTY.
 *
 * @param  orig_term  Output struct storing original attributes for restore.
 */
void	disable_echoctl(struct termios *orig_term)
{
	struct termios	term;

	if (!orig_term || !isatty(STDIN_FILENO))
		return ;
	if (tcgetattr(STDIN_FILENO, orig_term) != 0)
		return ;
	term = *orig_term;
	term.c_lflag &= ~(tcflag_t)ECHOCTL;
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/**
 * @brief  Restore previously saved terminal attributes on TTY stdin.
 *
 * Called before executing child commands and on shell exit so external
 * programs inherit the user's original ECHOCTL and line-discipline flags.
 *
 * @param  orig_term  Saved termios attributes captured by disable_echoctl().
 */
void	restore_terminal(const struct termios *orig_term)
{
	if (!orig_term || !isatty(STDIN_FILENO))
		return ;
	tcsetattr(STDIN_FILENO, TCSANOW, orig_term);
}

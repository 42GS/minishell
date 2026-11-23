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

#ifndef ECHOCTL
# define ECHOCTL 0
#endif

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

void	restore_terminal(const struct termios *orig_term)
{
	if (!orig_term || !isatty(STDIN_FILENO))
		return ;
	tcsetattr(STDIN_FILENO, TCSANOW, orig_term);
}

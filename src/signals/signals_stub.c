/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals_stub.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	set_signal_state(t_sig_state state)
{
	(void)state;
}

void	disable_echoctl(struct termios *orig_term)
{
	(void)orig_term;
}

void	restore_terminal(const struct termios *orig_term)
{
	(void)orig_term;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:35:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:35:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <stdbool.h>
# include <signal.h>
# include <termios.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "libft.h"
# include "lexer.h"
# include "parser.h"
# include "executor.h"
# include "builtins.h"
# include "signals.h"

# define SHELL_PROMPT "minishell$ "

extern volatile sig_atomic_t	g_signal_num;

typedef struct s_shell
{
	t_env_node		*env_list;
	int				last_exit_status;
	char			*cwd;
	char			*line;
	bool			is_interactive;
	t_ast_node		*current_ast;
	t_token			*current_tokens;
	struct termios	orig_term;
}	t_shell;

void	free_str_array(char **arr);
void	free_child_resources(t_shell *ctx);
char	*read_stdin_line(void);
void	exit_child_clean(t_shell *ctx, char *path, char **envp, int code);

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXECUTOR_H
# define EXECUTOR_H

# include "parser.h"

typedef struct s_shell	t_shell;

int		execute_ast(t_ast_node *node, t_shell *ctx);
int		apply_redirections(t_redir *redirs, t_shell *ctx);
int		collect_heredocs(t_ast_node *node, t_shell *ctx);
char	*resolve_cmd_path(const char *cmd, t_shell *ctx, int *err_code);

#endif

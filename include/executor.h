/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:48:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:48:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXECUTOR_H
# define EXECUTOR_H

# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <errno.h>
# include <string.h>
# include "parser.h"

typedef struct s_shell	t_shell;

typedef struct s_pipe_ctx
{
	t_ast_node	**cmds;
	int			count;
	int			*pipes;
	pid_t		*pids;
}	t_pipe_ctx;

typedef struct s_exp_buf
{
	char			**fields;
	t_strbuf		cur;
	t_quote_state	st;
}	t_exp_buf;

int		execute_ast(t_ast_node *node, t_shell *ctx);
int		execute_pipeline(t_ast_node *node, t_shell *ctx);
int		apply_redirections(t_redir *redirs, t_shell *ctx);
int		collect_heredocs(t_ast_node *node, t_shell *ctx);
char	*resolve_cmd_path(const char *cmd, t_shell *ctx, int *err_code);

void	close_all_pipes(int *pipes, int pipe_count);
int		spawn_pipe_children(t_pipe_ctx *pctx, t_shell *ctx);
int		wait_pipeline(pid_t *pids, int count, t_shell *ctx);
int		wait_child_status(pid_t pid, t_shell *ctx);
int		normalize_wait_status(int status);
void	exec_cmd_in_child(t_ast_node *cmd, t_shell *ctx);
void	print_exec_error(const char *cmd, const char *msg);
void	run_heredoc_child(int fds[2], t_redir *redir, t_shell *ctx);
void	close_ast_heredocs(t_ast_node *node, t_ast_node *skip);
char	*extract_path_dir(const char *p, size_t *pos, bool *done);
char	*join_path_cmd(char *dir, const char *cmd);
int		check_candidate(char *cand, char **denied);

char	*expand_dollar(const char *s, size_t *i, t_shell *ctx);
char	*expand_word(const char *raw, t_shell *ctx);
char	*expand_heredoc_line(const char *raw, t_shell *ctx);
char	*strip_quotes_only(const char *raw);
char	**expand_word_fields(const char *raw, t_shell *ctx, int *err);
char	*expand_redir_target(const char *raw, t_shell *ctx);
int		expand_cmd_args(t_ast_node *cmd, t_shell *ctx);
bool	is_dollar_quote(const char *s, size_t i, t_quote_state st);
int		sb_push_str_free(t_strbuf *sb, char *chunk);

#endif

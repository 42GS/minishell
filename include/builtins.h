/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILTINS_H
# define BUILTINS_H

# include <stdbool.h>

typedef struct s_shell	t_shell;

typedef struct s_env_node
{
	char				*key;
	char				*value;
	struct s_env_node	*next;
}	t_env_node;

t_env_node	*init_env_list(char **envp);
void		free_env_list(t_env_node **env_list);
const char	*env_get(t_env_node *env, const char *key);
int			env_set(t_env_node **env, const char *key, const char *value);
int			env_unset(t_env_node **env, const char *key);
char		**env_list_to_array(t_env_node *env);
bool		is_builtin(const char *cmd_name);
int			dispatch_builtin(char **argv, t_shell *ctx, bool in_child);
int			ft_echo(char **argv);
int			ft_cd(char **argv, t_shell *ctx);
int			ft_pwd(t_shell *ctx);
int			ft_export(char **argv, t_shell *ctx);
int			ft_unset(char **argv, t_shell *ctx);
int			ft_env(char **argv, t_shell *ctx);
int			ft_exit(char **argv, t_shell *ctx, bool in_child);

#endif

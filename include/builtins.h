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

/**
 * @file builtins.h
 * @brief Environment table operations and shell builtin command interfaces.
 */

/** @brief Forward declaration avoiding circular include with minishell.h. */
typedef struct s_shell	t_shell;

/**
 * @struct s_env_node
 * @brief  Singly-linked environment variable entry owned by t_shell.
 *
 * Key fields:
 * - key: Heap-allocated variable name.
 * - value: Heap-allocated value string, or NULL for value-less exports
 *   (printed by export without args, omitted by env and env_list_to_array).
 * - next: Next node in insertion order.
 */
typedef struct s_env_node
{
	char				*key;
	char				*value;
	struct s_env_node	*next;
}	t_env_node;

/**
 * @brief  Build an owned environment list from the process envp vector.
 *
 * @param  envp  NULL-terminated array of "KEY=VALUE" strings.
 * @return Caller-owned list head (free via free_env_list), or NULL on OOM.
 */
t_env_node	*init_env_list(char **envp);

/**
 * @brief  Ensure default PWD, value-less OLDPWD, and incremented SHLVL exist.
 *
 * @param  ctx  Shell context whose env_list is initialized in place.
 */
void		init_env_defaults(t_shell *ctx);

/**
 * @brief  Free every node, key, and value in *env_list and set it to NULL.
 *
 * @param  env_list  Address of list head pointer (safe no-op if NULL).
 */
void		free_env_list(t_env_node **env_list);

/**
 * @brief  Look up the borrowed value associated with key in env.
 *
 * @param  env  Environment list head.
 * @param  key  Variable name to search for.
 * @return Borrowed value pointer (do not free), or NULL if unset/value-less.
 */
const char	*env_get(t_env_node *env, const char *key);

/**
 * @brief  Insert or update key in *env, keeping existing value if value==NULL.
 *
 * Passing NULL for value registers an export-only entry without clobbering
 * an existing value if key already exists.
 *
 * @param  env    Address of environment list head.
 * @param  key    Variable name to insert or update.
 * @param  value  New value duplicated on heap, or NULL for bare export.
 * @return 0 on success, -1 on invalid args or allocation failure.
 */
int			env_set(t_env_node **env, const char *key, const char *value);

/**
 * @brief  Remove key from *env if present and free its node and strings.
 *
 * @param  env  Address of environment list head.
 * @param  key  Variable name to unlink.
 * @return Always returns 0.
 */
int			env_unset(t_env_node **env, const char *key);

/**
 * @brief  Serialize assigned (non-NULL value) env entries for execve().
 *
 * @param  env  Environment list head.
 * @return Caller-owned NULL-terminated "KEY=VALUE" array, or NULL on OOM.
 */
char		**env_list_to_array(t_env_node *env);

/** @brief Return true if cmd_name matches one of the 7 supported builtins. */
bool		is_builtin(const char *cmd_name);

/**
 * @brief  Dispatch a recognized builtin command with parent/child awareness.
 *
 * @param  argv      Expanded NULL-terminated argument vector.
 * @param  ctx       Shell runtime context.
 * @param  in_child  True inside a forked child/pipeline stage (affects exit).
 * @return Builtin exit status (0..255).
 */
int			dispatch_builtin(char **argv, t_shell *ctx, bool in_child);

/**
 * @brief  Write all bytes of s to fd, handling short writes cleanly.
 *
 * @param  fd  Target file descriptor.
 * @param  s   NUL-terminated string (NULL is a safe no-op returning 0).
 * @return 0 on success, 1 if write() fails.
 */
int			builtin_write_str(int fd, const char *s);

/**
 * @brief  Write s followed by '\n' to fd, propagating write() errors.
 *
 * @param  fd  Target file descriptor.
 * @param  s   NUL-terminated string (NULL writes just '\n').
 * @return 0 on success, 1 if any write() fails.
 */
int			builtin_write_endl(int fd, const char *s);

/**
 * @brief  Print arguments separated by spaces, honoring leading -n flags.
 *
 * @param  argv  Argument vector starting with "echo".
 * @return 0 on success, 1 on stdout write failure.
 */
int			ft_echo(char **argv);

/**
 * @brief  Change working directory (HOME default, "-" -> OLDPWD) and sync env.
 *
 * @param  argv  Argument vector starting with "cd".
 * @param  ctx   Shell context updating PWD, OLDPWD, and ctx->cwd.
 * @return 0 on success, 1 on argument, lookup, or chdir() failure.
 */
int			ft_cd(char **argv, t_shell *ctx);

/**
 * @brief  Print current working directory with ctx->cwd / PWD fallback.
 *
 * @param  ctx  Shell context supplying fallback directory path.
 * @return 0 on success, 1 if cwd lookup or stdout write fails.
 */
int			ft_pwd(t_shell *ctx);

/**
 * @brief  Display sorted exports or validate and apply KEY/KEY=V/KEY+=V args.
 *
 * @param  argv  Argument vector starting with "export".
 * @param  ctx   Shell context whose env_list is updated.
 * @return 0 if all items succeeded, 1 if any identifier or write failed.
 */
int			ft_export(char **argv, t_shell *ctx);

/**
 * @brief  Remove named variables from ctx->env_list (ignores missing names).
 *
 * @param  argv  Argument vector starting with "unset".
 * @param  ctx   Shell context whose env_list is modified.
 * @return Always returns 0.
 */
int			ft_unset(char **argv, t_shell *ctx);

/**
 * @brief  Print assigned KEY=VALUE environment entries (rejects extra args).
 *
 * @param  argv  Argument vector starting with "env".
 * @param  ctx   Shell context supplying env_list.
 * @return 0 on success, 1 on write failure, 127 if an argument is supplied.
 */
int			ft_env(char **argv, t_shell *ctx);

/**
 * @brief  Exit the shell or subshell cleanly after validating exit code arg.
 *
 * Suppresses "exit" prompt echo when in_child or ctx->in_subshell is true;
 * returns 1 without exiting if more than one argument is supplied after a
 * valid numeric code.
 *
 * @param  argv      Argument vector starting with "exit".
 * @param  ctx       Shell context freed prior to exit().
 * @param  in_child  True if invoked in a child process (skips TTY cleanup).
 * @return 1 if too many args prevent exit; otherwise never returns.
 */
int			ft_exit(char **argv, t_shell *ctx, bool in_child);

/**
 * @brief  Parse signed 64-bit decimal exit code reduced modulo 256.
 *
 * Rejects empty strings, non-digits, and signed long long overflow.
 *
 * @param  str   Candidate numeric exit argument (leading/trailing blanks ok).
 * @param  code  Out parameter receiving (unsigned char) value (0..255).
 * @return true if str is a valid signed 64-bit integer, false otherwise.
 */
bool		parse_exit_code(const char *str, int *code);

#endif

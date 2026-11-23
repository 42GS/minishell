/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:40:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 23:40:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * @file main.c
 * @brief Shell entrypoint, REPL loop, and per-line execution driver.
 */

/** @brief Single subject-allowed global holding the last async signal. */
volatile sig_atomic_t	g_signal_num = 0;

/**
 * @brief  Initialize `t_shell` context, environment, and TTY state.
 * @param  ctx   Shell state structure zeroed and populated on startup.
 * @param  envp  Process environment array inherited from `main`.
 * @return Always 0.
 *
 * Initializes `saved_in` and `saved_out` to `-1` (no backup FDs open yet)
 * and disables `ECHOCTL` only when `STDIN_FILENO` is an interactive TTY.
 */
static int	init_shell(t_shell *ctx, char **envp)
{
	ft_bzero(ctx, sizeof(t_shell));
	ctx->saved_in = -1;
	ctx->saved_out = -1;
	ctx->env_list = init_env_list(envp);
	init_env_defaults(ctx);
	ctx->is_interactive = isatty(STDIN_FILENO);
	ctx->cwd = getcwd(NULL, 0);
	if (ctx->is_interactive)
		disable_echoctl(&ctx->orig_term);
	return (0);
}

/**
 * @brief  Release shell resources and restore terminal state on exit.
 * @param  ctx  Shell context to tear down.
 *
 * Clears readline history and restores `orig_term` only in the interactive
 * parent shell (`!ctx->in_subshell`) so subshells never clobber parent TTY.
 */
static void	cleanup_shell(t_shell *ctx)
{
	free_child_resources(ctx);
	if (ctx->is_interactive && !ctx->in_subshell)
	{
		rl_clear_history();
		restore_terminal(&ctx->orig_term);
	}
}

/**
 * @brief  Tokenize, parse, collect heredocs, and execute `ctx->line`.
 * @param  ctx  Shell context holding the current input line and AST state.
 *
 * Frees `current_tokens` immediately after parsing and `current_ast` after
 * execution. Updates `last_exit_status` on lex/parse errors or syncs signal
 * status if heredoc collection is aborted by a signal.
 */
static void	process_line(t_shell *ctx)
{
	int	err;

	err = 0;
	ctx->interrupted = false;
	ctx->current_tokens = tokenize_input(ctx->line, &err);
	if (!ctx->current_tokens)
	{
		if (err != 0)
			ctx->last_exit_status = err;
		return ;
	}
	ctx->current_ast = parse_tokens(ctx->current_tokens, &err);
	free_tokens(&ctx->current_tokens);
	if (!ctx->current_ast)
	{
		if (err != 0)
			ctx->last_exit_status = err;
		return ;
	}
	if (collect_heredocs(ctx->current_ast, ctx))
		execute_ast(ctx->current_ast, ctx);
	else
		sync_signal_status(ctx);
	free_ast(&ctx->current_ast);
}

/**
 * @brief  Run the interactive or non-interactive read-eval-print loop.
 * @param  ctx  Shell context updated on each command cycle.
 *
 * Uses `readline` with prompt/history when `is_interactive` is true and
 * `read_stdin_line` otherwise. Syncs signal exit status right after input,
 * prints `"exit"` to stderr on interactive EOF, and frees `ctx->line`.
 */
static void	run_shell_loop(t_shell *ctx)
{
	while (1)
	{
		set_signal_state(SIG_STATE_INTERACTIVE);
		if (ctx->is_interactive)
			ctx->line = readline(SHELL_PROMPT);
		else
			ctx->line = read_stdin_line();
		sync_signal_status(ctx);
		if (!ctx->line)
		{
			if (ctx->is_interactive)
				ft_putendl_fd("exit", STDERR_FILENO);
			break ;
		}
		if (ctx->is_interactive && *(ctx->line))
			add_history(ctx->line);
		process_line(ctx);
		free(ctx->line);
		ctx->line = NULL;
	}
}

/**
 * @brief  Program entry point driving initialization, REPL, and teardown.
 * @param  argc  Argument count (unused).
 * @param  argv  Argument vector (unused).
 * @param  envp  Initial environment strings inherited from parent process.
 * @return Final `last_exit_status` recorded before shell exit.
 */
int	main(int argc, char **argv, char **envp)
{
	t_shell	ctx;

	(void)argc;
	(void)argv;
	init_shell(&ctx, envp);
	run_shell_loop(&ctx);
	cleanup_shell(&ctx);
	return (ctx.last_exit_status);
}

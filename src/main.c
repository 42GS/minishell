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

volatile sig_atomic_t	g_signal_num = 0;

static int	init_shell(t_shell *ctx, char **envp)
{
	ft_bzero(ctx, sizeof(t_shell));
	ctx->env_list = init_env_list(envp);
	ctx->is_interactive = isatty(STDIN_FILENO);
	ctx->cwd = getcwd(NULL, 0);
	if (ctx->is_interactive)
		tcgetattr(STDIN_FILENO, &ctx->orig_term);
	return (0);
}

static void	cleanup_shell(t_shell *ctx)
{
	free_child_resources(ctx);
	if (ctx->is_interactive)
	{
		rl_clear_history();
		restore_terminal(&ctx->orig_term);
	}
}

static void	process_line(t_shell *ctx)
{
	int	err;

	err = 0;
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
	else if (g_signal_num == SIGINT)
		ctx->last_exit_status = 130;
	free_ast(&ctx->current_ast);
}

static void	run_shell_loop(t_shell *ctx)
{
	while (1)
	{
		set_signal_state(SIG_STATE_INTERACTIVE);
		if (ctx->is_interactive)
			ctx->line = readline(SHELL_PROMPT);
		else
			ctx->line = read_stdin_line();
		if (!ctx->line)
			break ;
		if (ctx->is_interactive && *(ctx->line))
			add_history(ctx->line);
		process_line(ctx);
		free(ctx->line);
		ctx->line = NULL;
	}
}

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

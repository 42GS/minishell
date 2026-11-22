/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:10:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:10:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

volatile sig_atomic_t	g_signal_num = 0;

void	free_str_array(char **arr)
{
	size_t	i;

	if (!arr)
		return ;
	i = 0;
	while (arr[i])
	{
		free(arr[i]);
		i++;
	}
	free(arr);
}

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
	free_ast(&ctx->current_ast);
	free_tokens(&ctx->current_tokens);
	free_env_list(&ctx->env_list);
	if (ctx->cwd)
		free(ctx->cwd);
	if (ctx->is_interactive)
	{
		rl_clear_history();
		restore_terminal(&ctx->orig_term);
	}
}

static void	run_shell_loop(t_shell *ctx)
{
	char	*line;

	while (1)
	{
		set_signal_state(SIG_STATE_INTERACTIVE);
		if (ctx->is_interactive)
			line = readline(SHELL_PROMPT);
		else
			line = NULL;
		if (!line)
			break ;
		if (*line)
			add_history(line);
		free(line);
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

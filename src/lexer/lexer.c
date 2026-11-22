/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yoshin <yoshin@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 22:50:00 by yoshin            #+#    #+#             */
/*   Updated: 2026/11/22 22:50:00 by yoshin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_lexer_space(char c)
{
	return (c == ' ' || (c >= '\t' && c <= '\r'));
}

int	is_operator_start(const char *s)
{
	if (!s || !*s)
		return (0);
	if (*s == '|' || *s == '<' || *s == '>' || *s == '(' || *s == ')')
		return (1);
	if (s[0] == '&' && s[1] == '&')
		return (1);
	return (0);
}

static int	lexer_step(const char *in, size_t *i, t_token **head, int *err)
{
	int	ret;

	if (is_operator_start(&in[*i]))
		return (extract_op_token(in, i, head));
	ret = extract_word_token(in, i, head);
	if (ret == -1)
	{
		lexer_syntax_error("minishell: syntax error: unclosed quote", head,
			err);
		return (-1);
	}
	return (ret);
}

static t_token	*append_eof(t_token **head, int *err_code)
{
	t_token	*eof_tok;

	eof_tok = create_token(TOK_EOF, NULL, QUOTE_NONE);
	if (!eof_tok)
	{
		free_tokens(head);
		if (err_code)
			*err_code = 1;
		return (NULL);
	}
	append_token(head, eof_tok);
	return (*head);
}

t_token	*tokenize_input(const char *input, int *err_code)
{
	t_token	*head;
	size_t	i;
	int		ret;

	head = NULL;
	if (err_code)
		*err_code = 0;
	if (!input)
		return (append_eof(&head, err_code));
	i = 0;
	while (input[i])
	{
		while (input[i] && is_lexer_space(input[i]))
			i++;
		if (!input[i])
			break ;
		ret = lexer_step(input, &i, &head, err_code);
		if (ret < 0)
			return (NULL);
		if (ret == 0)
			return (free_tokens(&head), NULL);
	}
	return (append_eof(&head, err_code));
}

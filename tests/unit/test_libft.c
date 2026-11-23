#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "libft.h"

static void	test_string_utils(void)
{
	char	*dup;
	char	*sub;
	char	*joined;

	assert(ft_strdup(NULL) == NULL);
	dup = ft_strdup("minishell");
	assert(dup != NULL && strcmp(dup, "minishell") == 0);
	assert(ft_strlen(dup) == 9);
	sub = ft_substr(dup, 4, 5);
	assert(sub != NULL && strcmp(sub, "shell") == 0);
	joined = ft_strjoin("mini", sub);
	assert(joined != NULL && strcmp(joined, "minishell") == 0);
	assert(ft_strcmp("abc", "abc") == 0);
	assert(ft_strcmp("abc", "abd") < 0);
	free(dup);
	free(sub);
	free(joined);
}

static void	test_itoa_and_split(void)
{
	char	*num;
	char	**parts;

	num = ft_itoa(-42);
	assert(num != NULL && strcmp(num, "-42") == 0);
	free(num);
	num = ft_itoa(0);
	assert(num != NULL && strcmp(num, "0") == 0);
	free(num);
	parts = ft_split("/usr/bin:/bin", ':');
	assert(parts != NULL);
	assert(strcmp(parts[0], "/usr/bin") == 0);
	assert(strcmp(parts[1], "/bin") == 0);
	assert(parts[2] == NULL);
	free(parts[0]);
	free(parts[1]);
	free(parts);
}

static void	test_strbuf_scaling(void)
{
	t_strbuf	sb;
	int			i;

	ft_bzero(&sb, sizeof(t_strbuf));
	assert(sb_reserve(&sb, 0) == 1);
	assert(sb.data != NULL && sb.len == 0 && strcmp(sb.data, "") == 0);
	i = 0;
	while (i < 10000)
	{
		assert(sb_push_char(&sb, (char)('a' + (i % 26))) == 1);
		i++;
	}
	assert(sb.len == 10000 && sb.cap >= 10001);
	assert(sb_push_str(&sb, "_end") == 1);
	assert(sb.len == 10004 && strcmp(sb.data + 10000, "_end") == 0);
	sb_free(&sb);
	assert(sb.data == NULL && sb.len == 0 && sb.cap == 0);
}

int	main(void)
{
	printf("[UNIT] Running libft core tests...\n");
	test_string_utils();
	test_itoa_and_split();
	test_strbuf_scaling();
	printf("[UNIT] All libft unit tests PASSED!\n");
	return (0);
}

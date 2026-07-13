/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 10:25:29 by lucius            #+#    #+#             */
/*   Updated: 2024/11/12 13:40:59 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	fsplit(char **split, size_t j)
{
	while (j--)
	{
		if (split[j])
		{
			free(split[j]);
			split[j] = NULL;
		}
	}
	free(split);
}

static size_t	segcount(char const *s, char c)
{
	size_t	i;
	size_t	segnum;

	i = 0;
	segnum = 0;
	while (s[i])
	{
		while (s[i] == c)
			i++;
		if (s[i])
			segnum++;
		while (s[i] && s[i] != c)
			i++;
	}
	return (segnum);
}

static char	**spliter(char **split, char const *s, char c, size_t start)
{
	size_t	i;
	size_t	j;

	i = 0;
	j = 0;
	while (s[i])
	{
		while (s[i] == c)
			i++;
		start = i;
		while (s[i] && s[i] != c)
			i++;
		if (i > start)
		{
			split[j] = ft_substr(s, start, i - start);
			if (!split[j])
			{
				fsplit(split, j);
				return (NULL);
			}
			j++;
		}
	}
	split[j] = NULL;
	return (split);
}

char	**ft_split(char const *s, char c)
{
	size_t	segnum;
	char	**splited;

	if (!s)
		return (NULL);
	segnum = segcount(s, c);
	splited = (char **)malloc((segnum + 1) * sizeof(char *));
	if (!splited)
		return (NULL);
	if (!spliter(splited, s, c, 0))
		return (NULL);
	return (splited);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/19 10:24:58 by luluzuri          #+#    #+#             */
/*   Updated: 2024/11/26 10:46:18 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	copynclean_cache(char **line, char **cache)
{
	int		i;
	char	*tmp;

	if (!(*cache) || !(*cache)[0])
	{
		(free(*cache), *cache = NULL);
		*line = NULL;
		return ;
	}
	i = 0;
	while ((*cache)[i] && (*cache)[i] != '\n')
		i++;
	*line = gnl_substr(*cache, 0, i + ((*cache)[i] == '\n'));
	tmp = gnl_substr(*cache, i + ((*cache)[i] == '\n'), -1);
	if (tmp && !*tmp)
	{
		free(tmp);
		tmp = NULL;
	}
	free(*cache);
	*cache = tmp;
}

static int	found_newline(char *cache)
{
	int	i;

	if (!cache)
		return (0);
	i = 0;
	while (cache[i])
	{
		if (cache[i] == '\n')
			return (1);
		i++;
	}
	return (0);
}

static void	fill_cache(int fd, char **cache)
{
	char	*buffer;
	int		nbytes;
	int		i;

	nbytes = 1;
	buffer = (char *)malloc((BUFFER_SIZE + 1) * sizeof(char));
	if (!buffer)
		return ;
	while (!found_newline(*cache) && nbytes != 0)
	{
		i = 0;
		while (i < (BUFFER_SIZE + 1))
			buffer[i++] = 0;
		nbytes = read(fd, buffer, BUFFER_SIZE);
		if (nbytes < 0)
		{
			(free(*cache), *cache = NULL);
			break ;
		}
		if (*cache)
			*cache = gnl_strjoin(*cache, buffer);
		else
			*cache = gnl_strdup(buffer);
	}
	free(buffer);
}

char	*get_next_line(int fd)
{
	static char	*cache = NULL;
	char		*line;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	fill_cache(fd, &cache);
	if (!cache)
		return (NULL);
	copynclean_cache(&line, &cache);
	return (line);
}

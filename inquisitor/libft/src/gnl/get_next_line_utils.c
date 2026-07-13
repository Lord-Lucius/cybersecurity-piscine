/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/19 10:35:20 by luluzuri          #+#    #+#             */
/*   Updated: 2024/11/26 10:47:07 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	*gnl_memmove(void *dest, const void *src, size_t n)
{
	size_t	i;

	i = 0;
	if (!dest && !src)
		return (NULL);
	if (dest > src)
	{
		while (n--)
			((unsigned char *)dest)[n] = ((const unsigned char *)src)[n];
	}
	else
	{
		while (i < n)
		{
			((unsigned char *)dest)[i] = ((const unsigned char *)src)[i];
			i++;
		}
	}
	return (dest);
}

static size_t	gnl_strlen(const char *s)
{
	const char	*move = s;

	while (*move)
		move++;
	return (move - s);
}

char	*gnl_substr(const char *s, unsigned int start, size_t len)
{
	char	*sub;
	size_t	slen;
	size_t	i;

	slen = gnl_strlen(s);
	i = 0;
	if (start >= slen)
		return (gnl_strdup(""));
	if (len > slen - start)
		len = slen - start;
	sub = (char *)malloc((len + 1) * sizeof(char));
	if (!sub)
		return (sub);
	while (i < len)
	{
		sub[i] = s[start + i];
		i++;
	}
	sub[i] = '\0';
	return (sub);
}

char	*gnl_strdup(const char *s)
{
	char	*sub;
	int		slen;
	int		i;

	if (!s)
		return (NULL);
	slen = gnl_strlen(s);
	sub = (char *)malloc((slen + 1) * sizeof(char));
	if (!sub)
		return (sub);
	i = 0;
	while (i < slen)
	{
		sub[i] = s[i];
		i++;
	}
	sub[i] = '\0';
	return (sub);
}

char	*gnl_strjoin(char const *s1, char const *s2)
{
	size_t	flen;
	size_t	slen;
	char	*sub;

	if (!s1)
		return (gnl_strdup(s2));
	if (!s2)
		return (gnl_strdup(s1));
	flen = gnl_strlen(s1);
	slen = gnl_strlen(s2);
	sub = (char *)malloc((slen + flen + 1) * sizeof(char));
	if (!sub)
		return (sub);
	gnl_memmove(sub, s1, flen);
	gnl_memmove(sub + flen, s2, slen);
	sub[flen + slen] = '\0';
	free((void *)s1);
	return (sub);
}

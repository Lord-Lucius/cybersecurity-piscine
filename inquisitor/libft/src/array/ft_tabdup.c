/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_tabdup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 10:00:00 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/13 10:00:00 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	**ft_tabdup(char **tab)
{
	char	**copy;
	size_t	len;
	size_t	i;

	if (!tab)
		return (NULL);
	len = ft_tablen(tab);
	copy = (char **)malloc((len + 1) * sizeof(char *));
	if (!copy)
		return (NULL);
	i = 0;
	while (i < len)
	{
		copy[i] = ft_strdup(tab[i]);
		if (!copy[i])
			return (ft_free_split(copy), NULL);
		i++;
	}
	copy[i] = NULL;
	return (copy);
}

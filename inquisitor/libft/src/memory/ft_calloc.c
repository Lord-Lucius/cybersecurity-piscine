/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/10 14:10:01 by lucius            #+#    #+#             */
/*   Updated: 2024/11/12 13:40:06 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t elnum, size_t elsize)
{
	void	*ptr;

	if (elsize != 0 && elnum > (size_t)-1 / elsize)
		return (NULL);
	ptr = malloc(elnum * elsize);
	if (!ptr)
		return (ptr);
	ft_bzero(ptr, elnum * elsize);
	return (ptr);
}

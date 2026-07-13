/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/10 16:27:26 by lucius            #+#    #+#             */
/*   Updated: 2024/11/26 10:41:01 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strjoin(char const *s1, char const *s2)
{
	size_t	flen;
	size_t	slen;
	char	*sub;

	flen = ft_strlen(s1);
	slen = ft_strlen(s2);
	sub = (char *)malloc((slen + flen + 1) * sizeof(char));
	if (!sub)
		return (sub);
	ft_memmove(sub, s1, flen);
	ft_memmove(sub + flen, s2, slen);
	sub[flen + slen] = '\0';
	return (sub);
}

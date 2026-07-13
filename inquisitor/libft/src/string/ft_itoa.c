/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/11 20:29:48 by lucius            #+#    #+#             */
/*   Updated: 2024/11/12 15:28:05 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	dcount(long n)
{
	size_t	count;

	count = 0;
	if (n <= 0)
		count++;
	while (n)
	{
		count++;
		n /= 10;
	}
	return (count);
}

static void	int_to_str(char *result, long n, size_t count)
{
	result[count] = '\0';
	if (n == 0)
	{
		result[0] = '0';
		return ;
	}
	if (n < 0)
	{
		result[0] = '-';
		n = -n;
	}
	while (n)
	{
		result[--count] = (n % 10) + '0';
		n /= 10;
	}
}

char	*ft_itoa(int n)
{
	long	tmp;
	size_t	count;
	char	*result;

	tmp = n;
	count = dcount(tmp);
	result = (char *)malloc((count + 1) * sizeof(char));
	if (!result)
		return (NULL);
	int_to_str(result, tmp, count);
	return (result);
}

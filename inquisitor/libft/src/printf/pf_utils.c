/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pf_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 19:54:47 by luluzuri          #+#    #+#             */
/*   Updated: 2024/11/25 14:06:30 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	pf_putchar(char c)
{
	return (write(1, &c, 1));
}

int	pf_putstr(char *str)
{
	int	count;

	count = 0;
	if (str == NULL)
	{
		write(1, "(null)", 6);
		return (6);
	}
	while (*str)
	{
		count += pf_putchar(*str);
		str++;
	}
	return (count);
}

int	pf_putnbr(long n)
{
	int	count;

	count = 0;
	if (n == -2147483648)
	{
		write(1, "-2147483648", 11);
		return (11);
	}
	if (n < 0)
	{
		count += pf_putchar('-');
		n = -n;
	}
	if (n > 9)
		count += pf_putnbr(n / 10);
	count += pf_putchar((n % 10) + 48);
	return (count);
}

void	pf_puthex(char c, int *count, unsigned long n)
{
	int		i;
	int		tmp;
	char	result[256];

	i = 0;
	if (n == 0)
		*count += write(1, "0", 1);
	while (n != 0)
	{
		tmp = n % 16;
		if (tmp < 10)
			tmp = tmp + 48;
		else
		{
			if (c == 'x')
				tmp = tmp + 87;
			else
				tmp = tmp + 55;
		}
		result[i++] = tmp;
		n = n / 16;
	}
	while (i-- > 0)
		*count += pf_putchar(result[i]);
}

void	pf_putadr(void *adr, int *count)
{
	if (adr == 0)
	{
		*count += write(1, "(nil)", 5);
		return ;
	}
	*count += write(1, "0x", 2);
	pf_puthex('x', count, (unsigned long)adr);
}

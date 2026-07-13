/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/23 14:54:54 by luluzuri          #+#    #+#             */
/*   Updated: 2024/12/16 08:19:05 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	handle_format(const char *str, int *count, va_list args, int i)
{
	if (str[i + 1] == 'c')
		*count += pf_putchar(va_arg(args, unsigned int));
	else if (str[i + 1] == 's')
		*count += pf_putstr(va_arg(args, char *));
	else if (str[i + 1] == 'p')
		pf_putadr(va_arg(args, void *), count);
	else if (str[i + 1] == 'd' || str[i + 1] == 'i')
		*count += pf_putnbr(va_arg(args, int));
	else if (str[i + 1] == 'u')
		*count += pf_putnbr(va_arg(args, unsigned int));
	else if (str[i + 1] == 'x' || str[i + 1] == 'X')
		pf_puthex(str[i + 1], count, va_arg(args, unsigned int));
	else if (str[i + 1] == '%')
		*count += pf_putchar('%');
	return (i + 1);
}

static void	parse_arguments(const char *str, int *char_count, va_list args)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (str[i] == '%')
			i = handle_format(str, char_count, args, i);
		else
			*char_count += pf_putchar(str[i]);
		i++;
	}
}

int	ft_printf(const char *format, ...)
{
	int		count_char;
	va_list	args;

	count_char = 0;
	if (!format)
		return (-1);
	va_start(args, format);
	parse_arguments(format, &count_char, args);
	va_end(args);
	return (count_char);
}

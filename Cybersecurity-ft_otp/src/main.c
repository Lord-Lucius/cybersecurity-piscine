/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 19:54:58 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/17 21:07:01 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>

#include "ft_otp.h"

int main(int ac, char **av) {
	(void)av;
	if (ac != 3) error("Should get atleast one option (-g/-k) and it's related argument.");
	return (0);
}

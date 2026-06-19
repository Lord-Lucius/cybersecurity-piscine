/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 19:54:58 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/19 07:24:04 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ft_otp.h"

int main(int ac, char **av) {
	params_t params;
	memset(&params, 0, sizeof(params));

	if (ac != 3)
		error("Should get one option (-g/-k) and its related argument.");

	parse_option(&params, av);

	if (params.g_flag) {
		check_key(&params);
		store_key(&params);
		free(params.key);
	} else if (params.k_flag) {
		load_key(&params);
		printf("%06d\n", generate_otp(&params));
		free(params.key);
	}

	return (0);
}

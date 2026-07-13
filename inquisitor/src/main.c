/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:02:46 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/12 21:42:53 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>

#include "parsing.h"
#include "inquisitor.h"
#include "utils.h"

int main(int ac, char **av) {
	t_config config;
	parse_arguments(ac, av, &config);

	/* to delete */
	config.ip_local = "192.168.0.4";
	config.mac_local = "02:42:c0:a8:00:04";
	/* to delete */

	print_config(&config);

	return 0;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:02:46 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/13 22:53:12 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>

#include "parsing.h"
#include "inquisitor.h"

int main(int ac, char **av) {
	t_config config = {0};
	parse_arguments(ac, av, &config);

	/* to delete */
	config.mac_local = "02:42:c0:a8:00:04";
	/* to delete */

	print_config(&config);

	return 0;
}

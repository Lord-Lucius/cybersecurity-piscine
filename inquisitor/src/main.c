/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:02:46 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/11 08:55:36 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>

#include "parsing.h"
#include "inquisitor.h"

int main(int ac, char **av) {
	t_config config;
	parse_arguments(ac, av, &config);

	return 0;
}

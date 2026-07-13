/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 15:39:34 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/13 15:51:44 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void usage(char *msg) {
	if (msg) fprintf(stderr, "Error: %s\n", msg);
	fprintf(stderr, "Usage: ./inquisitor <IP-src> <MAC-src> <IP-target> "
					"<MAC-target> [-v]\n");
	exit(1);
}

void error(char *msg, int error_code, t_config *config) {
	free_ressources(config);
	fprintf(stderr, "Error: %s\n", msg);
	exit(error_code);
}

void free_ressources(t_config *config) {
	if (!config) return;
	free(config->ip_local);
	free(config->mac_local);
	config->ip_local = NULL;
	config->mac_local = NULL;
}

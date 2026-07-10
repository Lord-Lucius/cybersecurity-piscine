/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 13:06:59 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/10 13:08:15 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "util.h"

void usage(char *msg) {
	printf("%s\n", msg);
	exit(0);
}

void error(char *msg, int error_code) {
	printf("%s\n", msg);
	exit(error_code);
}

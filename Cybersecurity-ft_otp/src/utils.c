/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 20:45:02 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/18 22:58:47 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>

#include "ft_otp.h"

void error(char *msg) {
	char error_header[] = "ERROR: ";
	char *final_msg = (char *)malloc((strlen(error_header) + strlen(msg) + 2) * sizeof(char));
	sprintf(final_msg, "%s%s\n", error_header, msg);
	(void)!write(2, final_msg, strlen(final_msg));
	free(final_msg);
	exit(1);
}

void debug_hexdump(const char *label, const void *data, size_t len)
{
	const unsigned char *bytes = data;

	fprintf(stderr, "[DEBUG] %-12s (%zu octets): ", label, len);
	for (size_t i = 0; i < len; i++)
	{
		fprintf(stderr, "%02x", bytes[i]);
		if ((i + 1) % 4 == 0 && i + 1 < len)
			fprintf(stderr, " ");
	}
	fprintf(stderr, "\n");
}

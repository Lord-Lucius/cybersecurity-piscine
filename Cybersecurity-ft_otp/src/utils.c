/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 20:45:02 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/17 23:56:17 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "ft_otp.h"

void error(char *msg) {
	char error_header[] = "ERROR: ";
	char *final_msg = (char *)malloc((strlen(error_header) + strlen(msg) + 2) * sizeof(char));
	sprintf(final_msg, "%s%s\n", error_header, msg);
	(void)!write(2, final_msg, strlen(final_msg));
	free(final_msg);
	exit(1);
}

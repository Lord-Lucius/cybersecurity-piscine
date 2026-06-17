/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 20:27:18 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/17 23:56:48 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ft_otp.h"

void parse_option(params_t *params, char **av) {
	char *option = av[1];
	char *key_file = av[2];

	memset(params, 0, sizeof(params_t));
	if (option[0] != '-') error("first argument should be the option (-g/-k)");

	if (option[1] == 'g')
		params->g_flag = 1;
	else if (option[1] == 'k')
		params->k_flag = 1;
	else
		error("unrecognized option (-g/-k)");
	params->filename = key_file;
}

static char* concat(const char *s1, const char *s2)
{
	const size_t len1 = (s1 != NULL) ? strlen(s1) : 0;
	const size_t len2 = strlen(s2);
	char *result = malloc(len1 + len2 + 1);

	if (!result) error("malloc failed in concat");

	memcpy(result, s1, len1);
	memcpy(result + len1, s2, len2 + 1);
	result[len1 + len2] = '\0';
	return result;
}

// static void parse_hex(


void check_key(params_t params) {
	FILE *fptr;

	char data[64];
	char *rebuild_str = NULL;
	int count_char = 0;
	fptr = fopen(params.filename, "r");
	if (!fptr) error("could not open the file");

	while (fgets(data, 64, fptr)) {
		count_char += (int)strlen(data);
		rebuild_str = concat(rebuild_str, data);
	}

	if (DEBUG_FLAG) printf("character count: %ld\n", strlen(rebuild_str));

	if (count_char < 64) {
		fclose(fptr);
		char tmp[64];
		sprintf(tmp, "key needs minimum 64 character (%d)", count_char);
		error(tmp);
	}

	free(rebuild_str);
	fclose(fptr);
}

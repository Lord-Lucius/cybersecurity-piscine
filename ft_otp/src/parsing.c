/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 20:27:18 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/03 16:05:00 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ft_otp.h"

void parse_option(params_t *params, char **av) {
	char *option = av[1];
	char *key_file = av[2];

	if (option[0] != '-')
		error("first argument should be the option (-g/-k)");

	if (option[1] == 'g')
		params->g_flag = 1;
	else if (option[1] == 'k')
		params->k_flag = 1;
	else
		error("unrecognized option (-g/-k)");

	params->filename = key_file;
}

static char *concat(const char *s1, const char *s2) {
	const size_t len1 = (s1 != NULL) ? strlen(s1) : 0;
	const size_t len2 = strlen(s2);
	char *result = malloc(len1 + len2 + 1);

	if (!result)
		error("malloc failed in concat");
	if (len1 > 0)
		memcpy(result, s1, len1);
	memcpy(result + len1, s2, len2 + 1);
	return result;
}

void check_key(params_t *params) {
	FILE *fptr;
	char data[64];
	char *rebuild_str = NULL;
	size_t count_char;

	fptr = fopen(params->filename, "r");
	if (!fptr)
		error("could not open the file");
	while (fgets(data, 64, fptr)) {
		char *tmp = rebuild_str;
		rebuild_str = concat(tmp, data);
		free(tmp);
	}
	fclose(fptr);

	if (!rebuild_str)
		error("the key file is empty");

	count_char = strlen(rebuild_str);
	while (count_char > 0 && (rebuild_str[count_char - 1] == '\n' ||
							  rebuild_str[count_char - 1] == '\r'))
		rebuild_str[--count_char] = '\0';

	const char *msg = NULL;
	if (count_char < 64)
		msg = "key must be at least 64 hexadecimal characters";
	else if ((count_char % 2) != 0)
		msg = "key length must be even";
	if (msg != NULL) {
		free(rebuild_str);
		error((char *)msg);
	}
	for (size_t i = 0; i < count_char; i++) {
		if (isxdigit((unsigned char)rebuild_str[i]) == 0) {
			free(rebuild_str);
			error("the string is not hexadecimal");
		}
	}

	params->key = malloc(count_char);
	if (!params->key) {
		free(rebuild_str);
		error("malloc failed in check_key");
	}
	memcpy(params->key, rebuild_str, count_char);
	params->key_size = count_char;

	free(rebuild_str);
}

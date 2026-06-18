/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   totp.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:26:52 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/18 23:03:44 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/hmac.h>

#include "ft_otp.h"

static int hex_char_to_val(char c) {
	int ret = 0;

	if (c >= '0' && c <= '9')
		ret = c - '0';
	else if (c >= 'a' && c <= 'f')
		ret = c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		ret = c - 'A' + 10;
	return (ret);
}

void decode_hex(params_t *params) {
	params->decoded_key = (unsigned char *)malloc(params->decoded_key_size * sizeof(unsigned char));
	int strong = 0;
	int weak = 0;
	for (size_t i = 0; i < params->decoded_key_size; i++) {
		strong = hex_char_to_val(params->key[2*i]);
		weak = hex_char_to_val(params->key[2*i+1]);
		params->decoded_key[i] = (strong << 4) | weak;
	}
}

void encode_counter(params_t *params) {
	uint64_t t = time(NULL) / 30;
	for (int i = 0; i < 8; i++) {
		params->counter[i] = (t >> ((7 - i) * 8)) & 0xFF;
	}
}

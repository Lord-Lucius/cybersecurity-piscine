/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   totp.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 16:26:52 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/19 07:44:18 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include "ft_otp.h"

static int hex_char_to_val(char c) {
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return (0);
}

static void decode_hex(params_t *params) {
	params->decoded_key_size = params->key_size / 2;
	params->decoded_key = malloc(params->decoded_key_size);
	if (params->decoded_key == NULL)
		error("malloc failed in decode_hex");

	for (size_t i = 0; i < params->decoded_key_size; i++) {
		int strong = hex_char_to_val(params->key[2 * i]);
		int weak = hex_char_to_val(params->key[2 * i + 1]);
		params->decoded_key[i] = (unsigned char)((strong << 4) | weak);
	}
}

static void encode_message(params_t *params) {
	uint64_t t = (uint64_t)time(NULL) / 30;

	for (int i = 0; i < 8; i++)
		params->message[i] = (t >> ((7 - i) * 8)) & 0xFF;
}

static void compute_hmac(params_t *params) {
	unsigned char *res = HMAC(EVP_sha1(), params->decoded_key,
							  (int)params->decoded_key_size, params->message, 8,
							  params->hmac, &params->true_hmac_size);
	if (res == NULL)
		error("hmac failed");
}

static uint32_t truncate_hmac(const unsigned char *hmac) {
	int offset = hmac[19] & 0x0F;
	uint32_t bin = ((uint32_t)hmac[offset] << 24)
				 | ((uint32_t)hmac[offset + 1] << 16)
				 | ((uint32_t)hmac[offset + 2] << 8)
				 | (uint32_t)hmac[offset + 3];

	return (bin & 0x7FFFFFFF);
}

int generate_otp(params_t *params) {
	decode_hex(params);
	encode_message(params);
	compute_hmac(params);

	if (DEBUG_FLAG) {
		debug_hexdump("key", params->decoded_key, params->decoded_key_size);
		debug_hexdump("message", params->message, 8);
		debug_hexdump("hmac", params->hmac, (size_t)params->true_hmac_size);
	}

	uint32_t bin = truncate_hmac(params->hmac);

	free(params->decoded_key);
	params->decoded_key = NULL;

	return (int)(bin % 1000000);
}

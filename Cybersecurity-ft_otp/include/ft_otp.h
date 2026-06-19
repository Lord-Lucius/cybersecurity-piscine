/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_otp.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 20:27:42 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/19 20:08:33 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_OTP_H
#define FT_OTP_H

#define DEBUG_FLAG 0

#include <stddef.h>

typedef struct params_s {
	int g_flag;
	int k_flag;
	char *filename;
	unsigned char *key;					// malloc
	unsigned char *decoded_key; // malloc
	unsigned char message[8];
	unsigned char hmac[20];
	size_t key_size;
	size_t decoded_key_size;
	unsigned int true_hmac_size;
} params_t;

/* parsing.c */
void parse_option(params_t *params, char **av);
void check_key(params_t *params);

/* storage.c */
void store_key(params_t *params);
void load_key(params_t *params);

/* totp.c */
int generate_otp(params_t *params);

/* utils.c */
_Noreturn void error(char *msg);
void debug_hexdump(const char *label, const void *data, size_t len);

#endif

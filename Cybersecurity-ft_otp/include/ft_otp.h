/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_otp.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 20:27:42 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/18 22:59:59 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_OTP
# define FT_OTP

#define DEBUG_FLAG 1

#include <stdio.h>

typedef struct params_s {
	int		g_flag;
	int		k_flag;
	char	*filename;
	char *key; // malloc
	unsigned char *decoded_key; // malloc
	unsigned char counter[64];
	size_t	key_size;
	size_t	decoded_key_size;
}			params_t;

void parse_option(params_t *params, char **av);
void check_key(params_t *params);
void decode_hex(params_t *params);
void encode_counter(params_t *params);

/* UTILS */
_Noreturn void error(char *msg);
void debug_hexdump(const char *label, const void *data, size_t len);

#endif

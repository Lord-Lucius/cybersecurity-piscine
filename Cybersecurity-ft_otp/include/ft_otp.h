/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_otp.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 20:27:42 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/17 23:34:08 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_OTP
# define FT_OTP

# define DEBUG_FLAG 1

#include <stdio.h>

typedef struct params_s {
	int g_flag;
	int k_flag;
	char *filename;
} params_t;

void parse_option(params_t *params, char **av);
void check_key(params_t params);

/* UTILS */
void error(char *msg);

#endif

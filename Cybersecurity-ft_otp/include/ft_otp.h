/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_otp.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 20:27:42 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/17 21:18:27 by luluzuri         ###   ########.fr       */
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

int parse_option(char **av);

/* UTILS */
void error(char *msg);

#endif

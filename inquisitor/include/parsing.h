/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 13:52:59 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/28 20:22:38 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_H
#define PARSING_H

#include "inquisitor.h"
#include "libft.h"

void parse_arguments(int ac, char **av, t_config *config);

int is_ipv4(const char *src);
int is_mac_addr(const char *src);
int discover_interface(t_config *config);

void print_config(t_config *config);

#endif

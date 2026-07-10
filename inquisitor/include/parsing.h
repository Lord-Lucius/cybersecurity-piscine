/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 13:52:59 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/10 14:01:59 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_H
#define PARSING_H

#include "inquisitor.h"

int is_ipv4(char *src);
int is_mac_addr(char *src);
int discover_interface(t_config *config);

void print_config(t_config *config);

#endif

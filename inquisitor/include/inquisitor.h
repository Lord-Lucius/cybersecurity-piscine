/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inquisitor.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:03:02 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/09 21:00:43 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INQUISITOR_H
#define INQUISITOR_H

typedef struct s_config {
	char *spoof_ip;
	char *spoof_mac;

	char *target_ip;
	char *target_mac;

	char *local_ip;
	char *local_mac;

	int ifindex;

	char iface;
	int verbose;
} t_config;

#endif

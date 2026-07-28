/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inquisitor.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:03:02 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/28 20:30:53 by luluzuri         ###   ########.fr       */
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
} t_config;

#endif

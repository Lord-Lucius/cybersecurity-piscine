/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inquisitor.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:03:02 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/13 22:26:48 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INQUISITOR_H
#define INQUISITOR_H

typedef struct s_config {
	char *ip_src;
	char *mac_src;
	char *ip_dest;
	char *mac_dest;

	char *ip_local;
	char *mac_local;
	int ifindex;
} t_config;

#endif

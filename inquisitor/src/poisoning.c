/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   poisoning.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 15:06:51 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/28 16:09:02 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "poisoning.h"
#include <netinet/in.h>

void build_arp_trame(t_arp_frame *frame, char *sender_mac_str, char *sender_ip_str, char *target_mac_str, char *target_ip_str, char *local_mac_str) {
	frame->eth.mac_dst = ntohl(target_ip_str);
}

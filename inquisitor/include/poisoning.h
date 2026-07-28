/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   poisoning.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 17:52:07 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/28 15:09:35 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdint.h>

#ifndef POISONNING_H
#define POISONNING_H

typedef struct s_eth_header {
	unsigned char mac_dst[6];
	unsigned char mac_src[6];
	unsigned short ether_type;
} __attribute__((__packed__)) t_eth_header;

typedef struct s_arp_msg {
	unsigned short hardware_type;
	unsigned short protocol_type;
	unsigned short opcde;
	unsigned char hardware_addr_len;
	unsigned char protocol_addr_len;
	unsigned char sender_mac[6];
	unsigned char sender_ip[4];
	unsigned char target_mac[6];
	unsigned char target_ip[4];
} __attribute__((__packed__)) t_arp_msg;

typedef struct s_arp_frame {
	t_eth_header eth;
	t_arp_msg arp;
} __attribute__((__packed__)) t_arp_frame;

#endif

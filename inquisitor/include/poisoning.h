/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   poisoning.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 17:52:07 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/29 15:25:47 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POISONNING_H
#define POISONNING_H

#include <stdint.h>
#include "inquisitor.h"

typedef struct s_eth_header {
	unsigned char dst_mac[6];
	unsigned char src_mac[6];
	unsigned short ether_type;
} __attribute__((packed)) t_eth_header;

typedef struct s_arp_msg {
	unsigned short hardware_type;
	unsigned short protocol_type;
	unsigned char hardware_addr_len;
	unsigned char protocol_addr_len;

	unsigned short opcode;

	unsigned char sender_mac[6];
	unsigned char sender_ip[4];

	unsigned char target_mac[6];
	unsigned char target_ip[4];
} __attribute__((packed)) t_arp_msg;

typedef struct s_arp_frame {
	t_eth_header eth;
	t_arp_msg arp;
} __attribute__((__packed__)) t_arp_frame;

int get_hex_from_mac_addr(unsigned char *dest, const char *addr_str);
void build_arp_trame(t_arp_frame *frame, t_config config);
int open_inject_socket(t_config config);
int send_arp_frame(int fd, t_arp_frame *frame, const t_config config);

void restore_arp(int fd, t_config config);

#endif

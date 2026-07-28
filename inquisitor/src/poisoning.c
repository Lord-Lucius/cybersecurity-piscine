/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   poisoning.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 15:06:51 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/28 18:32:32 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "poisoning.h"
#include "inquisitor.h"
#include "utils.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ether.h>

int get_hex_from_mac_addr(unsigned char *dest, const char *addr_str)
{
	for (int i = 0; i < 6; i++)
	{
		if (sscanf(addr_str, "%hhx", &dest[i]) != 1)
			return 1;
		addr_str += 3;
	}
	return 0;
}

void build_arp_trame(t_arp_frame *frame, t_config config) {

	if (get_hex_from_mac_addr(frame->eth.mac_dst, config.mac_dest) != 0)
		error("destination mac address resolution failed", 1, &config);
	if (get_hex_from_mac_addr(frame->eth.mac_src, config.mac_local) != 0)
		error("destination mac address resolution failed", 1, &config);
	frame->eth.ether_type = htons(0x0806);

/*

    // ARP message
    frame.arp.hw_type  ← htons(1)
    frame.arp.proto    ← htons(0x0800)
    frame.arp.hw_len   ← 6
    frame.arp.proto_len← 4
    frame.arp.opcode   ← htons(2)                  // reply
    frame.arp.sender_mac ← bytes de local_mac_str  // mensonge : c'est toi
    frame.arp.sender_ip  ← bytes de sender_ip_str  // IP usurpée
    frame.arp.target_mac ← bytes de target_mac_str
    frame.arp.target_ip  ← bytes de target_ip_str

*/

	frame->arp.hardware_type = htons(1);
	frame->arp.protocol_type = htons(0x0800);
	frame->arp.hardware_addr_len = 6;
	frame->arp.protocol_addr_len = 4;
	frame->arp.opcde = htons(2);
	
}

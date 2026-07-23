/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   poisoning.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/16 17:52:07 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/23 19:43:48 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdint>
#include <stdio.h>

typedef struct s_eth_header {
	unsigned char mac_dst[6];
	unsigned char mac_src[6];
	unsigned short ether_type;
} __attribute__((__packed__)) s_eth_header;

typedef struct s_arp_msg {
	unsigned short hardware_type;
	unsigned short protocol_type;
	unsigned char hardware_addr_len;
	unsigned char protocol_addr_len;
} t_arp_msg;

typedef struct s_arp_frame {

} __attribute__((__packed__)) t_arp_frame;

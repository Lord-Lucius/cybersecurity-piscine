/*
** dump_frame.c — builds one ARP frame from argv and prints its 42 bytes as hex.
** Used by test_poisoning.py to verify frame field offsets without network I/O.
**
** Usage: ./dump_frame <spoof_ip> <spoof_mac> <target_ip> <target_mac>
*/

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include "poisoning.h"
#include "inquisitor.h"

/* local copy, renamed to avoid clashing with poisoning.h's declaration */
static int mac_to_bytes(unsigned char *dest, const char *addr_str) {
	for (int i = 0; i < 6; i++) {
		if (sscanf(addr_str, "%hhx", &dest[i]) != 1) return 1;
		addr_str += 3;
	}
	return 0;
}

int main(int ac, char **av) {
	if (ac != 5) {
		fprintf(stderr, "usage: ./dump_frame <spoof_ip> <spoof_mac>"
						" <target_ip> <target_mac>\n");
		return 1;
	}

	t_config config = {0};
	config.spoof_ip   = av[1];
	config.spoof_mac  = av[2];
	config.target_ip  = av[3];
	config.target_mac = av[4];
	config.local_ip   = "192.168.0.4";
	config.local_mac  = "02:42:c0:a8:00:04";
	config.ifindex    = 0;

	t_arp_frame frame;
	memset(&frame, 0, sizeof(frame));

	mac_to_bytes(frame.eth.dst_mac,  config.target_mac);
	mac_to_bytes(frame.eth.src_mac,  config.local_mac);
	frame.eth.ether_type = htons(0x0806);

	frame.arp.hardware_type    = htons(1);
	frame.arp.protocol_type    = htons(0x0800);
	frame.arp.hardware_addr_len = 6;
	frame.arp.protocol_addr_len = 4;
	frame.arp.opcode           = htons(2);

	mac_to_bytes(frame.arp.sender_mac, config.local_mac);
	inet_pton(AF_INET, config.spoof_ip,   frame.arp.sender_ip);
	mac_to_bytes(frame.arp.target_mac, config.target_mac);
	inet_pton(AF_INET, config.target_ip, frame.arp.target_ip);

	unsigned char *raw = (unsigned char *)&frame;
	for (size_t i = 0; i < sizeof(frame); i++)
		printf("%02x", raw[i]);
	printf("\n");
	return 0;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sniffing.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/08 15:50:39 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/11 15:35:28 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "sniffing.h"
#include "utils.h"
#include <pcap/pcap.h>

int start_sniffer(t_sniffer *s, t_config *config) {
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle = pcap_open_live(&config->iface, 65535, 1, 1000, errbuf);

	if (handle == NULL)
		error("pcap_open_live failed", 1, config);
	if (pcap_datalink(s->handle) != DLT_EN10MB) {
		pcap_close(s->handle);
		error("interface non-Ethernet", 1, config);
	}

	struct bpf_program fp;
	if (pcap_compile(handle, &fp, "tcp port 21", 1, PCAP_NETMASK_UNKNOWN) == -1) {
		pcap_close(s->handle);
		error(pcap_geterr(s->handle), 1, config);
	}
	if (pcap_setfilter(s->handle, &fp) == -1) {
		pcap_close(s->handle);
		pcap_freecode(&fp);
		error(pcap_geterr(s->handle), 1, config);
	}

	s->handle = handle;
	s->verbose = config->verbose;

	if (pthread_create(&s->thread, NULL, capture_loop, s) != 0) {
		pcap_close(s->handle);
		error("pthread failed", 1, config);
	}

	return 0;
}

void  *capture_loop(void *arg) {
	t_sniffer *s = (t_sniffer *)arg;
	pcap_loop(s->handle, -1, ftp_handler, (u_char *)s);
	return NULL;
}

void  ftp_handler(unsigned char *user, const struct pcap_pkthdr *header, const unsigned char *packet) {
	t_sniffer *s = (t_sniffer *)user;
	unsigned int clen = header->caplen;

	if (clen < 14 + 20 + 20)
		return;
	struct ether_header *eth = packet;
	if (stohs(eth->ether_type) != ETHERTYPE_IP)
		return;

	struct ip *ip = (packet + 14);
	unsigned int ip_len = ip->ip_hl * 4;
}
// void  stop_sniffer(t_sniffer *s);

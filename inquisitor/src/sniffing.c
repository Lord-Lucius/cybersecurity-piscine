/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sniffing.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/08 15:50:39 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/11 22:18:30 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "sniffing.h"
#include "utils.h"
#include <net/ethernet.h>
#include <netinet/in.h>
#include <pcap/pcap.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string.h>
#include <strings.h>

int start_sniffer(t_sniffer *s, t_config *config) {
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle = pcap_open_live(config->iface, 65535, 1, 1000, errbuf);

	if (handle == NULL)
		error("pcap_open_live failed", 1, config);
	if (pcap_datalink(handle) != DLT_EN10MB) {
		pcap_close(handle);
		error("interface non-Ethernet", 1, config);
	}

	struct bpf_program fp;
	if (pcap_compile(handle, &fp, "tcp port 21", 1, PCAP_NETMASK_UNKNOWN) == -1) {
		char *msg = pcap_geterr(handle);
		pcap_close(handle);
		error(msg, 1, config);
	}
	if (pcap_setfilter(handle, &fp) == -1) {
		char *msg = pcap_geterr(handle);
		pcap_close(handle);
		pcap_freecode(&fp);
		error(msg, 1, config);
	}

	s->handle = handle;
	s->verbose = config->verbose;

	if (pthread_create(&s->thread, NULL, capture_loop, s) != 0) {
		pcap_freecode(&fp);
		pcap_close(s->handle);
		error("pthread failed", 1, config);
	}

	pcap_freecode(&fp);
	return 0;
}

void  *capture_loop(void *arg) {
	t_sniffer *s = (t_sniffer *)arg;
	pcap_loop(s->handle, -1, ftp_handler, (u_char *)s);
	return NULL;
}

void ftp_handler(u_char *user, const struct pcap_pkthdr *header, const u_char *packet) {
	t_sniffer *sniffer = (t_sniffer *)user;
	uint len = header->caplen;

	if (len < 14 + 20 + 20) // Eth + min Ip + min tcp
		return ;

	struct ether_header *eth = (struct ether_header *)packet;
	if (ntohs(eth->ether_type) != ETHERTYPE_IP)
		return;

	struct ip *ip_struct = (struct ip*)(packet + 14);
	uint ip_len = ip_struct->ip_hl * 4; // word of 4 bytes
	if (ip_len < 20 || ip_struct->ip_p != IPPROTO_TCP)
		return;

	struct tcphdr *tcp = (struct tcphdr*)(packet + 14 + ip_len);
	uint tcp_len = tcp->doff * 4; // word of 4 bytes
	if (tcp_len < 20)
		return ;
	uint headers = 14 + ip_len + tcp_len;
	if (len <= headers)
		return;

	u_char *payload = (u_char*)(packet + headers);
	uint payload_len = len - headers;

	u_char *cur = payload;
	u_char *end = payload + payload_len;

	while (cur < end) {
		u_char *nl = (u_char*)memchr(cur, '\n', end - cur);
		u_char *line_end = nl ? nl : end;
		uint line_len = line_end - cur;
		if (line_len > 0 && line_end[-1] == '\r')
			line_len--;
		if (line_len == 0) {
			cur = line_end + 1;
			continue;
		}

		if (sniffer->verbose || (line_len >= 5 && (strncasecmp((char *)cur, "STOR ", 5) == 0 || strncasecmp((char *)cur, "RETR ", 5) == 0)))
			printf("[FTP] %.*s\n", (int)line_len, cur);
		if (!nl)
			break;
		cur = nl + 1;
	}
}
void  stop_sniffer(t_sniffer *s) {
	if (!s->handle)
		return;
	pcap_breakloop(s->handle);
	pthread_join(s->thread, NULL);
	pcap_close(s->handle);
	s->handle = NULL;
}

#include <stdio.h>
#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
	struct ip *ip_header = (struct ip *)(packet + 14); // 14 octets pour l'en-tête Ethernet

	// On calcule la taille de l'en-tête IP
	int ip_header_len = ip_header->ip_hl * 4;

	// Position de l'en-tête TCP
	struct tcphdr *tcp_header = (struct tcphdr *)(packet + 14 + ip_header_len);
	int tcp_header_len = tcp_header->th_off * 4;

	// Position et taille de la charge utile (Payload / Données FTP)
	const u_char *payload = packet + 14 + ip_header_len + tcp_header_len;
	int payload_len = header->len - (14 + ip_header_len + tcp_header_len);

	if (payload_len > 0) {
		printf("--- Données FTP Capturées ---\n");
		for (int i = 0; i < payload_len; i++) {
			// Affiche les caractères imprimables (commandes USER, PASS, etc.)
			if (payload[i] >= 32 && payload[i] <= 126) {
				putchar(payload[i]);
			} else if (payload[i] == '\n' || payload[i] == '\r') {
				putchar(payload[i]);
			}
		}
		printf("\n");
	}
}

int main() {
	char error_buffer[PCAP_ERRBUF_SIZE];
	pcap_t *handle;
	struct bpf_program filter;
	char filter_exp[] = "tcp port 21"; // Filtre uniquement le trafic FTP

	// Ouverture de l'interface réseau en mode promiscuous (ex: "eth0" ou "wlan0")
	handle = pcap_open_live("eth0", BUFSIZ, 1, 1000, error_buffer);
	if (handle == NULL) {
		fprintf(stderr, "Erreur d'ouverture : %s\n", error_buffer);
		return 1;
	}

	// Compilation et application du filtre de paquets
	if (pcap_compile(handle, &filter, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1 ||
		pcap_setfilter(handle, &filter) == -1) {
		fprintf(stderr, "Erreur d'application du filtre\n");
		return 1;
	}

	// Boucle de capture continue
	pcap_loop(handle, 0, packet_handler, NULL);

	pcap_close(handle);
	return 0;
}

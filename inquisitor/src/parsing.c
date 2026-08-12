/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 20:51:40 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/12 11:11:09 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ether.h>
#include "parsing.h"
#include "inquisitor.h"
#include "libft.h"
#include "utils.h"

#define CYAN "\033[0;36m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"

void print_config(t_config *config) {
	printf(CYAN "┌─────────────────────────────────────┐\n" RESET);
	printf(CYAN "│          inquisitor — config        │\n" RESET);
	printf(CYAN "├──────────────┬──────────────────────┤\n" RESET);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET " %-20s " CYAN "│\n" RESET,
		   "spoof_ip", config->spoof_ip);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET " %-20s " CYAN "│\n" RESET,
		   "spoof_mac", config->spoof_mac);
	printf(CYAN "├──────────────┼──────────────────────┤\n" RESET);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET " %-20s " CYAN "│\n" RESET,
		   "target_ip", config->target_ip);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET " %-20s " CYAN "│\n" RESET,
		   "target_mac", config->target_mac);
	printf(CYAN "├──────────────┼──────────────────────┤\n" RESET);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET YELLOW " %-20s " RESET CYAN
				"│\n" RESET,
		   "local_ip", config->local_ip);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET YELLOW " %-20s " RESET CYAN
				"│\n" RESET,
		   "local_mac", config->local_mac);
	printf(CYAN "└──────────────┴──────────────────────┘\n" RESET);
}

static int ft_parse_octet(const char *nptr, int *err) {
	int result;

	*err = 0;
	result = 0;
	if (!nptr || !*nptr) return (*err = 1, 0);
	while (*nptr) {
		if (*nptr < '0' || *nptr > '9') return (*err = 1, 0);
		result = (result * 10) + (*nptr - '0');
		if (result > 255) return (*err = 1, 0);
		nptr++;
	}
	return (result);
}

static int ft_parse_hex_octet(const char *nptr, int *err) {
	int result;

	*err = 0;
	result = 0;
	if (!nptr || !*nptr) return (*err = 1, 0);
	while (*nptr) {
		if (*nptr >= '0' && *nptr <= '9')
			result = (result * 16) + (*nptr - '0');
		else if (*nptr >= 'a' && *nptr <= 'f')
			result = (result * 16) + (*nptr - 'a' + 10);
		else if (*nptr >= 'A' && *nptr <= 'F')
			result = (result * 16) + (*nptr - 'A' + 10);
		else
			return (*err = 1, 0);
		if (result > 255) return (*err = 1, 0);
		nptr++;
	}
	return (result);
}

int is_ipv4(const char *src) {
	char **split_src = NULL;
	size_t split_src_len = 0;

	split_src = ft_split(src, '.');
	split_src_len = ft_tablen(split_src);
	if (ft_tablen(split_src) != 4) {
		ft_free_split(split_src);
		return (1);
	}
	for (size_t i = 0; i < split_src_len; i++) {
		int err;

		if (ft_strlen(split_src[i]) < 1 || ft_strlen(split_src[i]) > 3) {
			ft_free_split(split_src);
			return (1);
		}
		int converted_value = ft_parse_octet(split_src[i], &err);
		if (err) {
			ft_free_split(split_src);
			return (1);
		}
		if (converted_value < 0 || converted_value > 255) {
			ft_free_split(split_src);
			return (1);
		}
	}
	ft_free_split(split_src);
	return (0);
}

int is_mac_addr(const char *src) {
	char **split_src = NULL;
	size_t split_src_len = 0;

	split_src = ft_split(src, ':');
	split_src_len = ft_tablen(split_src);
	if (split_src_len != 6) {
		ft_free_split(split_src);
		return (1);
	}
	for (size_t i = 0; i < split_src_len; i++) {
		int err;

		if (ft_strlen(split_src[i]) != 2) {
			ft_free_split(split_src);
			return (1);
		}
		int converted_value = ft_parse_hex_octet(split_src[i], &err);
		if (err) {
			ft_free_split(split_src);
			return (1);
		}
		if (converted_value < 0 || converted_value > 255) {
			ft_free_split(split_src);
			return (1);
		}
	}
	ft_free_split(split_src);
	return (0);
}

int discover_interface(t_config *config) {
	struct ifaddrs *interface = NULL;
	struct ifaddrs *tmp = NULL;
	int socket_fd;
	struct ifreq ifr = {0};

	if (getifaddrs(&interface) == -1) return (-1);
	tmp = interface;
	while (tmp != NULL) {
		if (tmp->ifa_flags & IFF_LOOPBACK) {
			tmp = tmp->ifa_next;
			continue;
		}
		if (tmp->ifa_addr != NULL && tmp->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *addr = (struct sockaddr_in *)tmp->ifa_addr;

			config->local_ip = ft_strdup(inet_ntoa(addr->sin_addr));
			socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
			if (socket_fd == -1) {
				freeifaddrs(interface);
				return (-1);
			}
			ft_strlcpy(ifr.ifr_name, tmp->ifa_name, IFNAMSIZ);
			if (ioctl(socket_fd, SIOCGIFHWADDR, &ifr) == -1) {
				close(socket_fd);
				freeifaddrs(interface);
				return (-1);
			}
			unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
			char mac_str[18];

			snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
					 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			config->local_mac = ft_strdup(mac_str);
			if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) == -1) {
				close(socket_fd);
				freeifaddrs(interface);
				return (-1);
			}
			config->ifindex = ifr.ifr_ifindex;
			ft_strlcpy(config->iface, tmp->ifa_name, IFNAMSIZ);
			close(socket_fd);
			break;
		}
		tmp = tmp->ifa_next;
	}
	freeifaddrs(interface);
	if (!config->local_ip || !config->local_mac) return (-1);
	return (0);
}

void parse_arguments(int ac, char **av, t_config *config) {

	config->verbose = 0;
	if (ac == 6 && ft_strcmp(av[5], "-v") == 0) {
		config->verbose = 1;
		ac = 5;
	}

	if (ac != 5) error("invalid number of arguments", 1, config);
	config->spoof_ip = av[1];
	config->spoof_mac = av[2];
	config->target_ip = av[3];
	config->target_mac = av[4];
	if (is_ipv4(config->spoof_ip) != 0) error("ip source invalid", 1, config);
	if (is_ipv4(config->target_ip) != 0) error("ip dest invalid", 1, config);
	if (is_mac_addr(config->spoof_mac) != 0)
		error("mac source invalid", 1, config);
	if (is_mac_addr(config->target_mac) != 0)
		error("mac dest invalid", 1, config);
	if (discover_interface(config))
		error("interface discovering failed", 1, config);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 20:51:40 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/13 23:13:04 by luluzuri         ###   ########.fr       */
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
	printf(CYAN "│         inquisitor — config         │\n" RESET);
	printf(CYAN "├──────────────┬──────────────────────┤\n" RESET);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET " %-20s " CYAN "│\n" RESET,
		   "ip-src", config->ip_src);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET " %-20s " CYAN "│\n" RESET,
		   "mac-src", config->mac_src);
	printf(CYAN "├──────────────┼──────────────────────┤\n" RESET);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET " %-20s " CYAN "│\n" RESET,
		   "ip-dest", config->ip_dest);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET " %-20s " CYAN "│\n" RESET,
		   "mac-dest", config->mac_dest);
	printf(CYAN "├──────────────┼──────────────────────┤\n" RESET);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET YELLOW " %-20s " RESET CYAN
				"│\n" RESET,
		   "ip-local", config->ip_local);
	printf(CYAN "│" RESET " %-12s " CYAN "│" RESET YELLOW " %-20s " RESET CYAN
				"│\n" RESET,
		   "mac-local", config->mac_local);
	printf(CYAN "└──────────────┴──────────────────────┘\n" RESET);
}

static int ft_parse_octet(const char *nptr, int *err) {
	int result;

	*err = 0;
	result = 0;
	if (!nptr || !*nptr) return (*err = 1, 0);
	while (*nptr) {
		if (*nptr < '0' || *nptr > '9') return (*err = 1, 0);
		result = result * 10 + (*nptr - '0');
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
			result = result * 16 + (*nptr - '0');
		else if (*nptr >= 'a' && *nptr <= 'f')
			result = result * 16 + (*nptr - 'a' + 10);
		else if (*nptr >= 'A' && *nptr <= 'F')
			result = result * 16 + (*nptr - 'A' + 10);
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
		return 1;
	}
	for (size_t i = 0; i < split_src_len; i++) {
		int err;
		if (ft_strlen(split_src[i]) < 1 || ft_strlen(split_src[i]) > 3) {
			ft_free_split(split_src);
			return 1;
		}
		int converted_value = ft_parse_octet(split_src[i], &err);
		if (err) {
			ft_free_split(split_src);
			return 1;
		}
		if (converted_value < 0 || converted_value > 255) {
			ft_free_split(split_src);
			return 1;
		}
	}
	ft_free_split(split_src);
	return 0;
}

int is_mac_addr(const char *src) {
	char **split_src = NULL;
	size_t split_src_len = 0;

	split_src = ft_split(src, ':');
	split_src_len = ft_tablen(split_src);
	if (split_src_len != 6) {
		ft_free_split(split_src);
		return 1;
	}
	for (size_t i = 0; i < split_src_len; i++) {
		int err;
		if (ft_strlen(split_src[i]) != 2) {
			ft_free_split(split_src);
			return 1;
		}
		int converted_value = ft_parse_hex_octet(split_src[i], &err);
		if (err) {
			ft_free_split(split_src);
			return 1;
		}
		if (converted_value < 0 || converted_value > 255) {
			ft_free_split(split_src);
			return 1;
		}
	}
	ft_free_split(split_src);
	return 0;
}

int discover_interface(t_config *config) {
	struct ifaddrs *interface = NULL;
	struct ifaddrs *tmp = NULL;
	int socket_fd;
	struct ifreq ifr = {0};

	if (getifaddrs(&interface) == -1) return -1;
	tmp = interface;
	while (tmp != NULL) {
		if (tmp->ifa_flags & IFF_LOOPBACK) {
			tmp = tmp->ifa_next;
			continue;
		}
		if (tmp->ifa_addr != NULL && tmp->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in *addr = (struct sockaddr_in *)tmp->ifa_addr;
			config->ip_local = ft_strdup(inet_ntoa(addr->sin_addr));
			socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
			if (socket_fd == -1) {
				freeifaddrs(interface);
				return -1;
			}
			ft_strlcpy(ifr.ifr_name, tmp->ifa_name, IFNAMSIZ);
			if (ioctl(socket_fd, SIOCGIFHWADDR, &ifr) == -1) {
				close(socket_fd);
				freeifaddrs(interface);
				return -1;
			}
			config->mac_local = ft_strdup(
				ether_ntoa((struct ether_addr *)ifr.ifr_hwaddr.sa_data));
			if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) == -1) {
				close(socket_fd);
				freeifaddrs(interface);
				return -1;
			}
			config->ifindex = ifr.ifr_ifindex;
			close(socket_fd);
			break;
		}
		tmp = tmp->ifa_next;
	}
	freeifaddrs(interface);
	if (!config->ip_local || !config->mac_local) return -1;
	return 0;
}

void parse_arguments(int ac, char **av, t_config *config) {
	if (ac != 5) error("invalid number of arguments", 1, config);
	config->ip_src = av[1];
	config->mac_src = av[2];
	config->ip_dest = av[3];
	config->mac_dest = av[4];
	if (is_ipv4(config->ip_src) != 0) error("ip source invalid", 1, config);
	if (is_ipv4(config->ip_dest) != 0) error("ip dest invalid", 1, config);
	if (is_mac_addr(config->mac_src) != 0)
		error("mac source invalid", 1, config);
	if (is_mac_addr(config->mac_dest) != 0)
		error("mac dest invalid", 1, config);

	if (discover_interface(config))
		error("interface discovering failed", 1, config);
}

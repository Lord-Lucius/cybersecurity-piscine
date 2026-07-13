/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 20:51:40 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/13 16:12:26 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

int ft_parse_octet (const char *nptr, int *err) {
	int result;

	*err = 0;
	result = 0;
	if (!nptr || !*nptr)
		return (*err = 1, 0);
	while (*nptr)
	{
		if (*nptr < '0' || *nptr > '9')
			return (*err = 1, 0);
		result = result * 10 + (*nptr - '0');
		if (result > 255)
			return (*err = 1, 0);
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
		if (converted_value < 0 || converted_value > 255){
			ft_free_split(split_src);
			return 1;
		}
	}
	ft_free_split(split_src);
	return 0;
}

int is_mac_addr(const char *src) {
	(void)src;
	return 0;
}

int discover_interface(t_config *config) {
	(void)config;
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

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 15:02:48 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/29 15:24:54 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "signals.h"
#include "utils.h"

volatile sig_atomic_t g_running = 1;

void signalHandler(int sig) {
	(void)sig;
	g_running = 0;
}

void setup_signals(void) {
	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) != 0)
		error("sigaction failed", 1, NULL);
}

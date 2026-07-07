/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 09:40:18 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/07 20:44:41 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"
#include <iostream>

void utils::usage(char *av) {
	std::cout << "Usage: " << std::endl;
	std::cout << "\t./" << av << " <option> [arguments]" << std::endl;
	exit(0);
}

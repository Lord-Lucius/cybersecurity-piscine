/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 09:04:14 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/08 12:59:45 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Cipher.hpp"
#include "Discovering.hpp"
#include "Parser.hpp"

int main(int ac, char **av) {
	if (ac < 2) {
		std::cerr << "AssertionError: need atleast 1 argument and max 2 "
					 "(option - key)."
				  << std::endl;
		exit(1);
	}
	Parser p(ac, av);
	Discovering d(p.getOptionfield());
	Cipher c(d.getFinalPaths(), p.getKey(), p.getOptionfield());
	return 0;
}

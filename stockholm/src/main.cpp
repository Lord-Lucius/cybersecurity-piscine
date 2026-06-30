/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 09:04:14 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/30 09:35:44 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Parser.hpp"

int main(int ac, char **av) {
	if (ac < 2 || ac > 3) {
		std::cerr << "AssertionError: need atleast 1 argument and max 2 (option - key)." << std::endl;
		exit (1);
	}
	std::cout << "Hello stockholm" << std::endl;
	Parser *p = Parser().parse(ac, av);
	std::cout << p->getKey() << " :: " << p->getOptionCode() << std::endl;
	return 0;
}

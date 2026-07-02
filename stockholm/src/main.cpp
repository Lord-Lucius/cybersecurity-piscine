/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 09:04:14 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/02 22:09:32 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Parser.hpp"

int main(int ac, char **av) {
	if (ac < 2) {
		std::cerr << "AssertionError: need atleast 1 argument and max 2 (option - key)." << std::endl;
		exit (1);
	}
	std::cout << "Hello stockholm" << std::endl;
	Parser *p = Parser().parse(ac, av);
	std::cout << p->getKey() << " :: " << p->getOptionfield() << std::endl;
	return 0;
}

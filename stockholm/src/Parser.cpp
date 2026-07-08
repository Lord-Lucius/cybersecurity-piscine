/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 09:23:38 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/08 13:12:14 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstddef>
#include <iostream>
#include <algorithm>

#include "Parser.hpp"
#include "Utils.hpp"

Parser::Parser(int ac, char **av) {
	this->option_field = 0;
	if (cmdOptionExists(av, av + ac, "--help") ||
		cmdOptionExists(av, av + ac, "-h")) {
		utils::usage(av[0]);
		exit(0);
	}
	if (cmdOptionExists(av, av + ac, "--version") ||
		cmdOptionExists(av, av + ac, "-v")) {
		std::cout << "program version: " << PROGRAM_VERSION << std::endl;
		exit(0);
	}
	if (cmdOptionExists(av, av + ac, "--silent") ||
		cmdOptionExists(av, av + ac, "-s")) {
		this->option_field |= OPT_SILENT;
	}
	char *raw = nullptr;
	if (cmdOptionExists(av, av + ac, "--reverse") ||
		cmdOptionExists(av, av + ac, "-r")) {
		this->option_field |= OPT_REVERSE;
		raw = (cmdOptionExists(av, av + ac, "-r"))
				  ? getCmdOption(av, av + ac, "-r")
				  : getCmdOption(av, av + ac, "--reverse");
		if (raw == nullptr) {
			std::cerr << "Error: --reverse/-r need a key" << std::endl;
			exit(1);
		}
		this->key = raw;
	} else {
		char *found = nullptr;
		for (int i = 1; i < ac; i++) {
			if (av[i][0] != '-') {
				found = av[i];
				break;
			}
		}
		if (found == nullptr) {
			std::cerr << "Error: encryption key required" << std::endl;
			exit(1);
		}
		this->key = found;
	}
	if (this->key.length() < 16) {
		std::cerr << "Error: key must be at least 16 characters" << std::endl;
		exit(1);
	}
}

int Parser::getOptionfield(void) const {
	return this->option_field;
}

std::string Parser::getKey(void) const {
	return this->key;
}

char *Parser::getCmdOption(char **begin, char **end,
						   const std::string &option) {
	char **itr = std::find(begin, end, option);

	if (itr != end && ++itr != end) {
		return *itr;
	}
	return 0;
}

bool Parser::cmdOptionExists(char **begin, char **end,
							 const std::string &option) {
	return std::find(begin, end, option) != end;
}

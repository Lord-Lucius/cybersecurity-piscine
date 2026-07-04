/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 09:23:38 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/04 10:56:38 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
List of option accepted:
	* --help / -h show help
	* --version / -v show version
	* --reverse / -r <key> inverse infection
	* --silent / -s silent mod ( silence the print of the file being affected )
*/

#include <cstddef>
#include <iostream>
#include <algorithm>

#include "Parser.hpp"
#include "Utils.hpp"

void Parser::parse(int ac, char **av) {
	this->key = nullptr;
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
	if (cmdOptionExists(av, av + ac, "--reverse") ||
		cmdOptionExists(av, av + ac, "-r")) {
		this->option_field |= OPT_REVERSE;
		this->key = (cmdOptionExists(av, av + ac, "-r"))
						? getCmdOption(av, av + ac, "-r")
						: getCmdOption(av, av + ac, "--reverse");
		if (this->key == NULL) {
			std::cerr << "Error: --reverse/-r need a key" << std::endl;
			exit(1);
		}
	}
}

int Parser::getOptionfield(void) const {
	return this->option_field;
}

std::string Parser::getKey(void) const {
	if (this->key == nullptr)
		return "";
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

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 09:23:38 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/30 18:08:34 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
List of option accepted:
	* --help / -h show help
	* --version / -v show version
	* --reverse / -r <key> inverse infection
	* --silent / -s silent mod ( silence the print of the file being affected )
*/

#include <iostream>
#include <algorithm>

#include "Parser.hpp"
#include "Utils.hpp"

Parser *Parser::parse(int ac, char **av) {
	(void)ac;
	std::string key;
	std::string option = av[1];

	return this;
}

int Parser::getOptionCode(void) const {
	return this->option_code;
}

std::string Parser::getKey(void) const {
	return this->key;
}

char *Parser::getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool Parser::cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

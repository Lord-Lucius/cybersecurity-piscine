/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 12:02:18 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/06 12:02:23 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Utils.hpp"
#include <string>

class Parser {
private:
	int option_field;
	std::string key;
	static char *getCmdOption(char **begin, char **end,
							const std::string &option);
	static bool cmdOptionExists(char **begin, char **end,
								const std::string &option);
public:
	Parser(int ac, char **av);
	int getOptionfield(void) const;
	std::string getKey(void) const;
};

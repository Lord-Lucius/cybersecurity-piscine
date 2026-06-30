/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 09:06:17 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/30 09:56:44 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

class Parser {
	private:
		int option_code;
		std::string key;

		char *getCmdOption(char ** begin, char ** end, const std::string & option);
		bool cmdOptionExists(char** begin, char** end, const std::string& option);

	public:
		Parser *parse(int ac, char **av);
		int getOptionCode(void);
		std::string getKey(void);
};

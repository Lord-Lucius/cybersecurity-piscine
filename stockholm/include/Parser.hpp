/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 09:06:17 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/30 18:07:55 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

#define NORMAL_MOD_CODE 1
#define REVERSE_MOF_CODE 2
#define SILENT_MOD_CODE 3

class Parser {
	private:
		int option_code;
		std::string key;

		static char *getCmdOption(char ** begin, char ** end, const std::string & option);
		static bool cmdOptionExists(char** begin, char** end, const std::string& option);

	public:
		Parser *parse(int ac, char **av);
		int getOptionCode(void) const;
		std::string getKey(void) const;
};

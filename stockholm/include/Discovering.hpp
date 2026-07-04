/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Discovering.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 11:08:53 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/04 11:26:14 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

class Discovering {
	private:
		const std::string home = "HOME";
		const std::string infection_folder_name = "/infection";
		char *getenv_result;
		std::string complete_path;

	public:
		void discover();
};

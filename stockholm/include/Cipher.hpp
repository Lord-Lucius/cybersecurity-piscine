/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cipher.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 15:14:58 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/07 13:08:47 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <filesystem>
#include <iostream>
#include <vector>

class Cipher {
	private:
		const std::string new_ext = ".ft";
		void encryptFile(std::filesystem::directory_entry &path, std::string &key);
		void decryptFile(std::filesystem::directory_entry &path, std::string &key);

	public:
		Cipher(std::vector<std::filesystem::directory_entry> &final_paths_vector, std::string &key, int &option_field);
};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cipher.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 15:14:58 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/08 13:01:19 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <filesystem>
#include <iostream>
#include <vector>

class Cipher {
	private:
		const std::string new_ext = ".ft";
		void encryptFile(const std::filesystem::directory_entry &path, const std::string &key);
		static void decryptFile(const std::filesystem::directory_entry &path, const std::string &key);

	public:
		Cipher(const std::vector<std::filesystem::directory_entry> &final_paths_vector, const std::string &key, const int &option_field);
};

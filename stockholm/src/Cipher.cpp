/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cipher.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 15:22:04 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/07 21:29:39 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cipher.hpp"
#include "Utils.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <vector>

namespace fs = std::filesystem;

Cipher::Cipher(std::vector<fs::directory_entry> &final_paths_vector,
			   std::string &key, int &option_field) {
	for (fs::directory_entry &path : final_paths_vector) {
		if ((option_field & OPT_SILENT) == 0) {
			std::cout << path.path().filename() << std::endl;
		}
		if ((option_field & OPT_REVERSE) != 0) {
			decryptFile(path, key);
		} else {
			encryptFile(path, key);
		}
	}
}

void Cipher::encryptFile(std::filesystem::directory_entry &path, std::string &key) {
	EVP_CIPHER_CTX *ctx;
	ctx = EVP_CIPHER_CTX_new();
	if (ctx == nullptr) {
		std::cerr << "Error while creating cipher context" << std::endl;
		exit(1);
	}

	unsigned char iv[16];
	unsigned char final_key[32];
	RAND_bytes(iv, 16);
	EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), NULL,
				   (unsigned char *)key.c_str(), (int)key.length(), 1,
				   final_key, NULL);
	if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, final_key, iv) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		std::cerr << "Error: Encrypt init failed" << std::endl;
		exit(1);
	}

	std::ifstream ifile(path.path().string(), std::ios::binary);
	if (!ifile) {
		EVP_CIPHER_CTX_free(ctx);
		std::cerr << "Error: failed to open file (" << path.path().string() << ")" << std::endl;
		exit(1);
	}
	ifile.seekg(0, std::ios_base::end);
	std::vector<unsigned char> bytes(ifile.tellg());
	std::vector<unsigned char> encrypted_bytes(bytes.size() + 15);
	int out_buffer_size;

	ifile.seekg(0);
	ifile.read(reinterpret_cast<char *>(bytes.data()), (int)bytes.size());

	if (EVP_EncryptUpdate(ctx, encrypted_bytes.data(), &out_buffer_size, bytes.data(), (int)bytes.size()) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		ifile.close();
		std::cerr << "Error: EncryptUpdate failed" << std::endl;
		exit(1);
	}

	unsigned char encrypt_final_padding[16];
	int final_padding_lenght;
	if (EVP_EncryptFinal_ex(ctx, encrypt_final_padding, &final_padding_lenght) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		ifile.close();
		std::cerr << "Error: EncryptFinal failed" << std::endl;
		exit(1);
	}
	ifile.close();

	std::ofstream ofile(path.path().string(), std::ios::binary);
	if (!ofile) {
		EVP_CIPHER_CTX_free(ctx);
		std::cerr << "Error: failed to open file (" << path.path().string() << ")" << std::endl;
		exit(1);
	}
	ofile.write((const char *)iv, 16);
	ofile.write((const char *)encrypted_bytes.data(), out_buffer_size);
	ofile.write((const char *)encrypt_final_padding, final_padding_lenght);
	ofile.close();
	std::filesystem::rename(path.path(), path.path().string() + this->new_ext);
	EVP_CIPHER_CTX_free(ctx);
}

void Cipher::decryptFile(std::filesystem::directory_entry &path,
						 std::string &key) {}

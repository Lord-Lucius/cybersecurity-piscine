/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cipher.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/06 15:22:04 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/08 13:01:11 by luluzuri         ###   ########.fr       */
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

Cipher::Cipher(const std::vector<std::filesystem::directory_entry> &final_paths_vector, const std::string &key, const int &option_field) {
	for (const fs::directory_entry &path : final_paths_vector) {
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

void Cipher::encryptFile(const std::filesystem::directory_entry &path, const std::string &key) {
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

void Cipher::decryptFile(const std::filesystem::directory_entry &path, const std::string &key) {
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (ctx == nullptr) {
		std::cerr << "Error while creating cipher context" << std::endl;
		exit(1);
	}
	unsigned char iv[16];
	unsigned char final_key[32];
	EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), NULL,
				(unsigned char *)key.c_str(), (int)key.length(), 1,
				final_key, NULL);
	std::ifstream ifile(path.path().string(), std::ios::binary);
	if (!ifile) {
		EVP_CIPHER_CTX_free(ctx);
		std::cerr << "Error: failed to open file (" << path.path().string() << ")" << std::endl;
		exit(1);
	}
	ifile.read(reinterpret_cast<char *>(iv), 16);
	ifile.seekg(0, std::ios::end);
	std::streamsize total_size = ifile.tellg();
	std::streamsize encrypted_size = total_size - 16;
	std::vector<unsigned char> encrypted_bytes(encrypted_size);
	ifile.seekg(16);
	ifile.read(reinterpret_cast<char *>(encrypted_bytes.data()), encrypted_size);
	ifile.close();
	if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, final_key, iv) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		std::cerr << "Error: Decrypt init failed" << std::endl;
		exit(1);
	}
	std::vector<unsigned char> decrypted_bytes(encrypted_size);
	int out_buffer_size;
	if (EVP_DecryptUpdate(ctx, decrypted_bytes.data(), &out_buffer_size, encrypted_bytes.data(), (int)encrypted_size) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		std::cerr << "Error: DecryptUpdate failed" << std::endl;
		exit(1);
	}
	unsigned char decrypt_final[16];
	int final_length;
	if (EVP_DecryptFinal_ex(ctx, decrypt_final, &final_length) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		std::cerr << "Error: DecryptFinal failed — wrong key?" << std::endl;
		exit(1);
	}
	std::string new_path = path.path().string().substr(0, path.path().string().size() - 3);
	std::ofstream ofile(new_path, std::ios::binary);
	if (!ofile) {
		EVP_CIPHER_CTX_free(ctx);
		std::cerr << "Error: failed to open file for writing" << std::endl;
		exit(1);
	}
	ofile.write(reinterpret_cast<char *>(decrypted_bytes.data()), out_buffer_size);
	ofile.write(reinterpret_cast<char *>(decrypt_final), final_length);
	ofile.close();
	std::filesystem::remove(path.path());
	EVP_CIPHER_CTX_free(ctx);
}

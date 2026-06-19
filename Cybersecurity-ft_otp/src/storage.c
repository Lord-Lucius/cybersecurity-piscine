/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   storage.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 07:30:00 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/19 20:07:33 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <time.h>

#include "ft_otp.h"

static const unsigned char ENCRYPTION_KEY[32] = {
	0x2e, 0xc4, 0xcc, 0x27, 0x43, 0xc2, 0x4b, 0xc1, 0x52, 0x12, 0x31,
	0x63, 0xd9, 0xaf, 0x48, 0x1c, 0x8a, 0x3f, 0x90, 0x1d, 0x7b, 0x6e,
	0x42, 0xf5, 0x09, 0xab, 0xcd, 0xef, 0x11, 0x22, 0x33, 0x44};
#define IV_SIZE 16
#define BLOC_SIZE 16

void store_key(params_t *params) {
	int len = 0;
	int finale_len = 0;
	unsigned char cipher_text[params->key_size + BLOC_SIZE];

	FILE *fptr = fopen("ft_otp.key", "wb");
	if (!fptr) error("could not create ft_otp.key");

	unsigned char iv[IV_SIZE];
	if (RAND_bytes(iv, IV_SIZE) != 1) error("could not generate IV");
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!ctx) error("EVP cipher context creation failed");
	if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, ENCRYPTION_KEY, iv) !=
		1)
		error("EVP_EncryptInit_ex failed");
	if (EVP_EncryptUpdate(ctx, cipher_text, &len, (unsigned char *)params->key,
						  (int)params->key_size) != 1)
		error("EncryptUpdate failed");
	if (EVP_EncryptFinal_ex(ctx, cipher_text + len, &finale_len) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		error("EVP_EncryptFinal_ex failed");
	}
	int total = len + finale_len;

	fwrite(iv, 1, IV_SIZE, fptr);
	fwrite(cipher_text, 1, total, fptr);

	EVP_CIPHER_CTX_free(ctx);
	fclose(fptr);
	printf("Key was successfully saved in ft_otp.key.\n");
}

void load_key(params_t *params) {
	int len = 0;
	int finale_len = 0;

	FILE *fptr = fopen(params->filename, "rb");
	if (!fptr) error("could not open the key file");
	fseek(fptr, 0, SEEK_END);
	long size = ftell(fptr);
	if (size <= IV_SIZE) {
		fclose(fptr);
		error("invalid key file");
	}
	rewind(fptr);

	unsigned char *buf = malloc((size_t)size);
	if (!buf) {
		fclose(fptr);
		error("malloc failed in load_key");
	}
	size_t n = fread(buf, 1, (size_t)size, fptr);
	fclose(fptr);

	unsigned char iv[IV_SIZE];
	memcpy(iv, buf, IV_SIZE);
	unsigned char *cipher = buf + IV_SIZE;
	int cipher_len = (int)(n - IV_SIZE);

	unsigned char *plain = malloc((size_t)cipher_len + 1);
	if (!plain) {
		free(buf);
		error("malloc failed in load_key");
	}

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		free(buf);
		free(plain);
		error("EVP cipher context creation failed");
	}
	if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, ENCRYPTION_KEY, iv) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		free(buf);
		free(plain);
		error("EVP_DecryptInit_ex failed");
	}
	if (EVP_DecryptUpdate(ctx, plain, &len, cipher, cipher_len) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		free(buf);
		free(plain);
		error("EVP_DecryptUpdate failed");
	}
	if (EVP_DecryptFinal_ex(ctx, plain + len, &finale_len) != 1) {
		EVP_CIPHER_CTX_free(ctx);
		free(buf);
		free(plain);
		error("decryption failed (corrupted key file?)");
	}

	int total = len + finale_len;
	plain[total] = '\0';

	EVP_CIPHER_CTX_free(ctx);
	free(buf);

	params->key = (unsigned char *)plain;
	params->key_size = (size_t)total;
}

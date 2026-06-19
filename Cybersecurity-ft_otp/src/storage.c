/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   storage.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/19 07:30:00 by luluzuri          #+#    #+#             */
/*   Updated: 2026/06/19 17:56:59 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <time.h>

#include "ft_otp.h"

static const unsigned char ENCRYPTION_KEY[32] = {
	0x2e, 0xc4, 0xcc, 0x27, 0x43, 0xc2, 0x4b, 0xc1,
	0x52, 0x12, 0x31, 0x63, 0xd9, 0xaf, 0x48, 0x1c,
	0x8a, 0x3f, 0x90, 0x1d, 0x7b, 0x6e, 0x42, 0xf5,
	0x09, 0xab, 0xcd, 0xef, 0x11, 0x22, 0x33, 0x44
};
# define IV_SIZE 16
# define BLOC_SIZE 16

/* -g : écrit la clé dans ft_otp.key.
   TODO: chiffrer params->key AVANT de l'écrire (exigence du sujet). */
void store_key(params_t *params) {
	int len;
	unsigned char cipher_text[params->key_size + BLOC_SIZE];

	FILE *fptr = fopen("ft_otp.key", "wb");
	if (!fptr)
		error("could not create ft_otp.key");

	/* TODO: remplacer cette écriture en clair par une écriture chiffrée. */
	unsigned char iv[IV_SIZE];
	if (RAND_bytes(iv, IV_SIZE) != 1)
		error("could not generate IV");
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!ctx)
		error("EVP cipher context creation failed");
	if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, ENCRYPTION_KEY, iv) != 1)
		error("EVP_EncryptInit_ex failed");
	if (EVP_EncryptUpdate(ctx, cipher_text, &len, (unsigned char *)params->key, (int)params->key_size) != 1)
		error("EncryptUpdate failed");

	fwrite(params->key, 1, params->key_size, fptr);

	fclose(fptr);
	printf("Key was successfully saved in ft_otp.key.\n");
}

/* -k : lit le fichier (params->filename) et remplit params->key + key_size.
   TODO: déchiffrer le contenu (inverse de store_key). */
void load_key(params_t *params) {
	FILE *fptr = fopen(params->filename, "rb");
	if (!fptr)
		error("could not open the key file");

	fseek(fptr, 0, SEEK_END);
	long size = ftell(fptr);
	if (size <= 0) {
		fclose(fptr);
		error("the key file is empty");
	}
	rewind(fptr);

	char *buf = malloc((size_t)size + 1);
	if (!buf) {
		fclose(fptr);
		error("malloc failed in load_key");
	}

	size_t n = fread(buf, 1, (size_t)size, fptr);
	fclose(fptr);
	buf[n] = '\0';

	while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'))
		buf[--n] = '\0';

	/* TODO: déchiffrer buf ici avant de l'exposer comme clé. */
	params->key = buf;
	params->key_size = n;
}

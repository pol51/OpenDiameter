/**
 * @file aes_wrap.h
 * AES-based functions
 *
 * - AES Key Wrap Algorithm (128-bit KEK) (RFC3394)
 * - One-Key CBC MAC (OMAC1) hash with AES-128
 * - AES-128 CTR mode encryption
 * - AES-128 EAX mode encryption/decryption
 * - AES-128 CBC
 *
 * Copyright (c) 2003-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */
#include "aes.h"
#ifndef AES_WRAP_H
#define AES_WRAP_H

int aes_wrap(const uint8_t * kek, int n, const uint8_t * plain,
	     uint8_t * cipher);
int aes_unwrap(const uint8_t * kek, int n, const uint8_t * cipher,
	       uint8_t * plain);
int omac1_aes_128_vector(const uint8_t * key, size_t num_elem,
			 const uint8_t * addr[], const size_t * len,
			 uint8_t * mac);
int omac1_aes_128(const uint8_t * key, const uint8_t * data, size_t data_len,
		  uint8_t * mac);
int aes_128_encrypt_block(const uint8_t * key, const uint8_t * in,
			  uint8_t * out);
int aes_128_ctr_encrypt(const uint8_t * key, const uint8_t * nonce,
			uint8_t * data, size_t data_len);
int aes_128_eax_encrypt(const uint8_t * key, const uint8_t * nonce,
			size_t nonce_len, const uint8_t * hdr, size_t hdr_len,
			uint8_t * data, size_t data_len, uint8_t * tag);
int aes_128_eax_decrypt(const uint8_t * key, const uint8_t * nonce,
			size_t nonce_len, const uint8_t * hdr, size_t hdr_len,
			uint8_t * data, size_t data_len, const uint8_t * tag);
int aes_128_cbc_encrypt(const uint8_t * key, const uint8_t * iv, uint8_t * data,
			size_t data_len);
int aes_128_cbc_decrypt(const uint8_t * key, const uint8_t * iv, uint8_t * data,
			size_t data_len);

#endif				/* AES_WRAP_H */

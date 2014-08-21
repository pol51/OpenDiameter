/**
 * @file eap_fast_generate_pac.c
 * 
 * @brief provides all of the PAC functionality
 */
#include "eap_fast_generate_pac.h"
#include "aes.h"
#include "aes_wrap.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PAC_OPAQUE_TYPE_PAD 0
#define PAC_OPAQUE_TYPE_KEY 1
#define PAC_OPAQUE_TYPE_LIFETIME 2
#define PAC_OPAQUE_TYPE_IDENTITY 3

/*
 * Binary data format
 * 4-octet magic value: 6A E4 92 0C
 * 2-octet version (big endian)
 * <version specific data>
 *
 * version=0:
 * Sequence of PAC entries:
 *   2-octet PAC-Type (big endian)
 *   32-octet PAC-Key
 *   2-octet PAC-Opaque length (big endian)
 *   <variable len> PAC-Opaque data (length bytes)
 *   2-octet PAC-Info length (big endian)
 *   <variable len> PAC-Info data (length bytes)
 */

#define EAP_FAST_PAC_BINARY_MAGIC 0x6ae4920c
#define EAP_FAST_PAC_BINARY_FORMAT_VERSION 0

static int eap_fast_build_info(struct eap_fast_pac *pac);
static int eap_fast_write_pac(const char *pac_file, char *buf, size_t len);
static int eap_fast_build_opaque(struct eap_fast_pac *pac,
				 uint8_t * pac_opaque_encr_key, int lifetime);

/**
 * eap_fast_write_pac(const char *pac_file, char *buf, size_t len)
 * 
 * @brief Writes pac struct/buffer to file after creating a copy of the buffer
 * @param pac_file
 * @param len
 * @return -1 if error, zero if success
 * @note Later compiled versions of this application appear to have heap mangling
 * due to the pointer manipulation methods of the original author(s)
 */
static int eap_fast_write_pac(const char *pac_file, char *buf, size_t len)
{
	FILE *f = NULL;
	f = fopen(pac_file, "wb+");

	if (f == NULL) {
		printf("\nEAP-FAST: Failed to open PAC "
		       "file '%s' for writing", pac_file);
		return -1;
	}
	// Copy over the buffer to a new temp buffer
	uint8_t *tmpBuffer = malloc(len);
	int i = 0;
	for (i = 0; i < len; i++) {
		tmpBuffer[i] = buf[i];
	}

	printf("\tOpened file: %s\n", pac_file);
	if (fwrite(tmpBuffer, 1, len, f) != len) {
		printf("\nEAP-FAST: Failed to write all "
		       "PACs into '%s'", pac_file);
		fclose(f);
		return (-1);
	}
	printf("\tAttempting to close file\n");

	fclose(f);
	free(tmpBuffer);

	printf("\tClosed file\n");

	return 0;

}

/**
 * eap_fast_save_pac_bin - Save PAC entries (binary format)
 * @sm: Pointer to EAP state machine allocated with eap_peer_sm_init()
 * @pac_root: Root of the PAC list
 * @pac_file: Name of the PAC file/blob
 * Returns: 0 on success, -1 on failure
 */
int eap_fast_save_pac_bin(struct eap_fast_pac *pac_root, const char *pac_file)
{
	size_t len, count = 0;
	struct eap_fast_pac *pac;
	uint8_t *buf, *pos;

	len = 6;
	pac = pac_root;
	while (pac) {
		if (pac->pac_opaque_len > 65535 || pac->pac_info_len > 65535)
			return -1;
		len += 2 + EAP_FAST_PAC_KEY_LEN + 2 + pac->pac_opaque_len +
		    2 + pac->pac_info_len;
		pac = pac->next;
	}

	buf = malloc(len);
	if (buf == NULL) {
		return (-1);
	}

	pos = buf;
	WPA_PUT_BE32(pos, EAP_FAST_PAC_BINARY_MAGIC);
	pos += 4;
	WPA_PUT_BE16(pos, EAP_FAST_PAC_BINARY_FORMAT_VERSION);
	pos += 2;

	pac = pac_root;
	while (pac) {
		WPA_PUT_BE16(pos, pac->pac_type);
		pos += 2;
		memcpy(pos, pac->pac_key, EAP_FAST_PAC_KEY_LEN);
		pos += EAP_FAST_PAC_KEY_LEN;
		WPA_PUT_BE16(pos, pac->pac_opaque_len);
		pos += 2;

		memcpy(pos, pac->pac_opaque, pac->pac_opaque_len);

		if (MSG_DEBUG) {
			hexdump(1, "opaque", pac->pac_opaque,
				pac->pac_opaque_len);
		}

		pos += pac->pac_opaque_len;
		WPA_PUT_BE16(pos, pac->pac_info_len);
		pos += 2;
		memcpy(pos, pac->pac_info, pac->pac_info_len);
		pos += pac->pac_info_len;

		pac = pac->next;
		count++;
	}

	if (eap_fast_write_pac(pac_file, (char *)buf, len) < 0) {
		free(buf);
		return (-1);
	}

	if (MSG_DEBUG) {
		printf("EAP-FAST: Wrote %lu PAC entries into '%s' "
		       "(bin)\n", (unsigned long)count, pac_file);
	}

	return 0;
}

/**
 * eap_fast_build_info(struct eap_fast_pac *pac)
 * 
 * @brief Builds PAC info element
 * @param pac
 * @return -1 if error, 0 for success
 */
static int eap_fast_build_info(struct eap_fast_pac *pac)
{

	pac->pac_info_len = (pac->a_id_len + 4) + (pac->i_id_len + 4) +
	    (pac->a_id_info_len + 4);
	pac->pac_info = malloc(pac->pac_info_len);

	if (pac->pac_info == NULL) {
		printf(" >> Unable to malloc pac_info\n");
		return (-1);
	}
	uint8_t *pos;
	pos = pac->pac_info;
	if (pac->a_id) {
		WPA_PUT_BE16(pos, PAC_TYPE_A_ID);
		pos += 2;
		WPA_PUT_BE16(pos, pac->a_id_len);
		pos += 2;
		memcpy(pos, pac->a_id, pac->a_id_len);
		pos += pac->a_id_len;
	}
	if (pac->i_id) {
		WPA_PUT_BE16(pos, PAC_TYPE_I_ID);
		pos += 2;
		WPA_PUT_BE16(pos, pac->i_id_len);
		pos += 2;
		memcpy(pos, pac->i_id, pac->i_id_len);
		pos += pac->i_id_len;
	}
	if (pac->a_id_info) {
		WPA_PUT_BE16(pos, PAC_TYPE_A_ID_INFO);
		pos += 2;
		WPA_PUT_BE16(pos, pac->a_id_info_len);
		pos += 2;
		memcpy(pos, pac->a_id_info, pac->a_id_info_len);
		pos += pac->a_id_info_len;
	}
	return 0;
}

/**
 * eap_fast_build_opaque(struct eap_fast_pac *pac, uint8_t * pac_opaque_encr_key, int lifetime)
 * 
 * @brief Create the "secret" part of the PAC struct
 * @param pac
 * @param pac_opaque_encr_key
 * @param lifetime
 * @return -1 for error, 0 for success
 * @note I don't know what these magic numbers mean nor can the author of this application...
 */
static int eap_fast_build_opaque(struct eap_fast_pac *pac,
				 uint8_t * pac_opaque_encr_key, int lifetime)
{

	uint8_t *pac_buf = NULL;
	uint8_t *pos = NULL;
	int pac_buf_len = 0;
	struct os_time now;

	pac_buf_len =
	    (2 + EAP_FAST_PAC_KEY_LEN) + (2 + 4) + (2 + pac->a_id_len) + 8;
	pac_buf = malloc(pac_buf_len);
	if (pac_buf == NULL) {
		printf(" >> Unable to malloc pac_buf!\n");
		return (-1);
	}

	pos = pac_buf;

	*pos += PAC_OPAQUE_TYPE_KEY;
	*pos += EAP_FAST_PAC_KEY_LEN;
	os_memcpy(pos, pac->pac_key, EAP_FAST_PAC_KEY_LEN);
	pos += EAP_FAST_PAC_KEY_LEN;

	*pos += PAC_OPAQUE_TYPE_LIFETIME;
	*pos += 4;
	os_get_time(&now);
	WPA_PUT_BE32(pos, now.sec + lifetime);
	pos += 4;

	if (pac->a_id) {
		*pos += PAC_OPAQUE_TYPE_IDENTITY;
		*pos += pac->a_id_len;
		os_memcpy(pos, pac->a_id, pac->a_id_len);
		pos += pac->a_id_len;
	}

	pac_buf_len = pos - pac_buf;
	while (pac_buf_len % 8) {
		*pos += PAC_OPAQUE_TYPE_PAD;
		pac_buf_len++;
	}

	pac->pac_opaque = malloc(pac_buf_len);
	if (pac->pac_opaque == NULL) {
		printf("Unable to malloc pac->pac_opaque");
		os_free(pac_buf);
		printf("Unable to free pac_buf");
		return (-1);
	}

	/*
	 * int aes_wrap(const uint8_t *kek, int n, const uint8_t *plain, uint8_t *cipher)
	 * 
	 * @kek: 16-octet Key encryption key (KEK)
	 * @n: Length of the plaintext key in 64-bit units; e.g., 2 = 128-bit = 16
	 * bytes
	 * @plain: Plaintext key to be wrapped, n * 64 bits
	 * @cipher: Wrapped key, (n + 1) * 64 bits
	 * Returns: 0 on success, -1 on failure
	 */
	if (aes_wrap
	    (pac_opaque_encr_key, pac_buf_len / 8,
	     pac_buf, pac->pac_opaque) < 0) {
		os_free(pac_buf);
		printf(" >> AES_wrap has failed!\n");
		return (-1);
	}

	os_free(pac_buf);

	pac->pac_opaque_len = pac_buf_len + 8;

	return 1;
}

/**
 * eap_fast_build_pac(uint8_t * a_id, uint8_t * i_id, uint8_t * a_id_info, int lifetime, uint8_t * encr_key)
 * 
 * @brief Builds a PAC struct
 * @param a_id
 * @param i_id
 * @param a_id_info
 * @param lifetime
 * @param encr_key
 * @return NULL if error, non-NULL if success
 */
struct eap_fast_pac *eap_fast_build_pac(uint8_t *
					a_id,
					uint8_t *
					i_id,
					uint8_t *
					a_id_info, int lifetime,
					uint8_t * encr_key)
{
	struct eap_fast_pac *pac = NULL;
	struct os_time now;
	/*
	 * Malloc and zero PAC struct
	 */
	pac = (struct eap_fast_pac *)malloc(sizeof(struct eap_fast_pac));
	memset(pac, 0, sizeof(struct eap_fast_pac));
	/*
	 * PAC list may have many PAC objects in the list, but
	 * for the purposes of this test - set to NULL (one PAC)
	 */
	pac->next = NULL;
	/*
	 * Set PAC type to PAC_TYPE_TUNNEL_PAC
	 */
	pac->pac_type = PAC_TYPE_TUNNEL_PAC;
	printf("Set Pac type:%u\n", (unsigned int)pac->pac_type);
	/*
	 * Generate Random Key for PAC structure
	 */
	printf("Generating random PAC key\n");
	if (os_get_random
	    (pac->pac_key, EAP_FAST_PAC_KEY_LEN) < 0 || os_get_time(&now) < 0) {
		printf("return NULL\n");
		return NULL;
	}
	printf("\tGenerated random PAC and time is %d\n", (int)now.sec);
	/*
	 * Fill in General PAC structure
	 */
	printf("Building generic PAC struct with user input\n");
	pac->a_id_len = sizeof(uint8_t) * strlen((const char *)a_id);
	pac->a_id_info_len = sizeof(uint8_t) * strlen((const char *)a_id_info);
	pac->i_id_len = sizeof(uint8_t) * strlen((const char *)i_id);
	pac->a_id = malloc(pac->a_id_len);
	memcpy(pac->a_id, a_id, pac->a_id_len);
	pac->i_id = malloc(pac->a_id_info_len);
	memcpy(pac->i_id, i_id, pac->i_id_len);
	pac->a_id_info = malloc(pac->i_id_len);
	memcpy(pac->a_id_info, a_id_info, pac->a_id_info_len);
	printf("\tBuilt generic PAC struct with user input\n");
	/*
	 * Build PAC EAP FAST Opaque structure
	 */
	printf("Building EAP Fast Opaque structure\n");
	if (eap_fast_build_opaque(pac, encr_key, lifetime) < 0) {
		return NULL;
	}

	printf("\tBuilt EAP Fast Opaque structure\n");
	/*
	 *  Build PAC EAP FAST Info structure
	 */
	printf("Building EAP Fast Info structure\n");
	if (eap_fast_build_info(pac) < 0) {
		return NULL;
	}
	printf("\tBuilt EAP Fast Info structure\n");
	return pac;
}

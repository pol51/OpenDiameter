/**
 * @file eap_fast_generate_pac.h
 */
#include "os.h"
#ifndef EAP_FAST_GENERATE_PAC_H
#define EAP_FAST_GENERATE_PAC_H
/*
 * RFC 5422: Section 4.2.1 - Formats for PAC TLV Attributes / Type Field
 * Note: bit 0x8000 (Mandatory) and bit 0x4000 (Reserved) are also defined
 * in the general PAC TLV format (Section 4.2).
 */
#define PAC_TYPE_PAC_KEY 1
#define PAC_TYPE_PAC_OPAQUE 2
#define PAC_TYPE_CRED_LIFETIME 3
#define PAC_TYPE_A_ID 4
#define PAC_TYPE_I_ID 5
/*
 * 6 was previous assigned for SERVER_PROTECTED_DATA, but
 * draft-cam-winget-eap-fast-provisioning-02.txt changed this to Reserved.
 */
#define PAC_TYPE_A_ID_INFO 7
#define PAC_TYPE_PAC_ACKNOWLEDGEMENT 8
#define PAC_TYPE_PAC_INFO 9
#define PAC_TYPE_PAC_TYPE 10

#define EAP_FAST_PAC_KEY_LEN 32
/* RFC 5422: 4.2.6 PAC-Type TLV */
#define PAC_TYPE_TUNNEL_PAC 1
/* Application Specific Short Lived PACs (only in volatile storage) */
/* User Authorization PAC */
#define PAC_TYPE_USER_AUTHORIZATION 3
/* Application Specific Long Lived PACs */
/* Machine Authentication PAC */
#define PAC_TYPE_MACHINE_AUTHENTICATION 2

#define PAC_ENCR_KEY_LEN	16

struct eap_fast_pac {
	struct eap_fast_pac *next;

	uint8_t pac_key[EAP_FAST_PAC_KEY_LEN];
	uint8_t *pac_opaque;
	size_t pac_opaque_len;
	uint8_t *pac_info;
	size_t pac_info_len;
	uint8_t *a_id;
	size_t a_id_len;
	uint8_t *i_id;
	size_t i_id_len;
	uint8_t *a_id_info;
	size_t a_id_info_len;
	uint16_t pac_type;
};

int eap_fast_save_pac_bin(struct eap_fast_pac *pac_root, const char *pac_file);

struct eap_fast_pac *eap_fast_build_pac(uint8_t * a_id, uint8_t * i_id,
					uint8_t * a_id_info, int lifetime,
					uint8_t * encr_key);
#endif				//EAP_FAST_GENERATE_PAC

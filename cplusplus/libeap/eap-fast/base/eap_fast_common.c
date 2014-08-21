/**
 * @file eap_fast_common.c
 * @brief Additions by Dr. Bing Li@ Concordia University,
 *  Montreal, Canada
 */
#include "base/eap_fast_common.h"
#include "base/openssl_patches.h"
#include <openssl/ssl.h>

static u8 *eap_fast_get_auth_id(u8 * auth_id_data, size_t * auth_id_len);
static void
eap_fast_select_pac(struct FAST_context_peer *ctx,
		    const u8 * a_id, size_t a_id_len);
static int eap_fast_use_pac_opaque(FAST_data * data, struct eap_fast_pac *pac);

static u8 *eap_fast_derive_key(SSL * data, char *label, size_t len);
static u8 eap_fast_tlv_eap_payload(char **buf, size_t * len);

static u8
eap_fast_build_tlv_iresult_crypto_binding(FAST_data * data, u8 inner_eap_code,
					  EAP_tlv_crypto_binding_tlv * rbinding,
					  char **msg, size_t * len);
static u8 eap_fast_build_tlv_iresult(u8 iresult, char **msg, size_t * msg_len);

static int
eap_fast_parse_decrypted(u8 * decrypted, size_t decrypted_len,
			 EAP_fast_tlv_parse * tlv,
			 u8 * resp, size_t * resp_len);
static u8 *eap_fast_tlv_nak(int vendor_id, int tlv_type, size_t * len);
static u8 *eap_fast_tlv_result(int status, int intermediate, size_t * len);
static size_t eap_fast_get_inner_eap_key(u8 * isk, size_t isk_len);
static size_t eap_fast_get_phase2_key(u8 * isk, size_t isk_len);
static int eap_fast_update_icmk(FAST_data * data);
static u8 *eap_fast_build_phase2_res(FAST_data * fast, u8 * credential,
				     int credential_len, int *res_len);
static u8 eap_fast_build_tlv_result(u8 iresult, char **msg, size_t * msg_len);
static int
eap_fast_validate_crypto_binding(size_t flag,
				 EAP_tlv_crypto_binding_tlv * bind);
static u8
eap_fast_process_crypto_binding(FAST_data * data,
				EAP_tlv_crypto_binding_tlv * bind,
				size_t binding_len, u8 iresult,
				u8 tunnel_eap_code, char **resp,
				size_t * resp_len);
static u8 innerEapCode2iresult(u8 inner_eap_code);

static int eap_fast_clear_pac_opaque_ext(FAST_data * data);

void eap_fast_derive_master_secret(const u8 * pac_key, const u8 * server_random,
				   const u8 * client_random, u8 * master_secret)
{
#define TLS_RANDOM_LEN 32
#define TLS_MASTER_SECRET_LEN 48
	u8 seed[2 * TLS_RANDOM_LEN];

	hexdump(MSG_DEBUG, "EAP-FAST: client_random",
		client_random, TLS_RANDOM_LEN);
	hexdump(MSG_DEBUG, "EAP-FAST: server_random",
		server_random, TLS_RANDOM_LEN);

	/*
	 * RFC 4851, Section 5.1:
	 * master_secret = T-PRF(PAC-Key, "PAC to master secret label hash", 
	 *                       server_random + client_random, 48)
	 */
	os_memcpy(seed, server_random, TLS_RANDOM_LEN);
	os_memcpy(seed + TLS_RANDOM_LEN, client_random, TLS_RANDOM_LEN);
	sha1_t_prf(pac_key, EAP_FAST_PAC_KEY_LEN,
		   "PAC to master secret label hash",
		   seed, sizeof(seed), master_secret, TLS_MASTER_SECRET_LEN);

	hexdump_key(MSG_DEBUG, "EAP-FAST: master_secret",
		    master_secret, TLS_MASTER_SECRET_LEN);
}

static u8 *eap_fast_get_auth_id(u8 * auth_id_data, size_t * auth_id_len)
{
	unsigned char *auth_id;
	*auth_id_len = -1;

	if (auth_id_data[0] != 0 || auth_id_data[1] != FAST_DATA_TYPE_AUTH_ID) {
		printf("ERROR: AUTH ID is wrong\n");
		return NULL;
	} else {
		*auth_id_len = (auth_id_data[2] * 256 + auth_id_data[3]);

		auth_id = auth_id_data + FAST_DATA_HEADER_LEN;
		return auth_id;
	}

}

static void eap_fast_select_pac(struct FAST_context_peer *ctx,
				const u8 * a_id, size_t a_id_len)
{
	ctx->pac_curr = eap_fast_get_pac(ctx->pac_root, a_id, a_id_len,
					 PAC_TYPE_TUNNEL_PAC);

	if (ctx->pac_curr == NULL) {
		/*
		 * Tunnel PAC was not available for this A-ID. Try to use
		 * Machine Authentication PAC, if one is available.
		 */
		ctx->pac_curr = eap_fast_get_pac(ctx->pac_root, a_id, a_id_len,
						 PAC_TYPE_MACHINE_AUTHENTICATION);
	}
}

static int eap_fast_use_pac_opaque(FAST_data * data, struct eap_fast_pac *pac)
{
	u8 *tlv = NULL;
	size_t tlv_len, olen;
	EAP_tlv_hdr *ehdr = NULL;

	olen = pac->pac_opaque_len;
	tlv_len = sizeof(*ehdr) + olen;
	tlv = os_malloc(tlv_len);

	if (tlv) {
		ehdr = (EAP_tlv_hdr *) tlv;
		ehdr->tlv_type = host_to_be16(PAC_TYPE_PAC_OPAQUE);
		ehdr->length = host_to_be16(olen);
		os_memcpy(ehdr + 1, pac->pac_opaque, olen);
	}

	if (tlv == NULL ||
	    tls_connection_client_hello_ext(data,
					    TLS_EXT_PAC_OPAQUE,
					    tlv, tlv_len) < 0) {
		printf("EAP-FAST: Failed to add PAC-Opaque TLS " "extension");
		os_free(tlv);
		return -1;
	}

	os_free(tlv);
	return 0;
}

static int eap_fast_clear_pac_opaque_ext(FAST_data * data)
{
	if (tls_connection_client_hello_ext(data,
					    TLS_EXT_PAC_OPAQUE, NULL, 0) < 0) {
		printf("EAP-FAST: Failed to remove PAC-Opaque "
		       "TLS extension");
		return -1;
	}
	return 0;
}

static u8 *eap_fast_derive_key(SSL * data, char *label, size_t len)
{
	TLS_keys keys;
	u8 *rnd = NULL, *out;
	int block_size = 0;

	block_size = tls_connection_get_keyblock_size(data);

	if (block_size < 0) {
		printf("ERROR: block size =-1\n");
		return NULL;
	}

	out = os_malloc(block_size + len);
	if (out == NULL) {
		return NULL;
	}

	if (tls_connection_get_keys(data, &keys)) {
		goto fail;
	}

	rnd = os_malloc(keys.client_random_len + keys.server_random_len);
	if (rnd == NULL)
		goto fail;

	os_memcpy(rnd, keys.server_random, keys.server_random_len);
	os_memcpy(rnd + keys.server_random_len, keys.client_random,
		  keys.client_random_len);

	hexdump_key(MSG_DEBUG, "EAP-FAST: master_secret for key "
		    "expansion", keys.master_key, keys.master_key_len);

	if (tls_prf(keys.master_key, keys.master_key_len,
		    label, rnd, keys.client_random_len +
		    keys.server_random_len, out, block_size + len)) {
		goto fail;
	}

	os_free(rnd);
	os_memmove(out, out + block_size, len);
	return out;

 fail:
	os_free(rnd);
	os_free(out);
	return NULL;
}

static u8 eap_fast_tlv_eap_payload(char **buf, size_t * len)
{
	int err = 1;
	EAP_tlv_hdr *tlv;

	if (buf == NULL || *buf == NULL) {
		printf("EAP-FAST ERROR: inner eap payload is empty\n");
		return err;
	}

	/* Encapsulate EAP packet in EAP-Payload TLV */
	if (MSG_DEBUG) {
		printf("EAP-FAST: Add EAP-Payload TLV\n");
	}

	tlv = malloc(sizeof(*tlv) + *len);
	if (tlv == NULL) {
		printf("EAP-FAST: Failed to "
		       "allocate memory for TLV " "encapsulation\n");
		free(*buf);
		return err;
	}
	tlv->tlv_type = ((be16) (host_to_be16((EAP_TLV_TYPE_MANDATORY |
					       EAP_TLV_EAP_PAYLOAD_TLV))));
	tlv->length = host_to_be16(*len);
	os_memcpy(tlv + 1, *buf, *len);
	free(*buf);
	*len += sizeof(*tlv);
	*buf = (char *)tlv;
	err = 0;

	return err;
}

static u8 eap_fast_build_tlv_eap_payload(char **msg, size_t * len)
{
	hexdump_key(MSG_DEBUG, "EAP-FAST: Phase 2 EAP-Request", (const u8 *)msg,
		    *len);

	return eap_fast_tlv_eap_payload(msg, len);
}

static u8 eap_fast_build_tlv_iresult_crypto_binding(FAST_data * data,
						    u8 inner_eap_code,
						    EAP_tlv_crypto_binding_tlv *
						    rbinding, char **msg,
						    size_t * len)
{
	int err = 1;
	EAP_tlv_result_tlv *tlvs = NULL;
	EAP_tlv_crypto_binding_tlv *binding;
	int type;

	*len = sizeof(*tlvs) + sizeof(*binding);
	tlvs = malloc(*len);
	memset(tlvs, 0, *len);
	if (tlvs == NULL) {
		return err;
	}
	binding = (EAP_tlv_crypto_binding_tlv *) (tlvs + 1);

	type = EAP_TLV_INTERMEDIATE_RESULT_TLV;

	/* Result TLV */
	if (MSG_DEBUG) {
		printf("EAP-FAST: Add Result TLV (status=SUCCESS)");
	}
	tlvs->tlv_type = host_to_be16((EAP_TLV_TYPE_MANDATORY | type));
	tlvs->length = host_to_be16(2);
	if (inner_eap_code == EAP_CODE_SUCCESS) {
		tlvs->status = host_to_be16(EAP_TLV_RESULT_SUCCESS);
	} else {
		tlvs->status = host_to_be16(EAP_TLV_RESULT_FAILURE);
	}

	/* Crypto-Binding TLV */
	binding->tlv_type = (be16) (host_to_be16((EAP_TLV_TYPE_MANDATORY |
						  EAP_TLV_CRYPTO_BINDING_TLV)));
	binding->length =
	    (host_to_be16((sizeof(*binding) - sizeof(EAP_tlv_hdr))));
	binding->version = EAP_FAST_VERSION;
	
	
	//it is the request because it does not receive the request binding
	if (!rbinding) {	
		binding->received_version = data->recv_ver;
		binding->subtype = EAP_TLV_CRYPTO_BINDING_SUBTYPE_REQUEST;
	} else {
		binding->received_version = rbinding->version;
		binding->subtype = EAP_TLV_CRYPTO_BINDING_SUBTYPE_RESPONSE;
	}

	//it is the request because it does not receive the request binding
	if (!rbinding) {	
		int tmp = os_get_random(binding->nonce, sizeof(binding->nonce));
		if (tmp < 0) {
			os_free(tlvs);
			return err;
		}
		binding->nonce[sizeof(binding->nonce) - 1] &= ~0x01;

		/*os_memcpy(data->crypto_binding_nonce, binding->nonce,
		   sizeof(binding->nonce)); */

	} else {
		os_memcpy(binding->nonce, rbinding->nonce,
			  sizeof(rbinding->nonce));
	}

	/*
	 * RFC 4851, Section 4.2.8:
	 * The nonce in a request MUST have its least significant bit set to 0.
	 */

	/*
	 * RFC 4851, Section 5.3:
	 * CMK = CMK[j]
	 * Compound-MAC = HMAC-SHA1( CMK, Crypto-Binding TLV )
	 */
	hmac_sha1(data->cmk, 20, (u8 *) binding, sizeof(*binding),
		  binding->compound_mac);

	if (MSG_DEBUG){
		printf("EAP-FAST: Add Crypto-Binding TLV: Version %d "
		       "Received Version %d SubType %d",
		       binding->version, binding->received_version,
		       binding->subtype);
		   }
		   
	hexdump(MSG_DEBUG, "EAP-FAST: NONCE",
		binding->nonce, sizeof(binding->nonce));
	hexdump(MSG_DEBUG, "EAP-FAST: Compound MAC",
		binding->compound_mac, sizeof(binding->compound_mac));
	
	*msg = tlvs;
	err = 0;

	return err;
}

int eap_fast_parse_tlv(EAP_fast_tlv_parse * tlv,
		       int tlv_type, u8 * pos, int len)
{
	switch (tlv_type) {
	case EAP_TLV_EAP_PAYLOAD_TLV:
		hexdump(MSG_DEBUG, "EAP-FAST: EAP-Payload TLV", pos, len);
		if (tlv->eap_payload_tlv) {
			printf("EAP-FAST: More than one "
			       "EAP-Payload TLV in the message");
			tlv->iresult = EAP_TLV_RESULT_FAILURE;
			return -2;
		}
		tlv->eap_payload_tlv = pos;
		tlv->eap_payload_tlv_len = len;
		break;
	case EAP_TLV_RESULT_TLV:
		hexdump(MSG_DEBUG, "EAP-FAST: Result TLV", pos, len);
		if (tlv->result) {
			printf("EAP-FAST: More than one "
			       "Result TLV in the message");
			tlv->result = EAP_TLV_RESULT_FAILURE;
			return -2;
		}
		if (len < 2) {
			printf("EAP-FAST: Too short " "Result TLV");
			tlv->result = EAP_TLV_RESULT_FAILURE;
			break;
		}
		tlv->result = WPA_GET_BE16(pos);
		if (tlv->result != EAP_TLV_RESULT_SUCCESS &&
		    tlv->result != EAP_TLV_RESULT_FAILURE) {
			printf("EAP-FAST: Unknown Result %d", tlv->result);
			tlv->result = EAP_TLV_RESULT_FAILURE;
		}

		if (MSG_DEBUG)
			printf("EAP-FAST: Result: %s",
			       tlv->result == EAP_TLV_RESULT_SUCCESS ?
			       "Success" : "Failure");
		break;
	case EAP_TLV_INTERMEDIATE_RESULT_TLV:
		hexdump(MSG_DEBUG, "EAP-FAST: Intermediate Result TLV",
			pos, len);
		if (len < 2) {
			printf("EAP-FAST: Too short "
			       "Intermediate-Result TLV");
			tlv->iresult = EAP_TLV_RESULT_FAILURE;
			break;
		}
		if (tlv->iresult) {
			printf("EAP-FAST: More than one "
			       "Intermediate-Result TLV in the message");
			tlv->iresult = EAP_TLV_RESULT_FAILURE;
			return -2;
		}
		tlv->iresult = WPA_GET_BE16(pos);
		if (tlv->iresult != EAP_TLV_RESULT_SUCCESS &&
		    tlv->iresult != EAP_TLV_RESULT_FAILURE) {
			printf("EAP-FAST: Unknown Intermediate "
			       "Result %d", tlv->iresult);
			tlv->iresult = EAP_TLV_RESULT_FAILURE;
		}
		if (MSG_DEBUG)
			printf("EAP-FAST: Intermediate Result: %s",
			       tlv->iresult == EAP_TLV_RESULT_SUCCESS ?
			       "Success" : "Failure");
		break;
	case EAP_TLV_CRYPTO_BINDING_TLV:
		hexdump(MSG_DEBUG, "EAP-FAST: Crypto-Binding TLV", pos, len);
		if (tlv->crypto_binding) {
			printf("EAP-FAST: More than one "
			       "Crypto-Binding TLV in the message");
			tlv->iresult = EAP_TLV_RESULT_FAILURE;
			return -2;
		}
		tlv->crypto_binding_len = sizeof(EAP_tlv_hdr) + len;
		if (tlv->crypto_binding_len < sizeof(*tlv->crypto_binding)) {
			printf("EAP-FAST: Too short " "Crypto-Binding TLV");
			tlv->iresult = EAP_TLV_RESULT_FAILURE;
			return -2;
		}
		tlv->crypto_binding = (EAP_tlv_crypto_binding_tlv *)
		    (pos - sizeof(EAP_tlv_hdr));
		break;

	default:
		/* Unknown TLV */
		return -1;
	}

	return 0;
}

static u8 *eap_fast_tlv_nak(int vendor_id, int tlv_type, size_t * len)
{
	u8 *buf = NULL;
	EAP_tlv_nak_tlv *nak = NULL;
	*len = sizeof(*nak);

	buf = malloc(sizeof(*nak));
	if (buf == NULL) {
		return NULL;
	}

	nak = (EAP_tlv_nak_tlv *) buf;
	nak->tlv_type =
	    (be16) host_to_be16((EAP_TLV_TYPE_MANDATORY | EAP_TLV_NAK_TLV));
	nak->length = host_to_be16(6);
	nak->vendor_id = host_to_be32(vendor_id);
	nak->nak_type = host_to_be16(tlv_type);

	return buf;
}

static int eap_fast_parse_decrypted(u8 * decrypted, size_t decrypted_len,
				    EAP_fast_tlv_parse * tlv,
				    u8 * resp, size_t * resp_len)
{
	int mandatory, tlv_type, len, res;
	u8 *pos, *end;

	os_memset(tlv, 0, sizeof(*tlv));

	/* Parse TLVs from the decrypted Phase 2 data */
	pos = decrypted;
	end = pos + decrypted_len;
	while (pos + 4 < end) {
		mandatory = pos[0] & 0x80;
		tlv_type = WPA_GET_BE16(pos) & 0x3fff;
		pos += 2;
		len = WPA_GET_BE16(pos);
		pos += 2;
		if (pos + len > end) {
			printf("EAP-FAST: TLV overflow");
			return -1;
		}
		if (MSG_DEBUG)
			printf("EAP-FAST: Received Phase 2: "
			       "TLV type %d length %d%s",
			       tlv_type, len, mandatory ? " (mandatory)" : "");

		res = eap_fast_parse_tlv(tlv, tlv_type, pos, len);

		if (res == -2) {
			break;
		}
		if (res < 0) {
			if (mandatory) {
				printf("EAP-FAST: Nak unknown "
				       "mandatory TLV type %d", tlv_type);
				resp = eap_fast_tlv_nak(0, tlv_type, resp_len);
				break;
			} else {
				printf("EAP-FAST: ignored "
				       "unknown optional TLV type %d",
				       tlv_type);
			}
		}

		pos += len;
	}

	return 0;
}

static u8 *eap_fast_tlv_result(int status, int intermediate, size_t * len)
{
	u8 *buf;
	EAP_tlv_intermediate_result_tlv *result;
	buf = malloc(sizeof(*result));
	if (buf == NULL)
		return NULL;
	if (MSG_DEBUG)
		printf("EAP-FAST: Add %sResult TLV(status=%d)",
		       intermediate ? "Intermediate " : "", status);
	result = (EAP_tlv_intermediate_result_tlv *) buf;
	result->tlv_type = host_to_be16((EAP_TLV_TYPE_MANDATORY |
					(intermediate ?
					 EAP_TLV_INTERMEDIATE_RESULT_TLV :
					 EAP_TLV_RESULT_TLV)));
	result->length = host_to_be16(2);
	result->status = host_to_be16(status);
	*len = sizeof(*result);
	return buf;
}

int eap_fast_peer_process_start(struct FAST_context_peer *ctx, FAST_data * data,
				u8 flags, u8 * auth_id_data)
{
	const u8 *auth_id = NULL;
	size_t auth_id_len;

	/* EAP-FAST Version negotiation (section 3.1) */

	auth_id = eap_fast_get_auth_id(auth_id_data, &auth_id_len);
	eap_fast_select_pac(ctx, auth_id, auth_id_len);

	if (ctx->pac_curr) {
		/*
		 * PAC found for the A-ID and we are not resuming an old
		 * session, so add PAC-Opaque extension to ClientHello.
		 */
		if (eap_fast_use_pac_opaque(data, ctx->pac_curr) < 0) {
			return -1;
		}
	} else {

		printf("EAP-FAST: No PAC found and " "provisioning disabled");
		return -1;
	}

	return 0;
}

static size_t eap_fast_get_inner_eap_key(u8 * isk, size_t isk_len)
{
	memset(isk, 0, isk_len);
	return 1;
}

static size_t eap_fast_get_phase2_key(u8 * isk, size_t isk_len)
{
	return eap_fast_get_inner_eap_key(isk, isk_len);
}

static int eap_fast_update_icmk(FAST_data * data)
{
	u8 isk[32], imck[60];

	/*if(MSG_DEBUG)
	   printf("EAP-FAST: Deriving ICMK[%d] (S-IMCK and CMK)",
	   data->simck_idx + 1); */
	/*
	 * RFC 4851, Section 5.2:
	 * IMCK[j] = T-PRF(S-IMCK[j-1], "Inner Methods Compound Keys",
	 *                 MSK[j], 60)
	 * S-IMCK[j] = first 40 octets of IMCK[j]
	 * CMK[j] = last 20 octets of IMCK[j]
	 */

	if (eap_fast_get_phase2_key(isk, sizeof(isk)) < 0) {
		return -1;
	}

	hexdump_key(MSG_DEBUG, "EAP-FAST: ISK[j]", isk, sizeof(isk));

	sha1_t_prf(data->simck, EAP_FAST_SIMCK_LEN,
		   "Inner Methods Compound Keys",
		   isk, sizeof(isk), imck, sizeof(imck));

	os_memcpy(data->simck, imck, EAP_FAST_SIMCK_LEN);

	hexdump_key(MSG_DEBUG, "EAP-FAST: S-IMCK[j]",
		    data->simck, EAP_FAST_SIMCK_LEN);

	os_memcpy(data->cmk, imck + EAP_FAST_SIMCK_LEN, 20);

	hexdump_key(MSG_DEBUG, "EAP-FAST: CMK[j]", data->cmk, 20);

	return 0;
}

void eap_fast_derive_key_auth(FAST_data * data)
{
	u8 *sks = NULL;

	/* RFC 4851, Section 5.1:
	 * Extra key material after TLS key_block: session_key_seed[40]
	 */

	sks = eap_fast_derive_key(data->ssl_data, "key expansion",
				  EAP_FAST_SKS_LEN);
				  
	if (sks == NULL) {
		if(MSG_DEBUG) {
			printf("EAP-FAST: Failed to derive session_key_seed");
		}
		return;
	}

	/*
	 * RFC 4851, Section 5.2:
	 * S-IMCK[0] = session_key_seed
	 */
	hexdump_key(MSG_DEBUG,
		    "EAP-FAST: session_key_seed (SKS = S-IMCK[0])",
		    sks, EAP_FAST_SKS_LEN);

	os_memcpy(data->simck, sks, EAP_FAST_SIMCK_LEN);
	os_free(sks);
}

static u8 innerEapCode2iresult(u8 inner_eap_code)
{
	if (inner_eap_code == EAP_CODE_SUCCESS) {
		return EAP_TLV_RESULT_SUCCESS;
	}
	if (inner_eap_code == EAP_CODE_FAILURE) {
		return EAP_TLV_RESULT_FAILURE;
	}
	return (u8) 0;
}

u8 eap_fast_build_phase2_req(FAST_data * data, u8 inner_eap_code, char **msg,
			     size_t * len)
{
	u8 err = 1;

	switch (inner_eap_code) {

	case EAP_CODE_REQUEST:
	case EAP_CODE_RESPONSE:
		err = eap_fast_build_tlv_eap_payload(msg, len);
		break;
	case EAP_CODE_SUCCESS:
		eap_fast_update_icmk(data);
		err =
		    eap_fast_build_tlv_iresult_crypto_binding(data,
							      inner_eap_code,
							      NULL, msg, len);
		break;
	case EAP_CODE_FAILURE:
		err =
		    eap_fast_build_tlv_iresult(innerEapCode2iresult
					       (inner_eap_code), msg, len);
		break;

	default:
		printf("EAP-FAST ERROR: can not build the message that"
		       " contains a invalid inner EAP code (code = %d)\n",
		       inner_eap_code);
		return err;
	}

	return err;
}

u8 eap_fast_build_phase2_resp(FAST_data * data, u8 code, char **msg,
			      size_t * len)
{
	return eap_fast_build_phase2_req(data, code, msg, len);
}

static char tunnelEapCode2subType(u8 * tunnel_eap_code)
{
	if (tunnel_eap_code == (u8 *) EAP_CODE_REQUEST) {
		return EAP_TLV_CRYPTO_BINDING_SUBTYPE_REQUEST;
	}
	if (tunnel_eap_code == (u8 *) EAP_CODE_RESPONSE) {
		return EAP_TLV_CRYPTO_BINDING_SUBTYPE_RESPONSE;
	}
	return EAP_TLV_CRYPTO_BINDING_SUBTYPE_NONE;
}

u8 eap_fast_process_phase2_req(FAST_data * fast, u8 tunnel_eap_code,
			       char *data, size_t data_len,
			       char **resp, size_t * resp_len,
			       char **eapPayload, size_t * eapPayloadLen)
{
	EAP_fast_tlv_parse tlv;
	u8 err = 1;

	if (eap_fast_parse_decrypted(data, (int *)data_len, &tlv, resp, resp_len) < 0) {
		if (MSG_DEBUG) {
			printf("ERROR in eap_fast_parse_decrypted. \n");
		}
		return err;
	}

	if (*resp) {
		if (MSG_DEBUG) {
			printf("NAK will be send.\n");
		}
		return err;
	}

	if (tlv.eap_payload_tlv) {
		if ((tunnel_eap_code == EAP_CODE_REQUEST)
		    || (tunnel_eap_code == EAP_CODE_RESPONSE)) {

			*eapPayload = tlv.eap_payload_tlv;
			*eapPayloadLen = tlv.eap_payload_tlv_len;

			return 0;
		}
	}

	if (tlv.result == EAP_TLV_RESULT_FAILURE) {
		if (tunnel_eap_code == EAP_CODE_REQUEST) {
			fast->result = EAP_TLV_RESULT_FAILURE;
			err =
			    eap_fast_build_tlv_result(EAP_TLV_RESULT_FAILURE,
						      resp, resp_len);
		} else {
			if (fast->result == EAP_TLV_RESULT_FAILURE)
				err = 0;
		}
		return err;
	}

	if (tlv.iresult == EAP_TLV_RESULT_FAILURE) {
		if (tunnel_eap_code == EAP_CODE_REQUEST) {
			err =
			    eap_fast_build_tlv_iresult(EAP_TLV_RESULT_FAILURE,
						       resp, resp_len);
		} else		//tunnel_eap_code == EAP_CODE_RESPONSE
		{
			fast->result = EAP_TLV_RESULT_FAILURE;
			err =
			    eap_fast_build_tlv_result(EAP_TLV_RESULT_FAILURE,
						      resp, resp_len);
		}
		return err;
	}

	if (tlv.crypto_binding && tlv.iresult == EAP_TLV_RESULT_SUCCESS) {
		err = eap_fast_process_crypto_binding(fast, tlv.crypto_binding,
						      tlv.crypto_binding_len,
						      EAP_TLV_RESULT_SUCCESS,
						      tunnel_eap_code, resp,
						      resp_len);
		if (!err && tunnel_eap_code == EAP_CODE_RESPONSE) {
			fast->result = EAP_TLV_RESULT_SUCCESS;
		}
		return err;
	}

	if (tlv.result == EAP_TLV_RESULT_SUCCESS) {
		if (tunnel_eap_code == EAP_CODE_RESPONSE) {
			if (fast->result == EAP_TLV_RESULT_SUCCESS) {
				err = 0;
			}
			return err;
		}
		fast->result = EAP_TLV_RESULT_SUCCESS;
		err =
		    eap_fast_build_tlv_result(EAP_TLV_RESULT_SUCCESS, resp,
					      resp_len);
		return err;
	}

	if (MSG_DEBUG) {
		printf("ERROR in received TLV.\n");
	}

	return err;
}

u8 eap_fast_process_phase2_resp(FAST_data * fast, u8 tunnel_eap_code,
				char *data, size_t data_len,
				char **resp, size_t * respLen,
				char **eapPayload, size_t * eapPayloadLen)
{
	return eap_fast_process_phase2_req(fast, tunnel_eap_code, data,
					   data_len, resp, respLen, eapPayload,
					   eapPayloadLen);
}

static u8 *eap_fast_build_phase2_res(FAST_data * fast, u8 * credential,
				     int credential_len, int *res_len)
{
	u8 *res = NULL;
	/*if(fast ->result !=2)res = GTC_auth_build_res(fast ->phase2_id,res_len,credential,credential_len);
	   else res = GTC_auth_build_res(fast ->phase2_id,res_len,NULL,0); */
	return (u8 *)eap_fast_tlv_eap_payload(res, res_len);
}

u8 *eap_fast_buildRes(FAST_data * fast, u8 * credential, int credential_len,
		      int *res_len)
{
	*res_len = 0;
	u8 *res = eap_fast_build_phase2_res(fast, credential, credential_len,
					    res_len);
	return res;
}

int eap_fast_load_credential(u8 ** credential, const char *credential_file)
{
	int len = 0;
	*credential = NULL;
	*credential = (u8 *) os_readfile(credential_file, &len);
	return len;
}

int eap_fast_process_phase2_response(FAST_data * fast, u8 * res, int res_len,
				     size_t * valid)
{
	// Stub function to be used perhaps in the future
	return 0;
}

static u8 eap_fast_build_tlv_iresult(u8 iresult, char **msg, size_t * msg_len)
{
	int err = 1;
	EAP_tlv_result_tlv *tlv = malloc(sizeof(EAP_tlv_result_tlv));

	if (tlv == NULL) {
		return err;
	}
	memset(tlv, 0, sizeof(EAP_tlv_result_tlv));
	tlv->tlv_type =
	    host_to_be16((EAP_TLV_TYPE_MANDATORY |
			  EAP_TLV_INTERMEDIATE_RESULT_TLV));
	tlv->length = host_to_be16(2);
	tlv->status = host_to_be16(iresult);
	*msg = tlv;
	*msg_len = sizeof(EAP_tlv_result_tlv);
	if (MSG_DEBUG) {
		printf("[EAP-FAST] a iresult TLV is build (status = %d).\n",
		       iresult);
	}
	err = 0;
	return err;
}

static u8 eap_fast_build_tlv_result(u8 iresult, char **msg, size_t * msg_len)
{
	int err = 1;
	EAP_tlv_result_tlv *tlv = malloc(sizeof(EAP_tlv_result_tlv));

	if (tlv == NULL)
		return err;
	memset(tlv, 0, sizeof(EAP_tlv_result_tlv));
	tlv->tlv_type =
	    (be16) (host_to_be16((EAP_TLV_TYPE_MANDATORY | EAP_TLV_RESULT_TLV)));
	tlv->length = host_to_be16(2);
	tlv->status = host_to_be16(iresult);
	*msg_len = sizeof(EAP_tlv_result_tlv);
	if (MSG_DEBUG)
		printf("[EAP-FAST]a result TLV is build (status = %d).\n",
		       iresult);
	*msg = tlv;
	err = 0;
	return err;
}

/**
 *  //if flag = 1, request. if flag =0, response
 */
static int eap_fast_validate_crypto_binding(size_t flag,
					    EAP_tlv_crypto_binding_tlv * bind)
{
	if (MSG_DEBUG) {
		printf("EAP-FAST: Crypto-Binding TLV: Version %d "
		       "Received Version %d SubType %d",
		       bind->version, bind->received_version, bind->subtype);
	}

	hexdump(MSG_DEBUG, "EAP-FAST: NONCE", bind->nonce, sizeof(bind->nonce));
	hexdump(MSG_DEBUG, "EAP-FAST: Compound MAC",
		bind->compound_mac, sizeof(bind->compound_mac));

	if (bind->version != EAP_FAST_VERSION ||
	    bind->received_version != EAP_FAST_VERSION) {

		printf("EAP-FAST: Invalid version in "
		       "Crypto-Binding TLV: Version %d "
		       "Received Version %d \n",
		       bind->version, bind->received_version);
		return -1;
	}
	if (flag == EAP_CODE_REQUEST
	    && bind->subtype != EAP_TLV_CRYPTO_BINDING_SUBTYPE_REQUEST) {
		printf("EAP-FAST: Invalid Subtype : %d\n", bind->subtype);
		return -1;
	}
	if (flag == EAP_CODE_RESPONSE
	    && bind->subtype != EAP_TLV_CRYPTO_BINDING_SUBTYPE_RESPONSE) {
		printf("EAP-FAST: Invalid Subtype : %d\n", bind->subtype);
		return -1;
	}

	return 0;
}

static u8 iresult2innerEapCode(u8 iresult)
{
	if (iresult == EAP_TLV_RESULT_SUCCESS) {
		return EAP_CODE_SUCCESS;
	}
	if (iresult == EAP_TLV_RESULT_FAILURE) {
		return EAP_CODE_FAILURE;
	}
	return 0;
}

static u8
eap_fast_process_crypto_binding(FAST_data * data,
				EAP_tlv_crypto_binding_tlv * binding,
				size_t binding_len, u8 iresult,
				u8 tunnel_eap_code, char **resp,
				size_t * resp_len)
{
	u8 cmac[SHA1_MAC_LEN];
	u8 err = 1;
	int validation = 1;

	if (eap_fast_validate_crypto_binding(tunnel_eap_code, binding) < 0) {
		return err;
	}

	char sub_type = tunnelEapCode2subType(tunnel_eap_code);

	if (sub_type == EAP_TLV_CRYPTO_BINDING_SUBTYPE_REQUEST) {
		if (eap_fast_update_icmk(data) < 0) {
			return err;
		}
	}

	/* Validate received Compound MAC */
	os_memcpy(cmac, binding->compound_mac, sizeof(cmac));
	os_memset(binding->compound_mac, 0, sizeof(cmac));

	hexdump(MSG_DEBUG, "EAP-FAST: Crypto-Binding TLV for Compound "
		"MAC calculation", (u8 *) binding, binding_len);

	hmac_sha1(data->cmk, EAP_FAST_CMK_LEN, (u8 *) binding, binding_len,
		  binding->compound_mac);
	validation = memcmp(cmac, binding->compound_mac, sizeof(cmac));

	hexdump(MSG_DEBUG, "EAP-FAST: Received Compound MAC",
		cmac, sizeof(cmac));
	hexdump(MSG_DEBUG, "EAP-FAST: Calculated Compound MAC",
		binding->compound_mac, sizeof(cmac));

	if (validation != 0) {
		printf("EAP-FAST: Compound MAC did not match");
		os_memcpy(binding->compound_mac, cmac, sizeof(cmac));
		return err;
	}

	if (sub_type == EAP_TLV_CRYPTO_BINDING_SUBTYPE_RESPONSE) {
		data->result = iresult;
		err = eap_fast_build_tlv_result(iresult, resp, resp_len);
		return err;
	}

	/*
	 * Compound MAC was valid, so authentication succeeded. Reply with
	 * crypto binding to allow server to complete authentication.
	 */

	u8 inner_eap_code = iresult2innerEapCode(iresult);
	err =
	    eap_fast_build_tlv_iresult_crypto_binding(data, inner_eap_code,
						      binding, resp, resp_len);

	return err;

}

u8 *eap_fast_get_msk(u8 * simck, size_t * len)
{
	u8 *eapKeyData = NULL;

	/*
	 * RFC 4851, Section 5.4: EAP Master Session Key Genreration
	 * MSK = T-PRF(S-IMCK[j], "Session Key Generating Function", 64)
	 */

	eapKeyData = malloc(EAP_FAST_MSK_LEN);
	if (eapKeyData == NULL) {
		return NULL;
	}

	sha1_t_prf(simck, EAP_FAST_SIMCK_LEN,
		   "Session Key Generating Function", (u8 *) "", 0,
		   eapKeyData, EAP_FAST_MSK_LEN);

	hexdump_key(MSG_DEBUG, "EAP-FAST: Derived key (MSK)",
		    eapKeyData, EAP_FAST_MSK_LEN);

	*len = EAP_FAST_MSK_LEN;

	return eapKeyData;
}

u8 *eap_fast_get_emsk(u8 * simck, size_t * len)
{
	u8 *eapKeyData = NULL;

	/*
	 * RFC 4851, Section 5.4: EAP Master Session Key Genreration
	 * EMSK = T-PRF(S-IMCK[j],
	 *        "Extended Session Key Generating Function", 64)
	 */

	eapKeyData = os_malloc(EAP_FAST_EMSK_LEN);
	if (eapKeyData == NULL) {
		return NULL;
	}

	sha1_t_prf(simck, EAP_FAST_SIMCK_LEN,
		   "Extended Session Key Generating Function",
		   (u8 *) "", 0, eapKeyData, EAP_FAST_EMSK_LEN);
	hexdump_key(MSG_DEBUG, "EAP-FAST: Derived key (EMSK)",
		    eapKeyData, EAP_FAST_EMSK_LEN);

	*len = EAP_FAST_EMSK_LEN;

	return eapKeyData;
}

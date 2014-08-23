#ifndef EAP_FAST_COMMON_H
#define EAP_FAST_COMMON_H
 
#ifdef __cplusplus
extern "C"{
#endif

#include "base/os.h"
#include "base/md5.h"
#include "base/sha1.h"

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

#define TLS_EXT_PAC_OPAQUE 35

#define FAST_DATA_HEADER_LEN          4

#define FAST_DATA_TYPE_AUTH_ID	4

#define PAC_OPAQUE_TYPE_PAD 0
#define PAC_OPAQUE_TYPE_KEY 1
#define PAC_OPAQUE_TYPE_LIFETIME 2
#define PAC_KEY_REFRESH_TIME (1 * 24 * 60 * 60)

#define EAP_FAST_PROV_UNAUTH 1
#define EAP_FAST_PROV_AUTH 2

#define EAP_FAST_SKS_LEN 40
#define EAP_FAST_SIMCK_LEN 40

#define EAP_TLV_TYPE_MANDATORY 0x8000
#define EAP_TLV_EAP_PAYLOAD_TLV 9
#define EAP_TLV_RESULT_TLV 3 /* Acknowledged Result */
#define EAP_TLV_NAK_TLV 4
#define EAP_TLV_INTERMEDIATE_RESULT_TLV 10
#define EAP_TLV_CRYPTO_BINDING_TLV 12
#define EAP_TLV_PAC_TLV 11 /* RFC 5422, Section 4.2 */

#define EAP_TLV_RESULT_NONE 0
#define EAP_TLV_RESULT_SUCCESS 1
#define EAP_TLV_RESULT_FAILURE 2

#define	EAP_FAST_VERSION	1

#define EAP_TLV_CRYPTO_BINDING_SUBTYPE_REQUEST	0
#define EAP_TLV_CRYPTO_BINDING_SUBTYPE_RESPONSE	1
#define EAP_TLV_CRYPTO_BINDING_SUBTYPE_NONE	-1

#define EAP_FAST_CMK_LEN 20

#define EAP_FAST_MSK_LEN 64
#define EAP_FAST_EMSK_LEN 64


#define	EAP_CODE_REQUEST	1
#define EAP_CODE_RESPONSE	2
#define EAP_CODE_SUCCESS	3
#define EAP_CODE_FAILURE	4




struct EAP_HDR {
	u8 code;
	u8 identifier;
	be16 length; /* including code and identifier; network byte order */
	/* followed by length-4 octets of data */
};

struct eap_fast_pac {
	struct eap_fast_pac *next;

	u8 pac_key[EAP_FAST_PAC_KEY_LEN];
	u8 *pac_opaque;
	size_t pac_opaque_len;
	u8 *pac_info;
	size_t pac_info_len;
	u8 *a_id;
	size_t a_id_len;
	u8 *i_id;
	size_t i_id_len;
	u8 *a_id_info;
	size_t a_id_info_len;
	u16 pac_type;
};

typedef SSL_CTX TLS_context;

struct FAST_context_auth{
	TLS_context *tls_ctx;
	u8 pac_opaque_encr[16];
	int send_new_pac; /* server triggered re-keying of Tunnel PAC. initiate it with 0*/
	bool opaquevalid;
	
};
struct FAST_context_peer{
	TLS_context *tls_ctx;
	struct eap_fast_pac *pac_root;
	struct eap_fast_pac *pac_curr;//current pac. initate it with NULL
	int session_ticket_used;//whether to use ticket. 0 = not used. 1 = used .initate with 1 
	bool provisioning_allowed;
	bool opaquevalid;
};

typedef int (*tls_session_ticket_cb)
(void *ctx, const u8 *ticket, size_t len, const u8 *client_random,
 const u8 *server_random, u8 *master_secret);

struct FAST_DATA{
	SSL *ssl_data;
	tls_session_ticket_cb session_ticket_cb;
	void *session_ticket_cb_ctx;

	/* SessionTicket received from OpenSSL hello_extension_cb (server) */
	u8 *session_ticket;
	size_t session_ticket_len;
	size_t recv_ver;
	size_t result;
	u8 simck[EAP_FAST_SIMCK_LEN];
        u8 cmk[20];
	//int simck_idx;			
};
typedef struct FAST_DATA FAST_data;

struct EAP_TLV_HDR {
	be16 tlv_type;
	be16 length;
};
typedef struct EAP_TLV_HDR	EAP_tlv_hdr;



struct EAP_TLV_nak_TLV {
	be16 tlv_type;
	be16 length;
	be32 vendor_id;
	be16 nak_type;
};
typedef	struct EAP_TLV_nak_TLV	EAP_tlv_nak_tlv;

struct EAP_TLV_result_TLV {
	be16 tlv_type;
	be16 length;
	be16 status;
};
typedef	struct EAP_TLV_result_TLV	EAP_tlv_result_tlv;

/* RFC 4851, Section 4.2.7 - Intermediate-Result TLV */
struct EAP_TLV_intermediate_result_TLV {
	be16 tlv_type;
	be16 length;
	be16 status;
	/* Followed by optional TLVs */
};
typedef	struct EAP_TLV_intermediate_result_TLV	EAP_tlv_intermediate_result_tlv;

/* RFC 4851, Section 4.2.8 - Crypto-Binding TLV */
struct EAP_TLV_crypto_binding_TLV {
	be16 tlv_type;
	be16 length;
	u8 reserved;
	u8 version;
	u8 received_version;
	u8 subtype;
	u8 nonce[32];
	u8 compound_mac[20];
};
typedef	struct EAP_TLV_crypto_binding_TLV	EAP_tlv_crypto_binding_tlv;

struct EAP_FAST_TLV_PARSE {
	u8 *eap_payload_tlv;
	size_t eap_payload_tlv_len;
	EAP_tlv_crypto_binding_tlv *crypto_binding;
	size_t crypto_binding_len;
	int iresult;
	int result;
	int request_action;
	u8 *pac;
	size_t pac_len;
};
typedef	struct EAP_FAST_TLV_PARSE 	EAP_fast_tlv_parse;

struct TLS_KEYS {
	const u8 *master_key; /* TLS master secret */
	size_t master_key_len;
	const u8 *client_random;
	size_t client_random_len;
	const u8 *server_random;
	size_t server_random_len;
	const u8 *inner_secret; /* TLS/IA inner secret */
	size_t inner_secret_len;
};
typedef struct TLS_KEYS TLS_keys;


int 
eap_fast_peer_process_start(struct FAST_context_peer *ctx,FAST_data *data, 
				u8 flags, u8 * auth_id_data);
void 
eap_fast_derive_key_auth(FAST_data *data);


struct eap_fast_pac * eap_fast_get_pac(struct eap_fast_pac *pac_root,
				       const u8 *a_id, size_t a_id_len,
				       u16 pac_type);
void eap_fast_derive_master_secret(const u8 *pac_key, const u8 *server_random,
				   const u8 *client_random, u8 *master_secret);

int eap_fast_parse_tlv(	EAP_fast_tlv_parse *tlv,
		       int tlv_type, u8 *pos, int len); 


u8 eap_fast_build_phase2_req (FAST_data *data, u8 code, char** msg, size_t *len);
u8 eap_fast_build_phase2_resp(FAST_data *data, u8 code, char** msg, size_t *len);

u8 eap_fast_process_phase2_req(FAST_data * fast, u8 tunnel_eap_code, char *data, size_t data_len, 
					char **resp, size_t *respLen, 
					char** eapPayload, size_t *eapPayloadLen);
u8 eap_fast_process_phase2_resp(FAST_data * fast, u8 tunnel_eap_code, char *data, size_t data_len, 
					char **resp, size_t *respLen, 
					char** eapPayload, size_t *eapPayloadLen);
u8 * eap_fast_get_msk(u8 * simck, size_t *len);
u8 * eap_fast_get_emsk(u8 *simck, size_t *len);

#ifdef __cplusplus
}
#endif

#endif //EAP_FAST_COMMON_H





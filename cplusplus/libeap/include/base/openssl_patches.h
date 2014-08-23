//#include <openssl/ssl.h>

#ifndef OPENSSL_PATCHES_H
#define OPENSSL_PATCHES_H

#ifdef __cplusplus
extern "C"{
#endif

#include "base/eap_fast_common.h"
#include "base/os.h"

int tls_connection_auth_set_session_ticket_cb(void *tls_ctx,
					 FAST_data *conn,
					 tls_session_ticket_cb cb,
					 void *ctx);
int tls_connection_peer_set_session_ticket_cb(void *tls_ctx,
					 FAST_data *conn,
					 tls_session_ticket_cb cb,
					 void *ctx);
int tls_connection_set_cipher_list(FAST_data *conn, u8 *ciphers);
int tls_connection_get_keyblock_size(SSL *ssl);
int tls_connection_get_keys(SSL *ssl, TLS_keys *keys);
int tls_get_cipher(FAST_data *conn,
		   char *buf, size_t buflen);
int tls_connection_client_hello_ext(FAST_data *conn,
				    int ext_type, const u8 *data,
				    size_t data_len);

#ifdef __cplusplus
}
#endif
#endif//OPENSSL_PATCHES_H




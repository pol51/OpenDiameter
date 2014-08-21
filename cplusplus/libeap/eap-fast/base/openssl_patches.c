#include "base/openssl_patches.h"
//#include "eap_fast.hxx"

#include <openssl/ssl.h>

static int tls_sess_sec_cb(SSL *s, void *secret, int *secret_len,
			   STACK_OF(SSL_CIPHER) *peer_ciphers,
			   SSL_CIPHER **cipher, void *arg)
{
	FAST_data *conn = arg;
	int ret;


	if (conn == NULL || conn->session_ticket_cb == NULL)
		return 0;

	ret = conn->session_ticket_cb(conn->session_ticket_cb_ctx,
				      conn->session_ticket,
				      conn->session_ticket_len,
				      s->s3->client_random,
				      s->s3->server_random, secret);

	free(conn->session_ticket);
	conn->session_ticket = NULL;

	if (ret <= 0)
		return 0;

	*secret_len = SSL_MAX_MASTER_KEY_LENGTH;
	return 1;
}


static int tls_hello_ext_cb(SSL *s, TLS_EXTENSION *ext, void *arg)
{
	FAST_data *conn = arg;

	/*if (conn == NULL || conn->session_ticket_cb == NULL)
		{printf("error 1\n");return 1;}*/  
	
	/*wpa_printf(MSG_DEBUG, "OpenSSL: %s: type=%d length=%d", __func__,
		   ext->type, ext->length);*/

	if(conn->session_ticket!=NULL){
		free(conn->session_ticket);
	}
	
	conn->session_ticket = NULL;

	//if (ext->type == 35) {
		/*wpa_hexdump(MSG_DEBUG, "OpenSSL: ClientHello SessionTicket "
			    "extension", ext->data, ext->length);*/
		conn->session_ticket = os_malloc(ext->length);
		if (conn->session_ticket == NULL)
			return SSL_AD_INTERNAL_ERROR;

		memcpy(conn->session_ticket, ext->data, ext->length);
		conn->session_ticket_len = ext->length;
	//}

	return 0;
}
static int tls_session_ticket_ext_cb(SSL *s, const unsigned char *data, int len, void *arg)
{
	FAST_data *conn = arg;

	if (conn == NULL || conn->session_ticket_cb == NULL)
		return 0;

	//wpa_printf(MSG_DEBUG, "OpenSSL: %s: length=%d", __func__, len);

	if(conn->session_ticket!=NULL){
		free(conn->session_ticket);
	}
	conn->session_ticket = NULL;

	/*wpa_hexdump(MSG_DEBUG, "OpenSSL: ClientHello SessionTicket "
		    "extension", data, len);*/

	conn->session_ticket = os_malloc(len);
	if (conn->session_ticket == NULL)
		return 0;

	memcpy(conn->session_ticket, data, len);
	conn->session_ticket_len = len;

	return 1;
}

int tls_connection_get_keys(SSL *ssl,
			    TLS_keys *keys)
{
	//SSL *ssl;

	if (ssl == NULL || keys == NULL)
		return -1;
	//ssl = conn->ssl;
	if (ssl == NULL || ssl->s3 == NULL || ssl->session == NULL)
		return -1;

	os_memset(keys, 0, sizeof(*keys));
	keys->master_key = ssl->session->master_key;
	keys->master_key_len = ssl->session->master_key_length;
	keys->client_random = ssl->s3->client_random;
	keys->client_random_len = SSL3_RANDOM_SIZE;
	keys->server_random = ssl->s3->server_random;
	keys->server_random_len = SSL3_RANDOM_SIZE;

	return 0;
}
int tls_connection_get_keyblock_size(SSL *ssl)
{
	const EVP_CIPHER *c;
	const EVP_MD *h;

	if (ssl == NULL ||
	    ssl->enc_read_ctx == NULL ||
	    ssl->enc_read_ctx->cipher == NULL ||
	    ssl->read_hash == NULL)
		return -1;

	c = ssl->enc_read_ctx->cipher;

	h = ssl->read_hash;

	return 2 * (EVP_CIPHER_key_length(c) +
		    EVP_MD_size(EVP_MD_CTX_md(h)) +
		    EVP_CIPHER_iv_length(c));
}



int tls_connection_auth_set_session_ticket_cb(void *tls_ctx,
					 FAST_data *conn,
					 tls_session_ticket_cb cb,
					 void *ctx)
{ 
	conn->session_ticket_cb = cb; 
	conn->session_ticket_cb_ctx = ctx;

	if (cb) {
		if(SSL_set_session_secret_cb(conn->ssl_data, tls_sess_sec_cb,
					      conn) != 1)
			return -1;
		SSL_set_session_ticket_ext_cb(conn->ssl_data,
					      tls_session_ticket_ext_cb, conn);


		if(SSL_set_hello_extension_cb(conn->ssl_data, tls_hello_ext_cb,
					       conn) != 1)
			return -1;

	} 
	else {

		if (SSL_set_session_secret_cb(conn->ssl_data, NULL, NULL) != 1) 
			return -1;
		if (SSL_set_hello_extension_cb(conn->ssl_data, NULL, NULL)!= 1)
			return -1;
	}

	return 0;
}
int tls_connection_peer_set_session_ticket_cb(void *tls_ctx,
					 FAST_data  *conn,
					 tls_session_ticket_cb cb,
					 void *ctx)
{
	conn->session_ticket_cb = cb;
	conn->session_ticket_cb_ctx = ctx;

	if (cb) {
		if (SSL_set_session_secret_cb(conn->ssl_data, tls_sess_sec_cb,
					      conn) != 1)
			return -1;
		SSL_set_session_ticket_ext_cb(conn->ssl_data,
					      tls_session_ticket_ext_cb, conn);

		if (SSL_set_hello_extension_cb(conn->ssl_data, tls_hello_ext_cb,
					       conn) != 1)
			return -1;

	} else {
		if (SSL_set_session_secret_cb(conn->ssl_data, NULL, NULL) != 1)
			return -1;
#ifdef CONFIG_OPENSSL_TICKET_OVERRIDE
		SSL_set_session_ticket_ext_cb(conn->ssl_data, NULL, NULL);
#else /* CONFIG_OPENSSL_TICKET_OVERRIDE */
#ifdef SSL_OP_NO_TICKET

#else /* SSL_OP_NO_TICKET */
		if (SSL_set_hello_extension_cb(conn->ssl_data, NULL, NULL) != 1)
			return -1;
#endif /* SSL_OP_NO_TICKET */
#endif /* CONFIG_OPENSSL_TICKET_OVERRIDE */
	}

	return 0;
}
/* ClientHello TLS extensions require a patch to openssl, so this function is
 * commented out unless explicitly needed for EAP-FAST in order to be able to
 * build this file with unmodified openssl. */
int tls_connection_client_hello_ext(FAST_data *conn,
				    int ext_type, const u8 *data,
				    size_t data_len)
{
	if (conn == NULL || conn->ssl_data == NULL || ext_type != 35)
	{	
		return -1;
	}

#ifdef CONFIG_OPENSSL_TICKET_OVERRIDE

	if (SSL_set_session_ticket_ext(conn->ssl_data, (void *) data,
				       data_len) != 1)
		{return -1;}
#else /* CONFIG_OPENSSL_TICKET_OVERRIDE */

	if (SSL_set_hello_extension(conn->ssl_data, ext_type, (void *) data,
				    data_len) != 1)
		{return -1;}

#endif /* CONFIG_OPENSSL_TICKET_OVERRIDE */
	

	return 0;
}

int tls_connection_set_cipher_list(FAST_data *conn, u8 *ciphers)
{
	char buf[100], *pos, *end;
	u8 *c;
	int ret;
	if (conn == NULL || conn->ssl_data == NULL || ciphers == NULL)
		return -1;

	buf[0] = '\0';
	pos = buf;
	end = pos + sizeof(buf);

	c = ciphers;

	while (*c != TLS_CIPHER_NONE) {
		const char *suite;

		switch (*c) {
		case TLS_CIPHER_RC4_SHA:
			suite = "RC4-SHA";
			break;
		case TLS_CIPHER_AES128_SHA:
			suite = "AES128-SHA";
			break;
		case TLS_CIPHER_RSA_DHE_AES128_SHA:
			suite = "DHE-RSA-AES128-SHA";
			break;
		case TLS_CIPHER_ANON_DH_AES128_SHA:
			suite = "ADH-AES128-SHA";
			break;
		default:
			printf("TLS: Unsupported "
				   "cipher selection: %d", *c);
			return -1;
		}
		ret = snprintf(pos, end - pos, ":%s", suite);
		if (ret < 0 || ret >= end - pos)
			break;
		pos += ret;

		c++;
	}

	if(MSG_DEBUG) printf("OpenSSL: cipher suites: %s", buf + 1);

	if (SSL_set_cipher_list(conn->ssl_data, buf + 1) != 1) {
		printf("Cipher suite configuration failed");
		return -1;
	}

	return 0;
}
int tls_get_cipher(FAST_data *conn,
		   char *buf, size_t buflen)
{
	const char *name;
	if (conn == NULL || conn->ssl_data == NULL)
		return -1;

	name = SSL_get_cipher(conn->ssl_data);
	if (name == NULL)
		return -1;

	strncpy(buf, name, buflen-1);
	return 0;
}



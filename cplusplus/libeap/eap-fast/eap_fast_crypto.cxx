/* BEGIN_COPYRIGHT                                                        */ 
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */ 
/* Copyright (C) 2002-2007 Open Diameter Project                          */
/*                                                                        */ 
/* This program is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by   */
/* the Free Software Foundation; either version 2 of the License, or      */
/* (at your option) any later version.                                    */
/*                                                                        */           
/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */
/*                                                                        */
/* You should have received a copy of the GNU General Public License      */
/* along with this program; if not, write to the Free Software            */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307    */
/* USA.                                                                   */
/*                                                                        */
/* In addition, when you copy and redistribute some or the entire part of */
/* the source code of this software with or without modification, you     */
/* MUST include this copyright notice in each copy.                       */
/*                                                                        */
/* If you make any changes that are appeared to be useful, please send    */
/* sources that include the changed part to                               */
/* diameter-developers@lists.sourceforge.net so that we can reflect your  */
/* changes to one unified version of this software.                       */
/*                                                                        */
/* END_COPYRIGHT                                                          */
 
// $Id: eap_fast_crypto.cxx,v 1.5 2004/06/18 09:08:24 canelaman Exp $

// EAP-FAST cryptographic functions.
// Written by Rafael Marin Lopez (rafa@dif.um.es) (University of Murcia)

#include "eap_fast_session.hxx"
#include "eap_fast_mng.hxx" 
#include "base/aes_wrap.h"
#include "base/os.h"

#define OPAQUE_VALID 1
#define OPAQUE_INVALID 0


void EAPFASTCrypto_callbacks::cbfast_info(const SSL_data *s, ACE_INT32 where, ACE_INT32 ret)
{
   std::string str, state;
	ACE_INT32 w;
  
  	w = where & ~SSL_ST_MASK;
	if (w & SSL_ST_CONNECT) str="FAST_connect";
	else if (w & SSL_ST_ACCEPT) str="FAST_accept";
	else str="undefined";
   
	state = SSL_state_string_long(s);
	state = (state.length() == 0) ? state : "NULL";


  if (where & SSL_CB_LOOP) {
		EAP_LOG(LM_DEBUG, "%s: %s\n", str.c_str(), state.c_str());
	} else if (where & SSL_CB_HANDSHAKE_START) {
		EAP_LOG(LM_DEBUG, "%s: %s\n", str.c_str(), state.c_str());
	} else if (where & SSL_CB_HANDSHAKE_DONE) {
		EAP_LOG(LM_DEBUG, "%s: %s\n", str.c_str(), state.c_str());
	} else if (where & SSL_CB_ALERT) {
		str=(where & SSL_CB_READ)?"read":"write";
		EAP_LOG(LM_DEBUG,"FAST Alert %s:%s:%s\n", str.c_str(),
			SSL_alert_type_string_long(ret),
			SSL_alert_desc_string_long(ret));
	} else if (where & SSL_CB_EXIT) {
		if (ret == 0)
			EAP_LOG(LM_ERROR, "%s: failed in %s\n", str.c_str(), state.c_str());
		else if (ret < 0)
			EAP_LOG(LM_ERROR, "%s: error in %s\n", str.c_str(), state.c_str());
	}
 

}

/*
 * Before trusting a certificate, you must make sure that the certificate is
   'valid'. There are several steps that your application can take in
   determining if a certificate is valid. Commonly used steps are:

  1.Verifying the certificate's signature, and verifying that the certificate
    has been issued by a trusted Certificate Authority.

  2.Verifying that the certificate is valid for the present date (i.e. it is
    being presented within its validity dates).

  3.Verifying that the certificate has not been revoked by its issuing
    Certificate Authority, by checking with respect to a Certificate
    Revocation List (CRL).

  4.Verifying that the credentials presented by the certificate fulfill
    additional requirements specific to the application, such as with respect
    to access control lists or with respect to OCSP (Online Certificate Status
    Processing).
 */
/*
 * NOTE: This callback will be called multiple times based on the
 * depth of the root certificate chain
 */

ACE_INT32 EAPFASTCrypto_callbacks::cbfast_verify(ACE_INT32 ok, X509_store_certificate *ctx)
{

  ACE_Byte subject[256];
  ACE_Byte issuer[256];
  ACE_Byte buff[256];
  ACE_Byte *user_name;
  X509_certificate *client_cert;
  SSL_data *fast;
  ACE_INT32 err,depth;
  ACE_INT32 data_index(0);

   client_cert = X509_STORE_CTX_get_current_cert(ctx);
	err = X509_STORE_CTX_get_error(ctx);
	depth = X509_STORE_CTX_get_error_depth(ctx);


  if (err==10||err==1)ok=1;

	if(!ok)
		EAP_LOG(LM_ERROR,"--> verify error:num=%d:%s\n",err,X509_verify_cert_error_string(err));
	std::cout<<"continue\n";
	/*
	Catch too long Certificate chains
	*/

	/*
	 * Retrieve the pointer to the SSL of the connection currently treated
	 * and the application specific data stored into the SSL object.
	 */
	fast = (SSL_data *)X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	user_name = (ACE_Byte *)SSL_get_ex_data(fast, data_index);

	/*
	 * Get the Subject & Issuer
	 */
	subject[0] = issuer[0] = '\0';
	X509_NAME_oneline(X509_get_subject_name(client_cert), (char *)subject, 256);
	X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert), (char *)issuer, 256);

	/* Get the Common Name */
	X509_NAME_get_text_by_NID(X509_get_subject_name(client_cert),
             NID_commonName, (char *)buff, 256);

	switch (ctx->error) {

	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
		EAP_LOG(LM_ERROR, "issuer= %s\n", issuer);
		break;
	case X509_V_ERR_CERT_NOT_YET_VALID:
	case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
		EAP_LOG(LM_ERROR, "notBefore=");
		//ASN1_TIME_print(bio_err, X509_get_notBefore(ctx->current_cert));
		break;
	case X509_V_ERR_CERT_HAS_EXPIRED:
	case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
		EAP_LOG(LM_ERROR, "notAfter=");
		//ASN1_TIME_print(bio_err, X509_get_notAfter(ctx->current_cert));
		break;
    }
    return ok;

}

void EAPFASTCrypto_callbacks::cbfast_msg(int write_p, int msg_version, int content_type, const void *buf, size_t len, SSL *ssl, void *arg)
{


  EAPFAST_session_t  *state = (EAPFAST_session_t*)arg;
  
  EAPFAST_info_t *info = state->get_info();
  info->set_origin( (ACE_Byte)write_p);
  info->set_content_type((ACE_Byte)content_type);
  info->set_record_len(len);
  info->set_version(msg_version);

  if (content_type == 21) {
    info->set_alert_level(((const ACE_Byte*)buf)[0]);
    info->set_alert_description(((const ACE_Byte*)buf)[1]);
    info->set_handshake_type((ACE_Byte)0x00);

	} else if (content_type == 22) {
		info->set_handshake_type(((const ACE_Byte*)buf)[0]);
		info->set_alert_level(0x00);
		info->set_alert_description(0x00);

	}
  
	EAPFAST_fast_mng::fast_session_information(state);
}
ACE_INT32 EAPFASTCrypto_callbacks::cbfast_password(char *buf, ACE_INT32 num, ACE_INT32 rwflag, void *userdata)
{
      strcpy(buf, (char *)userdata);
      return(strlen((char *)userdata));
}
RSA_key *EAPFASTCrypto_callbacks::cbfast_rsa(SSL_data *s, ACE_INT32 is_export, ACE_INT32 keylength)
{
  static RSA_key *rsa_tmp=NULL;
  
	if (rsa_tmp == NULL)
	{
		EAP_LOG(LM_DEBUG, "Generating temp (%d bit) RSA key...", keylength);
		rsa_tmp=RSA_generate_key(keylength, RSA_F4, NULL, NULL);
	}
	return(rsa_tmp);
}


/*
 * FAST PRF from RFC 2246
 */
void EAPFASTCrypto_callbacks::P_hash(const Hash *evp_md,
		   const ACE_Byte *secret, ACE_UINT32 secret_len,
		   const ACE_Byte *seed,   ACE_UINT32 seed_len,
		   ACE_Byte *out, ACE_UINT32 out_len)
{
	HMAC_CTX ctx_a, ctx_out;
	ACE_Byte a[HMAC_MAX_MD_CBLOCK];
	ACE_UINT32 size;

	HMAC_CTX_init(&ctx_a);
	HMAC_CTX_init(&ctx_out);
	HMAC_Init_ex(&ctx_a, secret, secret_len, evp_md, NULL);
	HMAC_Init_ex(&ctx_out, secret, secret_len, evp_md, NULL);

	size = HMAC_size(&ctx_out);

	/* Calculate A(1) */
	HMAC_Update(&ctx_a, seed, seed_len);
	HMAC_Final(&ctx_a, a, NULL);

	while (1) {
		/* Calculate next part of output */
		HMAC_Update(&ctx_out, a, size);
		HMAC_Update(&ctx_out, seed, seed_len);

		/* Check if last part */
		if (out_len < size) {
			HMAC_Final(&ctx_out, a, NULL);
			memcpy(out, a, out_len);
			break;
		}

		/* Place digest in output buffer */
		HMAC_Final(&ctx_out, out, NULL);
		HMAC_Init_ex(&ctx_out, NULL, 0, NULL, NULL);
		out += size;
		out_len -= size;

		/* Calculate next A(i) */
		HMAC_Init_ex(&ctx_a, NULL, 0, NULL, NULL);
		HMAC_Update(&ctx_a, a, size);
		HMAC_Final(&ctx_a, a, NULL);
	}

	HMAC_CTX_cleanup(&ctx_a);
	HMAC_CTX_cleanup(&ctx_out);
	ACE_OS::memset(a, 0, sizeof(a));
}

void EAPFASTCrypto_callbacks::PRF(const ACE_Byte *secret, ACE_UINT32 secret_len,
		const ACE_Byte *seed,   ACE_UINT32 seed_len,
		ACE_Byte *out, ACE_Byte *buf, ACE_UINT32 out_len)
{
  ACE_UINT32 i;
  ACE_UINT32 len = (secret_len + 1) / 2;
  const ACE_Byte *s1 = secret;
  const ACE_Byte *s2 = secret + (secret_len - len);

  P_hash(EVP_md5(),  s1, len, seed, seed_len, out, out_len);
  P_hash(EVP_sha1(), s2, len, seed, seed_len, buf, out_len);

  for (i=0; i < out_len; i++) {
	        out[i] ^= buf[i];
	}
}

/*
 * Generate keys according to RFC 2716 
 */
AAAMessageBlock *EAPFASTCrypto_callbacks::eapfast_gen_mppe_keys(AAAMessageBlock *master_key,
                                                                                                                AAAMessageBlock *client_random,
                                                                                                                AAAMessageBlock *server_random)
{
	ACE_Byte out[2*EAPFAST_MPPE_KEY_LEN], buf[2*EAPFAST_MPPE_KEY_LEN];
	ACE_Byte seed[sizeof(EAPFAST_PRF_LABEL)-1 + 2*SSL3_RANDOM_SIZE];
	ACE_Byte *p = seed;
	ACE_OS::memcpy(p, EAPFAST_PRF_LABEL, sizeof(EAPFAST_PRF_LABEL)-1);
	p += sizeof(EAPFAST_PRF_LABEL)-1;
	ACE_OS::memcpy(p, client_random->base(), SSL3_RANDOM_SIZE); 
	p += SSL3_RANDOM_SIZE;
	ACE_OS::memcpy(p, server_random->base(), SSL3_RANDOM_SIZE); 

	PRF((const ACE_Byte *)master_key->base(), master_key->length(),
	    seed, sizeof(seed), out, buf, 2*EAPFAST_MPPE_KEY_LEN);

   AAAMessageBlock *msg = AAAMessageBlock::Acquire(2*EAPFAST_MPPE_KEY_LEN); 
   msg->copy((const char *)out,2*EAPFAST_MPPE_KEY_LEN);
   return msg; 
} 
  
int EAPFASTCrypto_callbacks::eap_fast_session_auth_ticket_cb(void *ctx, const u8 *ticket, size_t len,
				      const u8 *client_random,
				      const u8 *server_random,
				      u8 *master_secret)
{ 
	struct FAST_context_auth *data =(struct FAST_context_auth *) ctx;
	data -> opaquevalid = OPAQUE_INVALID;
#define TLS_RANDOM_LEN 32  
#define TLS_MASTER_SECRET_LEN 48
	u8 seed[2 * TLS_RANDOM_LEN];
	const u8 *pac_opaque;
	size_t pac_opaque_len;
	u8 *buf, *pos, *end, *pac_key = NULL; 
	time_t lifetime = 0; 
	struct os_time now;

	EAP_LOG(LM_DEBUG, "EAP-FAST: SessionTicket callback 	auth\n");

	if (len < 4 || WPA_GET_BE16(ticket) != PAC_TYPE_PAC_OPAQUE) {
		EAP_LOG(LM_INFO, "EAP-FAST: Ignore invalid "
			   "SessionTicket");
		return 0;
	}

	pac_opaque_len = WPA_GET_BE16(ticket + 2);
	pac_opaque = ticket + 4;
	EAP_LOG(LM_DEBUG, "[EAP-FAST]pac_opaque_len = %d\n", pac_opaque_len);
	if (pac_opaque_len < 8 || pac_opaque_len % 8 ||
	    pac_opaque_len > len - 4) {
		EAP_LOG(LM_INFO, "EAP-FAST: Ignore invalid PAC-Opaque "
			   "(len=%lu left=%lu)",
			   (unsigned long) pac_opaque_len,
			   (unsigned long) len); 
		return 0;
	}
	hexdump(MSG_DEBUG, "EAP-FAST: Received PAC-Opaque",
		    pac_opaque, pac_opaque_len);
 
	buf = (u8 *)malloc(pac_opaque_len - 8);
	if (buf == NULL) {
		EAP_LOG(LM_INFO, "EAP-FAST: Failed to allocate memory "
			   "for decrypting PAC-Opaque\n");
		return 0;
	}

	if (aes_unwrap(data->pac_opaque_encr, (pac_opaque_len - 8) / 8,
		       pac_opaque, buf) < 0) {
		EAP_LOG(LM_INFO, "EAP-FAST: Failed to decrypt "
			   "PAC-Opaque\n");
		free(buf);
		/*
		 * This may have been caused by server changing the PAC-Opaque
		 * encryption key, so just ignore this PAC-Opaque instead of
		 * failing the authentication completely. Provisioning can now
		 * be used to provision a new PAC.
		 */
		return -1;
	}

	end = buf + pac_opaque_len - 8;
	hexdump_key(MSG_DEBUG, "EAP-FAST: Decrypted PAC-Opaque",
			buf, end - buf);

	pos = buf;
	
	while (pos + 1 < end) {
		if (pos + 2 + pos[1] > end)
			break;

		switch (*pos) {
		case PAC_OPAQUE_TYPE_PAD:
			pos = end;
			break;
		case PAC_OPAQUE_TYPE_KEY:
			if (pos[1] != EAP_FAST_PAC_KEY_LEN) {
				EAP_LOG(LM_INFO, "EAP-FAST: Invalid "
					   "PAC-Key length %d", pos[1]);
				free(buf);
				return -1;
			}
			pac_key = pos + 2;
			hexdump_key(MSG_DEBUG, "EAP-FAST: PAC-Key from "
					"decrypted PAC-Opaque",
					pac_key, EAP_FAST_PAC_KEY_LEN);
			break;
		case PAC_OPAQUE_TYPE_LIFETIME:
			if (pos[1] != 4) {
				EAP_LOG(LM_INFO, "EAP-FAST: Invalid "
					   "PAC-Key lifetime length %d",
					   pos[1]);
				free(buf);
				return -1;
			}
			lifetime = WPA_GET_BE32(pos + 2);
			break;
		}

		pos += 2 + pos[1];
	}

	if (pac_key == NULL) {
		EAP_LOG(LM_INFO, "EAP-FAST: No PAC-Key included in "
			   "PAC-Opaque");
		free(buf);
		return -1;
	}

	if (os_get_time(&now) < 0 || lifetime <= 0 || now.sec > lifetime) {
		EAP_LOG(LM_INFO, "EAP-FAST: PAC-Key not valid anymore "
			   "(lifetime=%ld now=%ld)", lifetime, now.sec);
		free(buf);
		return 0;
	}

	if (lifetime - now.sec < PAC_KEY_REFRESH_TIME)
		data->send_new_pac = 1;

	/*  
	 * RFC 4851, Section 5.1:
	 * master_secret = T-PRF(PAC-Key, "PAC to master secret label hash", 
	 *                       server_random + client_random, 48)
	 */
	memcpy(seed, server_random, TLS_RANDOM_LEN);
	memcpy(seed + TLS_RANDOM_LEN, client_random, TLS_RANDOM_LEN);
	sha1_t_prf(pac_key, EAP_FAST_PAC_KEY_LEN,
		   "PAC to master secret label hash",
		   seed, sizeof(seed), master_secret, TLS_MASTER_SECRET_LEN);

	hexdump_key(MSG_DEBUG, "EAP-FAST: master_secret",
			master_secret, TLS_MASTER_SECRET_LEN);

	
	
	data -> opaquevalid = OPAQUE_VALID;
	free(buf);

	return 1;
}

int EAPFASTCrypto_callbacks::eap_fast_session_peer_ticket_cb(void *ctx, const u8 *ticket, size_t len,
				      const u8 *client_random,
				      const u8 *server_random,
				      u8 *master_secret)
{
	struct FAST_context_peer *data =(struct FAST_context_peer *) ctx;

	EAP_LOG(LM_DEBUG, "EAP-FAST: SessionTicket callback	peer\n");

	if (client_random == NULL || server_random == NULL ||
	    master_secret == NULL) {
		EAP_LOG(LM_INFO, "EAP-FAST: SessionTicket failed - fall "
			   "back to full TLS handshake"); 
		data->session_ticket_used = 0;
		/*if (data->provisioning_allowed) {
			printf("EAP-FAST: Try to provision a "
				   "new PAC-Key");
			data->provisioning = 1;
			data->current_pac = NULL;
		}*/
		return 0;
	}

	hexdump(MSG_DEBUG, "EAP-FAST: SessionTicket", ticket, len);

	if (data->pac_curr == NULL) {
		EAP_LOG(LM_INFO, "EAP-FAST: No PAC-Key available for "
			   "using SessionTicket");
		data->session_ticket_used = 0;
		return 0;
	}

	eap_fast_derive_master_secret(data->pac_curr->pac_key,
				      server_random, client_random,
				      master_secret);

	data->session_ticket_used = 1;

	return 1;
}





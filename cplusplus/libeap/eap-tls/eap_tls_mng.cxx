/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2004 Open Diameter Project                          */
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
/***************************************************************************
                          eap_tls_mng.cxx  -  description
                             -------------------
    begin                : jue mar 11 2004
    copyright            : (C) 2004 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/                

#include "eap_tls_mng.hxx"


ACE_INT32 EAPTLS_tls_mng::load_dh_params(TLS_context *ctx, std::string &file)
{
	DH_params *dh = NULL;
	BufferTLS *bio;

	if ((bio = BIO_new_file(file.c_str(), "r")) == NULL) {
	   EAP_LOG(LM_ERROR, "rlm_eap_tls: Unable to open DH file - %s", file.c_str());
		return -1;
	}

	dh = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
	BIO_free(bio);
	if (SSL_CTX_set_tmp_dh(ctx, dh) < 0) {
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Unable to set DH parameters");
		DH_free(dh);
		return -1;
	}

	DH_free(dh);
	return 0;

}

ACE_INT32 EAPTLS_tls_mng::generate_eph_rsa_key(TLS_context *ctx)
{
  RSA *rsa;
  rsa = RSA_generate_key(512, RSA_F4, NULL, NULL);
  if (!SSL_CTX_set_tmp_rsa(ctx, rsa)) {
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Couldn't set RSA key");
		return -1;
	}

  RSA_free(rsa);
	return 0;

}


/*
 * Create Global context SSL and use it in every new session
 * # Load the trusted CAs
 * # Load the Private key & the certificate
 * # Set the Context options & Verify options
 */

TLS_context *EAPTLS_tls_mng_auth::init_tls_ctx(EAPTLS_config &conf,ACE_INT32 session_id_context)
{
   	TLS_method *meth;
	   TLS_context *ctx;
	   ACE_INT32 verify_mode = 0;
	   ACE_INT32 ctx_options = 0;
	   ACE_INT32 type;

	/*
	 * Add all the default ciphers and message digests
	 * Create our context
	 */
	SSL_library_init();
	SSL_load_error_strings();

	meth = TLSv1_method();
	ctx = SSL_CTX_new(meth);

	/*
	 * Identify the type of certificates that needs to be loaded
	 */
	if (conf.get_file_type()) {
		type = SSL_FILETYPE_PEM;
	} else {
		type = SSL_FILETYPE_ASN1;
	}

	/* Load the CAs we trust */
  std::string ca_file = conf.get_ca_file();
  std::string ca_path = conf.get_ca_path();
   if (!(SSL_CTX_load_verify_locations(ctx, ca_file.c_str(), ca_path.c_str())) ||
			(!SSL_CTX_set_default_verify_paths(ctx))) {
		ERR_print_errors_fp(stderr);
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Error reading Trusted root CA list");
		return NULL;
	}
  SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(conf.get_ca_file().c_str()));
	/*
	 * Set the password to load private key
	 */
   std::string pkey = conf.get_private_key_password();
	if (pkey.length()!=0) {
		SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *)pkey.c_str());
		SSL_CTX_set_default_passwd_cb(ctx,EAPTLSCrypto_callbacks::cbtls_password);
	}

	/* Load our keys and certificates*/
  std::string certFile = conf.get_certificate_file();
	if (!(SSL_CTX_use_certificate_file(ctx, certFile.c_str(), type))) {
		ERR_print_errors_fp(stderr);
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Error reading certificate file");
		return NULL;
	}
   std::string pkeyFile =  conf.get_private_key_file();
	if (!(SSL_CTX_use_PrivateKey_file(ctx, pkeyFile.c_str(), type))) {
		ERR_print_errors_fp(stderr);
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Error reading private key file");
		return NULL;
	}

	/*
	 * Check if the loaded private key is the right one
	 */
	if (!SSL_CTX_check_private_key(ctx)) {
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Private key does not match the certificate public key");
		return NULL;
	}

	/*
	 * Set ctx_options
	 */
	ctx_options |= SSL_OP_NO_SSLv2;
   ctx_options |= SSL_OP_NO_SSLv3;
	/*
       SSL_OP_SINGLE_DH_USE must be used in order to prevent
	   small subgroup attacks and forward secrecy. Always using
       SSL_OP_SINGLE_DH_USE has an impact on the computer time
       needed during negotiation, but it is not very large.
	 */
   ctx_options |= SSL_OP_SINGLE_DH_USE;
	SSL_CTX_set_options(ctx, ctx_options);

	/*
	 * TODO: Set the RSA & DH
	SSL_CTX_set_tmp_rsa_callback(ctx, cbtls_rsa);
	SSL_CTX_set_tmp_dh_callback(ctx, cbtls_dh);
	 */

	/*
	 * set the message callback to identify the type of message.
	 * For every new session, there can be a different callback argument
	SSL_CTX_set_msg_callback(ctx, cbtls_msg);
	 */

	/* Set Info callback */
	SSL_CTX_set_info_callback(ctx, EAPTLSCrypto_callbacks::cbtls_info);

	/*
	 * Set verify modes
	 * Always verify the peer certificate
	 */
	verify_mode |= SSL_VERIFY_PEER;
	verify_mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
	verify_mode |= SSL_VERIFY_CLIENT_ONCE;
	SSL_CTX_set_verify(ctx, verify_mode, EAPTLSCrypto_callbacks::cbtls_verify);

	if (conf.get_verify_depth()) {
		SSL_CTX_set_verify_depth(ctx, conf.get_verify_depth());
	}

	/* Load randomness */
  std::string random_file = conf.get_random_file();
	if (!(RAND_load_file(random_file.c_str(), 1024*1024))) {
		ERR_print_errors_fp(stderr);
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Error loading randomness");
		return NULL;
	}

   SSL_CTX_set_session_id_context(ctx,(const unsigned char*)&session_id_context,sizeof session_id_context);
	return ctx;
}

/*
 * Create Global context SSL and use it in every new session
 * # Load the trusted CAs
 * # Load the Private key & the certificate
 * # Set the Context options & Verify options
 */

TLS_context *EAPTLS_tls_mng_peer::init_tls_ctx(EAPTLS_config &conf)
{
   	TLS_method *meth;
	   TLS_context *ctx;
	   ACE_INT32 verify_mode = 0;
	   ACE_INT32 ctx_options = 0;
	   ACE_INT32 type;

	/*
	 * Add all the default ciphers and message digests
	 * Create our context
	 */
	SSL_library_init();
	SSL_load_error_strings();

	meth = TLSv1_method();
	ctx = SSL_CTX_new(meth);

	/*
	 * Identify the type of certificates that needs to be loaded
	 */
	if (conf.get_file_type()) {
		type = SSL_FILETYPE_PEM;
	} else {
		type = SSL_FILETYPE_ASN1;
	}

	/* Load the CAs we trust */
  std::string ca_file = conf.get_ca_file();
  std::string ca_path = conf.get_ca_path();
	if (!(SSL_CTX_load_verify_locations(ctx, ca_file.c_str(), ca_path.c_str())) ||
			(!SSL_CTX_set_default_verify_paths(ctx))) {
		ERR_print_errors_fp(stderr);
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Error reading Trusted root CA list");
		return NULL;
	}
  SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(conf.get_ca_file().c_str()));

	/*
	 * Set the password to load private key
	 */
   std::string pkey = conf.get_private_key_password();
	if (pkey.length()!=0) {
		SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *)pkey.c_str());
		SSL_CTX_set_default_passwd_cb(ctx, EAPTLSCrypto_callbacks::cbtls_password);
	}

	/* Load our keys and certificates*/
  std::string certFile = conf.get_certificate_file();
	if (!(SSL_CTX_use_certificate_file(ctx, certFile.c_str(), type))) {
		ERR_print_errors_fp(stderr);
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Error reading certificate file");
		return NULL;
	}
   std::string pkeyFile =  conf.get_private_key_file();
	if (!(SSL_CTX_use_PrivateKey_file(ctx, pkeyFile.c_str(), type))) {
		ERR_print_errors_fp(stderr);
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Error reading private key file");
		return NULL;
	}

	/*
	 * Check if the loaded private key is the right one
	 */
	if (!SSL_CTX_check_private_key(ctx)) {
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Private key does not match the certificate public key");
		return NULL;
	}

	/*
	 * Set ctx_options
	 */
	ctx_options |= SSL_OP_NO_SSLv2;
   ctx_options |= SSL_OP_NO_SSLv3;
	/*
       SSL_OP_SINGLE_DH_USE must be used in order to prevent
	   small subgroup attacks and forward secrecy. Always using
       SSL_OP_SINGLE_DH_USE has an impact on the computer time
       needed during negotiation, but it is not very large.
	 */
   ctx_options |= SSL_OP_SINGLE_DH_USE;
	SSL_CTX_set_options(ctx, ctx_options);

	/*
	 * TODO: Set the RSA & DH
	SSL_CTX_set_tmp_rsa_callback(ctx, cbtls_rsa);
	SSL_CTX_set_tmp_dh_callback(ctx, cbtls_dh);
	 */

	/*
	 * set the message callback to identify the type of message.
	 * For every new session, there can be a different callback argument
	SSL_CTX_set_msg_callback(ctx, cbtls_msg);
	 */

	/* Set Info callback */
	SSL_CTX_set_info_callback(ctx, EAPTLSCrypto_callbacks::cbtls_info);

	/*
	 * Set verify modes
	 * Always verify the peer certificate
	 */
	verify_mode |= SSL_VERIFY_PEER;
	verify_mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
	//verify_mode |= SSL_VERIFY_CLIENT_ONCE;
	SSL_CTX_set_verify(ctx, verify_mode, EAPTLSCrypto_callbacks::cbtls_verify);

	if (conf.get_verify_depth()) {
		SSL_CTX_set_verify_depth(ctx, conf.get_verify_depth());
	}

	/* Load randomness */
  std::string random_file = conf.get_random_file();
	if (!(RAND_load_file(random_file.c_str(), 1024*1024))) {
		ERR_print_errors_fp(stderr);
		EAP_LOG(LM_ERROR, "rlm_eap_tls: Error loading randomness");
		return NULL;
	}

	return ctx;
}

void EAPTLS_tls_mng::tls_check_state(TLS_data *s, ACE_INT32 ret)
{
   ACE_INT32 e;

	ERR_print_errors_fp(stderr);
	e = SSL_get_error(s, ret);

	EAP_LOG(LM_ERROR, " Error code is ..... %d\n", e);

	switch(e) {
		/* These seem to be harmless and already "dealt with" by our
		 * non-blocking environment. NB: "ZERO_RETURN" is the clean
		 * "error" indicating a successfully closed SSL tunnel. We let
		 * this happen because our IO loop should not appear to have
		 * broken on this condition - and outside the IO loop, the
		 * "shutdown" state is checked. */
	case SSL_ERROR_NONE:
       EAP_LOG(LM_ERROR,"No TLS error!...\n");
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
	case SSL_ERROR_WANT_X509_LOOKUP:
	case SSL_ERROR_ZERO_RETURN:
		EAP_LOG(LM_ERROR, " SSL Error ..... %d\n", e);
		return;
		/* These seem to be indications of a genuine error that should
		 * result in the SSL tunnel being regarded as "dead". */
	case SSL_ERROR_SYSCALL:
	case SSL_ERROR_SSL:
		EAP_LOG(LM_ERROR, " Error in SSL ..... %d\n", e);
		SSL_set_app_data(s, (char *)1);
		return;
	default:
		break;
	}
	EAP_LOG(LM_ERROR, "Unknown Error ..... %d\n", e);
	/* For any other errors that (a) exist, and (b) crop up - we need to
	 * interpret what to do with them - so "politely inform" the caller that
	 * the code needs updating here. */
	//abort(); Commented by open diameter.
}


ACE_INT32 EAPTLS_tls_mng::tls_handshake_recv(EAPTLS_session_t *ssn)
{
   ACE_INT32 err;

	ACE_INT32 err_code = 0;

   BIO_write(ssn->get_bufferTLS_in(), (const void *)(ssn->get_dirty_in()->base()), ssn->get_dirty_in()->length());
   err = SSL_read(ssn->get_tls_data(), (void *)(ssn->get_clean_out()->base()), MAX_RECORD_SIZE); 
	if (err > 0) {
		ssn->get_clean_out()->wr_ptr(err);
	} else {
		EAP_LOG(LM_DEBUG, "rlm_eap_tls: SSL_read Error");
		tls_check_state(ssn->get_tls_data(), err);
	err_code = SSL_get_error(ssn->get_tls_data(), err);
	} 

	/* Some Extra STATE information for easy debugging */
	/*
	if (SSL_is_init_finished(ssn->ssl)) {
		printf("SSL Connection Established\n");
	}
   	if (SSL_in_init(ssn->ssl)) {
		printf("In SSL Handshake Phase\n");
	}
   	if (SSL_in_before(ssn->ssl)) {
		printf("Before SSL Handshake Phase\n");
	}
   	if (SSL_in_accept_init(ssn->ssl)) {
		printf("In SSL Accept mode \n");
	}
   	if (SSL_in_connect_init(ssn->ssl)) {
		printf("In SSL Connect mode \n");
	}
	*/
	if (ssn->get_info()->get_content_type() != application_data)
  {
      ssn->get_dirty_out()->reset();
      err = BIO_read(ssn->get_bufferTLS_out(),(void *)(ssn->get_dirty_out()->base()), MAX_RECORD_SIZE);
		if (err > 0) {
      EAP_LOG(LM_ERROR,"rlm_eap_tls: Readed bytes %d\n",err);
			ssn->get_dirty_out()->wr_ptr(err);
		} else {
			EAP_LOG(LM_ERROR, "rlm_eap_tls: BIO_read Error");
			tls_check_state(ssn->get_tls_data(), err);
         ssn->get_dirty_in()->reset();
			if (err_code == SSL_ERROR_SYSCALL)
			{
				return EAPTLS_tls_mng::StAlertReceive;
			}
		}
	} else {
		EAP_LOG(LM_DEBUG, "rlm_eap_tls: Application Data");
		/* Its clean application data, do whatever we want */
      ssn->get_clean_out()->reset();
	}

	/* We are done with dirty_in, reinitialize it */
	ssn->get_dirty_in()->reset();
	if (err_code == SSL_ERROR_SYSCALL)
	{
		return EAPTLS_tls_mng::StAlertSend;
	}
	else
	{
		return EAPTLS_tls_mng::StOk;
	}
}


void EAPTLS_tls_mng::tls_session_information(EAPTLS_session_t *tls_session)
{
	std::string str_write_p, str_version, str_content_type(""), str_details1(""), str_details2("");
   EAPTLS_info_t *info = tls_session->get_info();

	str_write_p = info->get_origin() ? ">>>" : "<<<";

	switch (info->get_version())
	{
	case SSL2_VERSION:
		str_version = "SSL 2.0";
		break;
	case SSL3_VERSION:
		str_version = "SSL 3.0 ";
		break;
	case TLS1_VERSION:
		str_version = "TLS 1.0 ";
		break;
	default:
		str_version = "???";
	}

	if (info->get_version() == SSL3_VERSION ||
	    info->get_version() == TLS1_VERSION) {
		switch (info->get_content_type()) {
		case 20:
			str_content_type = "ChangeCipherSpec";
			break;
		case 21:
			str_content_type = "Alert";
			break;
		case 22:
			str_content_type = "Handshake";
			break;
		}

		if (info->get_content_type() == 21) { /* Alert */
			str_details1 = ", ???";

			if (info->get_record_len() == 2) {

				switch (info->get_alert_level()) {
				case 1:
					str_details1 = ", warning";
					break;
				case 2:
					str_details1 = ", fatal";
					break;
				}

				str_details2 = " ???";
				switch (tls_session->get_info()->get_alert_description()) {
				case 0:
					str_details2 = " close_notify";
					break;
				case 10:
					str_details2 = " unexpected_message";
					break;
				case 20:
					str_details2 = " bad_record_mac";
					break;
				case 21:
					str_details2 = " decryption_failed";
					break;
				case 22:
					str_details2 = " record_overflow";
					break;
				case 30:
					str_details2 = " decompression_failure";
					break;
				case 40:
					str_details2 = " handshake_failure";
					break;
				case 42:
					str_details2 = " bad_certificate";
					break;
				case 43:
					str_details2 = " unsupported_certificate";
					break;
				case 44:
					str_details2 = " certificate_revoked";
					break;
				case 45:
					str_details2 = " certificate_expired";
					break;
				case 46:
					str_details2 = " certificate_unknown";
					break;
				case 47:
					str_details2 = " illegal_parameter";
					break;
				case 48:
					str_details2 = " unknown_ca";
					break;
				case 49:
					str_details2 = " access_denied";
					break;
				case 50:
					str_details2 = " decode_error";
					break;
				case 51:
					str_details2 = " decrypt_error";
					break;
				case 60:
					str_details2 = " export_restriction";
					break;
				case 70:
					str_details2 = " protocol_version";
					break;
				case 71:
					str_details2 = " insufficient_security";
					break;
				case 80:
					str_details2 = " internal_error";
					break;
				case 90:
					str_details2 = " user_canceled";
					break;
				case 100:
					str_details2 = " no_renegotiation";
					break;
				}
			}
		}

		if (info->get_content_type() == 22) /* Handshake */
		{
			str_details1 = "???";

			if (info->get_record_len() > 0)
			switch (info->get_handshake_type())
			{
			case 0:
				str_details1 = ", HelloRequest";
				break;
			case 1:
				str_details1 = ", ClientHello";
				break;
			case 2:
				str_details1 = ", ServerHello";
				break;
			case 11:
				str_details1 = ", Certificate";
				break;
			case 12:
				str_details1 = ", ServerKeyExchange";
				break;
			case 13:
				str_details1 = ", CertificateRequest";
				break;
			case 14:
				str_details1 = ", ServerHelloDone";
				break;
			case 15:
				str_details1 = ", CertificateVerify";
				break;
			case 16:
				str_details1 = ", ClientKeyExchange";
				break;
			case 20:
				str_details1 = ", Finished";
				break;
			}
		}
	}

  char *info_description = new char[256];
  ACE_OS::sprintf(info_description, "%s %s%s [length %04lx]%s ... %s\n",str_write_p.c_str(), str_version.c_str(), str_content_type.c_str(),(unsigned long)info->get_record_len(), str_details1.c_str(), str_details2.c_str());
  std::string str_info = info_description;
  info->set_info_description(str_info);
  delete info_description;
  EAP_LOG(LM_ERROR,"rlm_eap_tls: %s\n", info->get_info_description().c_str());
}



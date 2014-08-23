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
/***************************************************************************
                          eap_fast_session.hxx  -  description
                             -------------------
    begin                : mar abr 6 2007
    copyright            : (C) 2007 by 
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

#ifndef  __EAP_FAST_SESSION_H__
#define  __EAP_FAST_SESSION_H__

#include "eap_fast.hxx"
#include "base/openssl_patches.h"

#include <openssl/hmac.h>
#define EAPFAST_PRF_LABEL        "client EAP encryption"
#define EAPFAST_MPPE_KEY_LEN     32

typedef X509_STORE_CTX  X509_store_certificate;
typedef X509 X509_certificate;
typedef RSA RSA_key;
typedef EVP_MD Hash;

class EAPFASTCrypto_callbacks
{
  public:
    EAPFASTCrypto_callbacks(){};
    virtual ~EAPFASTCrypto_callbacks(){};
    static void cbfast_info(const SSL_data *s, ACE_INT32 where, ACE_INT32 ret);
    static ACE_INT32 cbfast_verify(ACE_INT32 ok, X509_store_certificate *ctx);
    static void cbfast_msg(int write_p, int msg_version, int content_type, const void *buf, size_t len, SSL *ssl, void *arg);
    static int eap_fast_session_auth_ticket_cb(void *ctx, const u8 *ticket, size_t len,
				      const u8 *client_random,
				      const u8 *server_random,
				      u8 *master_secret);
    static int eap_fast_session_peer_ticket_cb(void *ctx, const u8 *ticket, size_t len,
				      const u8 *client_random,
				      const u8 *server_random,
				      u8 *master_secret);
    static ACE_INT32 cbfast_password(char *buf, ACE_INT32 num, ACE_INT32 rwflag, void *userdata);
    static RSA_key *cbfast_rsa(SSL_data *s, ACE_INT32 is_export, ACE_INT32 keylength);
    static void P_hash(const Hash *evp_md,
                                  const ACE_Byte *secret, ACE_UINT32 secret_len,
                                  const ACE_Byte *seed,   ACE_UINT32 seed_len,
                                  ACE_Byte *out, ACE_UINT32 out_len);
    static void PRF(const ACE_Byte *secret, ACE_UINT32 secret_len,
    const ACE_Byte *seed,   ACE_UINT32 seed_len,
		ACE_Byte *out, ACE_Byte *buf, ACE_UINT32 out_len);

    static AAAMessageBlock *eapfast_gen_mppe_keys(AAAMessageBlock *mk,
                                                                                    AAAMessageBlock *client_random,
                                                                                    AAAMessageBlock *server_random);
};

class EAPFAST_session_t
{
  public:
   
  EAPFAST_session_t()
  {
    // this->session_init(false);
     this->eapfast = NULL;
      this->cb = new EAPFASTCrypto_callbacks();
     info=new EAPFAST_info_t();
     this->clean_in = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->clean_out = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->dirty_in = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->dirty_out = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->master_key = NULL;
     this->client_random = AAAMessageBlock::Acquire(SSL3_RANDOM_SIZE);
     this->server_random = AAAMessageBlock::Acquire(SSL3_RANDOM_SIZE);
  };

  EAPFAST_session_t(EAPFAST_fast_t *eapfast)
  {
     this->eapfast=eapfast;
     this->cb = new EAPFASTCrypto_callbacks();
     //this->session_init(false);
     info=new EAPFAST_info_t();
     this->clean_in = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->clean_out = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->dirty_in = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->dirty_out = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->master_key = NULL;
     this->client_random = AAAMessageBlock::Acquire(SSL3_RANDOM_SIZE);
     this->server_random = AAAMessageBlock::Acquire(SSL3_RANDOM_SIZE);
  };

 virtual ~EAPFAST_session_t()
 {
     if (info != NULL) delete info;
     if (cb != NULL)  delete cb;
     clean_in->Release();
     clean_out->Release();
     dirty_in->Release(); 
     dirty_out->Release();
     client_random->Release(); 
     server_random->Release();
     if (master_key != NULL) master_key->Release();   
 };

 virtual EAPFAST_session_t *session_init(bool resume)
  {

      this->first_fragment=(this->eapfast->get_config()->get_fragment_size() != 0);
      this->fast_msg_length=0;
      this->fast = (FAST_data *)malloc(sizeof( FAST_data)); 
      this->fast->session_ticket_cb_ctx = NULL;
      this->fast->session_ticket = NULL;
      this->fast->session_ticket_len = 0;
      this->fast->result = EAP_TLV_RESULT_NONE;

	TLS_context *tls;
	if(this->eapfast->get_fast_auth_context()==NULL){//this is a peer session
		tls=this->eapfast->get_fast_peer_context()->tls_ctx;
	} 
	else{ //this is a auth session
		tls=this->eapfast->get_fast_auth_context()->tls_ctx;
	}

      this->fast->ssl_data = SSL_new(tls);  

      if (this->fast->ssl_data == NULL) {
         EAP_LOG(LM_ERROR, "rlm_eap_fast: Error creating new SSL");
         ERR_print_errors_fp(stderr);
         return NULL;
      }

      SSL_set_app_data(this->fast->ssl_data,this->fast);

      this->fast_in=BIO_new(BIO_s_mem());
      this->fast_out=BIO_new(BIO_s_mem());
      SSL_set_bio(this->fast->ssl_data, fast_in, fast_out); 

      SSL_set_msg_callback(this->fast->ssl_data, cb->cbfast_msg);
      SSL_set_msg_callback_arg(this->fast->ssl_data, this);
      SSL_set_info_callback(this->fast->ssl_data, cb->cbfast_info);
      return this;
  };
  virtual void session_close()
  {
    if(this->fast)
    {

      this->sess=SSL_get1_session(this->fast->ssl_data);
      if (this->master_key != NULL) 
	master_key->Release();


      memcpy(this->simck,this->fast->simck,EAP_FAST_SIMCK_LEN);

	size_t keyLen = 0;
      u8 *msk = eap_fast_get_msk(this->simck,&keyLen);

     this->master_key = AAAMessageBlock::Acquire(keyLen);
     this->master_key->copy((const char *)msk,keyLen);
     this->server_random->copy((const char *)(this->fast->ssl_data->s3->server_random),SSL3_RANDOM_SIZE);
     this->client_random->copy((const char *)(this->fast->ssl_data->s3->client_random),SSL3_RANDOM_SIZE);

      SSL_shutdown(this->fast->ssl_data);
      SSL_free(this->fast->ssl_data);
    }
    clean_in->reset();
    clean_out->reset();
    dirty_in->reset();
    dirty_out->reset();
  } ;


  void set_fast_data(FAST_data *fast) { this->fast = fast;};
  void set_info(EAPFAST_info_t *info) { this->info = info;};
  void set_bufferFAST_in(BufferFAST *fast_in) { this->fast_in = fast_in;};
  void set_bufferFAST_out(BufferFAST *fast_out){ this->fast_out = fast_out;};
  void set_clean_in(EAPFAST_record_t *record){ this->clean_in->reset(); this->clean_in->copy(record->rd_ptr(),record->length());};
  void set_clean_out(EAPFAST_record_t *record){ this->clean_out->reset(); this->clean_out->copy(record->rd_ptr(),record->length());};
  void append_dirty_in(EAPFAST_record_t *record)
  {
    if (record != NULL) this->dirty_in->copy(record->rd_ptr(),record->length());
  };
  void set_dirty_in(EAPFAST_record_t *record)
  {
    this->dirty_in->reset();
    append_dirty_in(record);
  };
  void set_dirty_out(EAPFAST_record_t *record){this->dirty_out->reset(); this->dirty_out->copy(record->rd_ptr(),record->length());};
  void set_first_fragment(bool first_fragment) {this->first_fragment = first_fragment;};
  void set_fast_msg_length(ACE_UINT32 fast_msg_length) {this->fast_msg_length = fast_msg_length;};
  void set_length_to_send(ACE_UINT32 length_to_send) {this->length_to_send = length_to_send;};
  void set_flags_to_send(ACE_Byte flags_to_send) {this->flags_to_send = flags_to_send;};
  void set_fragments(AAAMessageBlock *fragments) {this->fragments=fragments;};

  FAST_data *get_fast_data() { return fast;};  
  //Information to derive new session keys
  AAAMessageBlock *get_master_key() {return this->master_key;};
  AAAMessageBlock *get_client_random() {return this->client_random;};             
  AAAMessageBlock *get_server_random(){return this->server_random;};
  u8* get_simck(){return this->simck;};
  //-------------------------------------------------                   
  EAPFAST_info_t *get_info() { return info;};
  BufferFAST *get_bufferFAST_in() {return fast_in;};
  BufferFAST *get_bufferFAST_out() { return fast_out;}; 
  EAPFAST_record_t  *get_clean_in() {return clean_in;};
  EAPFAST_record_t  *get_clean_out() {return this->clean_out;};  
  EAPFAST_record_t  *get_dirty_in() {return this->dirty_in;};
  EAPFAST_record_t  *get_dirty_out() {return this->dirty_out;};
  ACE_UINT32 get_fast_msg_length() {return fast_msg_length;};
  ACE_UINT32 get_length_to_send() {return length_to_send;};
  AAAMessageBlock *restore_fragments(){return fragments;};
  ACE_Byte get_flags_to_send() {return flags_to_send;}; 
  bool if_first_fragment() {return first_fragment;};
  ACE_UINT32 get_fragment_size() {return eapfast->get_config()->get_fragment_size();}; //return FRAGMENT size
  std::string &get_auth_id() {return eapfast->get_config()->get_auth_id();}
  bool if_length_included() {return eapfast->get_config()->get_include_length();};
  struct FAST_context_peer * get_peer_context() {return eapfast->get_fast_peer_context();}
  struct FAST_context_auth * get_auth_context() {return eapfast->get_fast_auth_context();}

  protected:
    EAPFAST_fast_t *eapfast;
    FAST_data *fast;
    EAPFAST_info_t *info;
    BufferFAST *fast_in;
    BufferFAST *fast_out;
    EAPFAST_record_t *clean_in;
    EAPFAST_record_t *clean_out;
    EAPFAST_record_t *dirty_in;
    EAPFAST_record_t *dirty_out;
    EAPFASTCrypto_callbacks *cb;
    //Used for fragmentation
    ACE_UINT32 fast_msg_length;
    ACE_UINT32 length_to_send;
    ACE_Byte flags_to_send;
    bool first_fragment;
    AAAMessageBlock *master_key;
    AAAMessageBlock *client_random;
    AAAMessageBlock *server_random;
    AAAMessageBlock *fragments;
    u8 simck[EAP_FAST_SIMCK_LEN];
    SSL_SESSION *sess;
   
};

class EAPFAST_session_t_peer : public EAPFAST_session_t
{
     public:
        EAPFAST_session_t_peer():EAPFAST_session_t(){this->sess=NULL;};
        EAPFAST_session_t_peer(EAPFAST_fast_t *eapfast):EAPFAST_session_t(eapfast){this->sess=NULL;};
        virtual EAPFAST_session_t_peer *session_init(bool resume)
        {
             ACE_INT32 verify_mode = 0; 
             EAPFAST_session_t::session_init(resume);
		 if (tls_connection_peer_set_session_ticket_cb((this->eapfast->get_fast_peer_context())->tls_ctx, 
						 this->fast,
						 EAPFASTCrypto_callbacks::eap_fast_session_peer_ticket_cb,
						 this->eapfast->get_fast_peer_context()) < 0) {
		printf("EAP-FAST: Failed to set SessionTicket "
			   "callback\n");
		//eap_fast_reset(sm, data);
		return NULL;
	      }	
             /* Always verify the peer certificate */
	          verify_mode |= SSL_VERIFY_PEER;
          	 verify_mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
		
	       	 SSL_set_verify(this->fast->ssl_data, verify_mode, cb->cbfast_verify);
          	/* In Client mode we only connect.  */
	          SSL_set_connect_state(this->fast->ssl_data);
             if (resume && (this->sess !=NULL)) SSL_set_session(this->fast->ssl_data,this->sess);
             return this;
        }
}; 
 
class EAPFAST_session_t_auth : public EAPFAST_session_t
{
    public:
        EAPFAST_session_t_auth():EAPFAST_session_t(){this->sess=NULL;};
        EAPFAST_session_t_auth(EAPFAST_fast_t *eapfast):EAPFAST_session_t(eapfast){this->sess=NULL;};

        virtual EAPFAST_session_t_auth *session_init(bool resume)
        {
             ACE_INT32 verify_mode = 0;
             EAPFAST_session_t::session_init(resume); 
	     u8 ciphers[5] = {
		TLS_CIPHER_ANON_DH_AES128_SHA,
		TLS_CIPHER_AES128_SHA,
		TLS_CIPHER_RSA_DHE_AES128_SHA,
		TLS_CIPHER_RC4_SHA,
		TLS_CIPHER_NONE
		};
	     if (tls_connection_set_cipher_list(this->fast,ciphers) < 0) {
		printf("EAP-FAST: Failed to set TLS cipher "  
			   "suites");
		return NULL;
	     }

             if (tls_connection_auth_set_session_ticket_cb((this->eapfast->get_fast_auth_context())->tls_ctx, 
						 this->fast,
						 EAPFASTCrypto_callbacks::eap_fast_session_auth_ticket_cb,
						 this->eapfast->get_fast_auth_context()) < 0) {
		printf("EAP-FAST: Failed to set SessionTicket                 4           "
			   "callback"); 
		return NULL;
	      }	
             /* Always verify the peer certificate */
	          verify_mode |= SSL_VERIFY_PEER;
           	 verify_mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
	          verify_mode |= SSL_VERIFY_CLIENT_ONCE;
			
	          SSL_set_verify(this->fast->ssl_data, verify_mode, cb->cbfast_verify);
	          /* In Server mode we only accept.  */
            SSL_set_accept_state(this->fast->ssl_data);
            return this;
        }
};

#endif


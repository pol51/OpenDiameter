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
                          eap_tls_session.hxx  -  description
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

#ifndef  __EAP_TLS_SESSION_H__
#define  __EAP_TLS_SESSION_H__

#include "eap_tls.hxx"

#include <openssl/hmac.h>
#define EAPTLS_PRF_LABEL        "client EAP encryption"
#define EAPTLS_MPPE_KEY_LEN     32

typedef X509_STORE_CTX  X509_store_certificate;
typedef X509 X509_certificate;
typedef RSA RSA_key;
typedef EVP_MD Hash;

class EAPTLSCrypto_callbacks
{
  public:
    EAPTLSCrypto_callbacks(){};
    virtual ~EAPTLSCrypto_callbacks(){};
    static void cbtls_info(const TLS_data *s, ACE_INT32 where, ACE_INT32 ret);
    static ACE_INT32 cbtls_verify(ACE_INT32 ok, X509_store_certificate *ctx);
    static void cbtls_msg(ACE_INT32 write_p, ACE_INT32 msg_version, ACE_INT32 content_type, const void *buf, ACE_UINT32 len, TLS_data *ssl, void *arg);
    static ACE_INT32 cbtls_password(char *buf, ACE_INT32 num, ACE_INT32 rwflag, void *userdata);
    static RSA_key *cbtls_rsa(TLS_data *s, ACE_INT32 is_export, ACE_INT32 keylength);
    static void P_hash(const Hash *evp_md,
                                  const ACE_Byte *secret, ACE_UINT32 secret_len,
                                  const ACE_Byte *seed,   ACE_UINT32 seed_len,
                                  ACE_Byte *out, ACE_UINT32 out_len);
    static void PRF(const ACE_Byte *secret, ACE_UINT32 secret_len,
    const ACE_Byte *seed,   ACE_UINT32 seed_len,
		ACE_Byte *out, ACE_Byte *buf, ACE_UINT32 out_len);

    static AAAMessageBlock *eaptls_gen_mppe_keys(AAAMessageBlock *mk,
                                                                                    AAAMessageBlock *client_random,
                                                                                    AAAMessageBlock *server_random);
};

class EAPTLS_session_t
{
  public:
   
  EAPTLS_session_t()
  {
    // this->session_init(false);
      this->cb = new EAPTLSCrypto_callbacks();
     info=new EAPTLS_info_t();
     this->clean_in = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->clean_out = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->dirty_in = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->dirty_out = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->master_key = NULL;
     this->client_random = AAAMessageBlock::Acquire(SSL3_RANDOM_SIZE);
     this->server_random = AAAMessageBlock::Acquire(SSL3_RANDOM_SIZE);
  };

  EAPTLS_session_t(EAPTLS_tls_t *eaptls)
  {
     this->eaptls=eaptls;
     this->cb = new EAPTLSCrypto_callbacks();
     //this->session_init(false);
     info=new EAPTLS_info_t();
     this->clean_in = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->clean_out = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->dirty_in = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->dirty_out = AAAMessageBlock::Acquire(MAX_RECORD_SIZE);
     this->master_key = NULL;
     this->client_random = AAAMessageBlock::Acquire(SSL3_RANDOM_SIZE);
     this->server_random = AAAMessageBlock::Acquire(SSL3_RANDOM_SIZE);
  };

 virtual ~EAPTLS_session_t()
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

 virtual EAPTLS_session_t *session_init(bool resume)
  {

      this->first_fragment=(this->eaptls->get_config()->get_fragment_size() != 0);
      this->tls_msg_length=0;
     
      if ((this->tls = SSL_new(eaptls->get_tls_context())) == NULL) {
         EAP_LOG(LM_ERROR, "rlm_eap_tls: Error creating new SSL");
         ERR_print_errors_fp(stderr);
         return NULL;
      }

      SSL_set_app_data(this->tls, NULL);

      this->tls_in=BIO_new(BIO_s_mem());
      this->tls_out=BIO_new(BIO_s_mem());
      SSL_set_bio(this->tls, tls_in, tls_out);

      SSL_set_msg_callback(this->tls, cb->cbtls_msg);
      SSL_set_msg_callback_arg(this->tls, this);
      SSL_set_info_callback(this->tls, cb->cbtls_info);

      return this;
  };
  virtual void session_close()
  {
    if(this->tls)
    {
      this->sess=SSL_get1_session(this->tls);
      if (this->master_key != NULL) master_key->Release();
      this->master_key = AAAMessageBlock::Acquire(this->sess->master_key_length);
      
      this->master_key->copy((const char *)this->sess->master_key,this->sess->master_key_length);
      this->server_random->copy((const char *)(this->tls->s3->server_random),SSL3_RANDOM_SIZE);
      this->client_random->copy((const char *)(this->tls->s3->client_random),SSL3_RANDOM_SIZE);

      SSL_shutdown(this->tls);
      SSL_free(this->tls);
    }
    #if 0
    /*
     * WARNING: SSL_free seems to decrement the reference counts already,
     * 	so doing this might crash the application.
     */
	 if(tls_in)
    {
     BIO_free(tls_in);
    }

	 if(tls_out)
    {
		BIO_free(tls_out);
    }
    #endif
    clean_in->reset();
    clean_out->reset();
    dirty_in->reset();
    dirty_out->reset();
  } ;


  void set_tls_data(TLS_data *tls) { this->tls = tls;};
  void set_info(EAPTLS_info_t *info) { this->info = info;};
  void set_bufferTLS_in(BufferTLS *tls_in) { this->tls_in = tls_in;};
  void set_bufferTLS_out(BufferTLS *tls_out){ this->tls_out = tls_out;};
  void set_clean_in(EAPTLS_record_t *record){ this->clean_in->reset(); this->clean_in->copy(record->rd_ptr(),record->length());};
  void set_clean_out(EAPTLS_record_t *record){ this->clean_out->reset(); this->clean_out->copy(record->rd_ptr(),record->length());};
  void append_dirty_in(EAPTLS_record_t *record)
  {
    if (record != NULL) this->dirty_in->copy(record->rd_ptr(),record->length());
  };
  void set_dirty_in(EAPTLS_record_t *record)
  {
    this->dirty_in->reset();
    append_dirty_in(record);
  };
  void set_dirty_out(EAPTLS_record_t *record){this->dirty_out->reset(); this->dirty_out->copy(record->rd_ptr(),record->length());};
  void set_first_fragment(bool first_fragment) {this->first_fragment = first_fragment;};
  void set_tls_msg_length(ACE_UINT32 tls_msg_length) {this->tls_msg_length = tls_msg_length;};
  void set_length_to_send(ACE_UINT32 length_to_send) {this->length_to_send = length_to_send;};
  void set_flags_to_send(ACE_Byte flags_to_send) {this->flags_to_send = flags_to_send;};
  void set_fragments(AAAMessageBlock *fragments) {this->fragments=fragments;};

  TLS_data *get_tls_data() { return tls;};
  //Information to derive new session keys
  AAAMessageBlock *get_master_key() {return this->master_key;};
  AAAMessageBlock *get_client_random() {return this->client_random;};             
  AAAMessageBlock *get_server_random(){return this->server_random;};
  //-------------------------------------------------                   
  EAPTLS_info_t *get_info() { return info;};
  BufferTLS *get_bufferTLS_in() {return tls_in;};
  BufferTLS *get_bufferTLS_out() { return tls_out;};
  EAPTLS_record_t  *get_clean_in() {return clean_in;};
  EAPTLS_record_t  *get_clean_out() {return this->clean_out;};
  EAPTLS_record_t  *get_dirty_in() {return this->dirty_in;};
  EAPTLS_record_t  *get_dirty_out() {return this->dirty_out;};
  ACE_UINT32 get_tls_msg_length() {return tls_msg_length;};
  ACE_UINT32 get_length_to_send() {return length_to_send;};
  AAAMessageBlock *restore_fragments(){return fragments;};
  ACE_Byte get_flags_to_send() {return flags_to_send;};
  bool if_first_fragment() {return first_fragment;};
  ACE_UINT32 get_fragment_size() {return eaptls->get_config()->get_fragment_size();}; //return FRAGMENT size
  bool if_length_included() {return eaptls->get_config()->get_include_length();};

  protected:
    EAPTLS_tls_t *eaptls;
    TLS_data *tls;
    EAPTLS_info_t *info;
    BufferTLS *tls_in;
    BufferTLS *tls_out;
    EAPTLS_record_t *clean_in;
    EAPTLS_record_t *clean_out;
    EAPTLS_record_t *dirty_in;
    EAPTLS_record_t *dirty_out;
    EAPTLSCrypto_callbacks *cb;
    //Used for fragmentation
    ACE_UINT32 tls_msg_length;
    ACE_UINT32 length_to_send;
    ACE_Byte flags_to_send;
    bool first_fragment;
    AAAMessageBlock *master_key;
    AAAMessageBlock *client_random;
    AAAMessageBlock *server_random;
    AAAMessageBlock *fragments;
    SSL_SESSION *sess;
   
};

class EAPTLS_session_t_peer : public EAPTLS_session_t
{
     public:
        EAPTLS_session_t_peer():EAPTLS_session_t(){this->sess=NULL;};
        EAPTLS_session_t_peer(EAPTLS_tls_t *eaptls):EAPTLS_session_t(eaptls){this->sess=NULL;};
        virtual EAPTLS_session_t_peer *session_init(bool resume)
        {
             ACE_INT32 verify_mode = 0;
             EAPTLS_session_t::session_init(resume);
             /* Always verify the peer certificate */
	          verify_mode |= SSL_VERIFY_PEER;
          	 verify_mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
	       	 SSL_set_verify(this->tls, verify_mode, cb->cbtls_verify);
          	/* In Client mode we only connect.  */
	          SSL_set_connect_state(this->tls);
             if (resume && (this->sess !=NULL)) SSL_set_session(this->tls,this->sess);
             return this;
        }
};

class EAPTLS_session_t_auth : public EAPTLS_session_t
{
    public:
        EAPTLS_session_t_auth():EAPTLS_session_t(){this->sess=NULL;};
        EAPTLS_session_t_auth(EAPTLS_tls_t *eaptls):EAPTLS_session_t(eaptls){this->sess=NULL;};

        virtual EAPTLS_session_t_auth *session_init(bool resume)
        {
             ACE_INT32 verify_mode = 0;
             EAPTLS_session_t::session_init(resume);
             /* Always verify the peer certificate */
	          verify_mode |= SSL_VERIFY_PEER;
           	 verify_mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
	          verify_mode |= SSL_VERIFY_CLIENT_ONCE;
	          SSL_set_verify(this->tls, verify_mode, cb->cbtls_verify);
	          /* In Server mode we only accept.  */
            SSL_set_accept_state(this->tls);
            return this;
        }
};

#endif


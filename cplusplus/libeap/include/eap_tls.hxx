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
                          eap_tls.hxx  -  description
                             -------------------
    begin                : lun mar 8 2007
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

#ifndef __EAP_TLS_HXX__
#define __EAP_TLS_HXX__


#include <ace/Basic_Types.h>
#include "eap.hxx"
#include "eap_log.hxx"


#ifdef WIN32
   #if defined(EAP_TLS_EXPORT)
       #define EAP_TLS_EXPORTS __declspec(dllexport)
   #else
       #define EAP_TLS_EXPORTS __declspec(dllimport)
   #endif
#else
   #define EAP_TLS_EXPORTS
   #define EAP_TLS_IMPORTS
#endif

#ifndef NO_OPENSSL
#include <openssl/err.h>
#if HAVE_OPENSSL_ENGINE_H
#include <openssl/engine.h>
#endif
#include <openssl/ssl.h>
#include <openssl/rand.h>
#endif /* !defined(NO_OPENSSL) */

#define BUFFER_SIZE 1024
#define MAX_RECORD_SIZE 16384

#define EAP_TLS_START          	1
#define EAP_TLS_ACK          	2
#define EAP_TLS_SUCCESS         3
#define EAP_TLS_FAIL          	4
#define EAP_TLS_ALERT          	9

#define TLS_HEADER_LEN          4

#define TLS_START(x) 		(((x) & 0x20) >> 5)
#define TLS_MORE_FRAGMENTS(x) 	(((x) & 0x40) >> 6)
#define TLS_LENGTH_INCLUDED(x) 	(((x) & 0x80) >> 7)

#define TLS_CHANGE_CIPHER_SPEC(x) 	(((x) & 0x0014) == 0x0014)
#define TLS_ALERT(x) 			(((x) & 0x0015) == 0x0015)
#define TLS_HANDSHAKE(x) 		(((x) & 0x0016) == 0x0016)

#define SET_START(x) 		((x) | (0x20))
#define SET_MORE_FRAGMENTS(x) 	((x) | (0x40))
#define SET_LENGTH_INCLUDED(x) 	((x) | (0x80))

#define TLS_MAX_MASTER_KEY_LENGTH		48

/// \def EAP Request/Response Type code assigned for
/// EAP-TLS.  This should be replaced with an IANA allocated value.
#define TLS_METHOD_TYPE  13


typedef enum {
        EAPTLS_INVALID = 0,	  	/* invalid, don't reply */
        EAPTLS_REQUEST,       		/* request, ok to send, invalid to receive */
        EAPTLS_RESPONSE,       		/* response, ok to receive, invalid to send */
        EAPTLS_SUCCESS,       		/* success, send success */
        EAPTLS_FAIL,       		/* fail, send fail */
        EAPTLS_NOOP,       		/* noop, continue */

        EAPTLS_START,       		/* start, ok to send, invalid to receive */
        EAPTLS_OK, 	         	/* ok, continue */
        EAPTLS_ACK,       		/* acknowledge, continue */
        EAPTLS_FIRST_FRAGMENT,    	/* first fragment */
        EAPTLS_MORE_FRAGMENTS,    	/* more fragments, to send/receive */
        EAPTLS_LENGTH_INCLUDED,          	/* length included */
        EAPTLS_MORE_FRAGMENTS_WITH_LENGTH    /* more fragments with length */
} eaptls_status_t;

/* Following enums from rfc2246 */
 typedef      enum {
           change_cipher_spec = 20,
           alert = 21,
           handshake = 22,
           application_data = 23
       } ContentType;

typedef       enum { warning = 1, fatal = 2 } AlertLevel;

typedef       enum {
           close_notify = 0,
           unexpected_message = 10,
           bad_record_mac = 20,
           decryption_failed = 21,
           record_overflow = 22,
           decompression_failure = 30,
           handshake_failure = 40,
           bad_certificate = 42,
           unsupported_certificate = 43,
           certificate_revoked = 44,
           certificate_expired = 45,
           certificate_unknown = 46,
           illegal_parameter = 47,
           unknown_ca = 48,
           access_denied = 49,
           decode_error = 50,
           decrypt_error = 51,
           export_restriction = 60,
           protocol_version = 70,
           insufficient_security = 71,
           internal_error = 80,
           user_canceled = 90,
           no_renegotiation = 100
       } AlertDescription;

typedef       enum {
           hello_request = 0,
           client_hello = 1,
           server_hello = 2,
           certificate = 11,
           server_key_exchange  = 12,
           certificate_request = 13,
           server_hello_done = 14,
           certificate_verify = 15,
           client_key_exchange = 16,
           finished = 20
       } HandshakeType;


typedef SSL TLS_data;
typedef SSL_CTX TLS_context;
typedef SSL_METHOD TLS_method;
typedef BIO BufferTLS;
typedef DH DH_params;
typedef AAAMessageBlock EAPTLS_record_t ;

class EAPTLS_info_t
{

  public:

  EAPTLS_info_t():origin(0),content_type(0),
                           handshake_type(0),alert_level(0),alert_description(0),
                           info_description(""),record_len(0),version(0){};
  EAPTLS_info_t(ACE_Byte origin,
                           ACE_Byte content_type,
                           ACE_Byte handshake_type,
                           ACE_Byte alert_level,
                           ACE_Byte alert_description,
                           std::string info_description,
                           ACE_UINT32 record_len,
                           ACE_UINT32 version):origin(origin),content_type(content_type),handshake_type(handshake_type),alert_level(alert_level),alert_description(alert_description),
                                                              info_description(info_description),record_len(record_len),version(version){};

  void set_origin(ACE_Byte origin) { this->origin = origin;}
  void set_content_type(ACE_Byte content_type){ this->content_type = content_type;}
  void set_handshake_type(ACE_Byte handshake_type) {this->handshake_type = handshake_type;};
  void set_alert_level(ACE_Byte alert_level){ this->alert_level = alert_level;};
  void set_alert_description(ACE_Byte alert_description){this-> alert_description = alert_description;};
  void set_info_description(std::string &info_description){this->info_description = info_description;};
  void set_record_len(ACE_INT32 record_len){this->record_len = record_len;};
  void set_version(ACE_INT32 version){this->version = version;};


  ACE_Byte get_origin() { return origin;};
  ACE_Byte get_content_type() { return content_type;};
  ACE_Byte get_handshake_type() {return handshake_type;};
  ACE_Byte get_alert_level(){ return alert_level;};
  ACE_Byte get_alert_description(){return alert_description;};
  std::string &get_info_description(){return info_description;};
  ACE_INT32 get_record_len(){return record_len;};
  ACE_INT32 get_version(){return version;};

  
  protected:
  ACE_Byte origin;
  ACE_Byte content_type;
  ACE_Byte handshake_type;
  ACE_Byte alert_level;
  ACE_Byte alert_description;
  std::string info_description;
  ACE_INT32 record_len;
  ACE_INT32 version;
};

class EAPTLS_config
{
    public:
    EAPTLS_config(std::string &private_key_password,
                              std::string &private_key_file,
                              std::string &certificate_file,
                              std::string &random_file,
                              std::string &ca_path,
                              std::string &ca_file,                            
                              std::string &dh_file,
                              ACE_INT32 rsa_key,
                              ACE_INT32 dh_key,
                              ACE_INT32 rsa_key_length,
                              ACE_INT32 dh_key_length,
                              ACE_INT32 verify_depth,
                              ACE_INT32 file_type,
                              bool include_length,
                              ACE_INT32 fragment_size)
    {
          this->private_key_password = private_key_password;
          this->private_key_file = private_key_file;
          this->certificate_file = certificate_file;
          this->random_file = random_file;
          this->ca_path = ca_path;
          this->ca_file = ca_file;
          this->dh_file = dh_file;
          this->rsa_key = rsa_key;
          this->dh_key = dh_key;
          this->rsa_key_length = rsa_key_length;
          this->dh_key_length = dh_key_length;
          this->verify_depth = verify_depth;
          this->file_type = file_type;
          this->include_length = include_length;
          this->fragment_size = fragment_size;
    }

    void read_config(std::string &config_file){};    //TODO: Read from a XML file all these params.
    std::string &get_private_key_password() {return private_key_password;};
    std::string &get_private_key_file() { return private_key_file;};
    std::string &get_certificate_file() {return certificate_file;};
    std::string &get_random_file(){return random_file;};
    std::string &get_ca_path(){return ca_path;};
    std::string &get_ca_file() {return ca_file;};
    std::string &get_dh_file() {return dh_file;};
    ACE_INT32 get_rsa_key() {return rsa_key;};
    ACE_INT32 get_dh_key() {return dh_key;};
    ACE_INT32 get_rsa_key_length() {return rsa_key_length;};
    ACE_INT32 get_dh_key_length() {return dh_key_length;};
    ACE_INT32 get_verify_depth() {return verify_depth;};
    ACE_INT32 get_file_type() {return file_type;};
    bool get_include_length() {return include_length;};
    ACE_INT32 get_fragment_size() {return fragment_size;};

    protected:
          std::string private_key_password;
          std::string private_key_file;
          std::string certificate_file;
          std::string random_file;
          std::string ca_path;
          std::string ca_file;
          std::string dh_file;
          ACE_INT32 rsa_key;
          ACE_INT32 dh_key;
          ACE_INT32 rsa_key_length;
          ACE_INT32 dh_key_length;
          ACE_INT32 verify_depth;
          ACE_INT32 file_type;
          bool include_length;
          ACE_INT32 fragment_size;
};


/* This class gets stored in arg */
class EAPTLS_tls_t
{
  public:
  EAPTLS_tls_t() {this->conf = NULL; this->ctx = NULL;};
  EAPTLS_tls_t(EAPTLS_config *conf,TLS_context *ctx){this->conf = conf; this->ctx = ctx;};
  virtual ~EAPTLS_tls_t() {if (conf !=NULL) delete conf; if (ctx != NULL) delete ctx;};
  EAPTLS_config *get_config() {return conf;};
  TLS_context *get_tls_context() {return ctx;};
  protected:
  EAPTLS_config *conf;
  TLS_context *ctx;
};

/*
 * From rfc
   Flags

      0 1 2 3 4 5 6 7 8
      +-+-+-+-+-+-+-+-+
      |L M S R R R R R|
      +-+-+-+-+-+-+-+-+

      L = Length included
      M = More fragments
      S = EAP-TLS start
      R = Reserved

      The L bit (length included) is set to indicate the presence of the
      four octet TLS Message Length field, and MUST be set for the first
      fragment of a fragmented TLS message or set of messages. The M bit
      (more fragments) is set on all but the last fragment. The S bit
      (EAP-TLS start) is set in an EAP-TLS Start message.  This
      differentiates the EAP-TLS Start message from a fragment
      acknowledgement.

   TLS Message Length

      The TLS Message Length field is four octets, and is present only
      if the L bit is set. This field provides the total length of the
      TLS message or set of messages that is being fragmented.

   TLS data

      The TLS data consists of the encapsulated TLS packet in TLS record
      format.
 *
 * The data structures present here
 * maps only to the typedata in the EAP packet
 *
 * Based on the L bit flag, first 4 bytes of data indicate the length
 */


/// EAP-TLS/Request message.
class EAP_TLS_EXPORTS EapRequestTls: public EapRequest
{
public:
  EapRequestTls(ACE_Byte flags) : EapRequest(EapType(TLS_METHOD_TYPE)), flags(flags) {this->data=NULL;is_ack=false;};

  /// Use this function to obtain a reference to flags.
  ACE_Byte get_flags() { return flags; };
  /// Use this function to obtain a reference to TLS message length
  ACE_UINT32 get_tls_message_length() { return tls_message_length;};
  /// Use this function to obtain a reference to TLS message data
  AAAMessageBlock *get_data() {return this->data;};
  bool get_is_ack(){return is_ack;};
  /// Use this function to obtain a reference to flags.
  void set_flags(ACE_Byte flags) { this->flags=flags;};
  void set_is_ack(bool is_ack) {this->is_ack = is_ack;};
  /// Use this function to obtain a reference to TLS message length
  void set_tls_message_length(ACE_UINT32 tls_message_length) { this->tls_message_length = tls_message_length;};
  /// Use this function to obtain a reference to TLS message data
  void  set_data(AAAMessageBlock *data)
  {
    if (this->data) this->data->release();
    this->data = data;
  };

protected:
  bool is_ack;
  /// TLS Flags
  ACE_Byte flags;
  /// TLS message length
  ACE_UINT32 tls_message_length;
  /// TLS data
  AAAMessageBlock *data;
};

/// EAP-TLS/Response message.
class EAP_TLS_EXPORTS EapResponseTls: public EapRequestTls
{                                                                                                                                                                                                        
public:
  EapResponseTls(ACE_Byte flags) : EapRequestTls(flags) {}
};

#endif // __EAP_TLS_HXX__






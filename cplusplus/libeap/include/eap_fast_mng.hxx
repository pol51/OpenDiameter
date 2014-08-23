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
                          eap_fast_mng.hxx  -  description
                             -------------------
    begin                : jue mar 11 2007
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


#ifndef  __EAP_FAST_MNG_H__
#define  __EAP_FAST_MNG_H__

#include "eap_fast.hxx"
#include "eap_fast_session.hxx"
//#include "eap_fast_pac.h"

class EAPFAST_fast_mng
{
  public:
  EAPFAST_fast_mng() {};
  virtual ~EAPFAST_fast_mng(){};
  /*
  * TODO: Check for the type of key exchange
  *  like conf->dh_key
 */
  ACE_INT32 load_dh_params(TLS_context *ctx, std::string &file);
  ACE_INT32 generate_eph_rsa_key(TLS_context *ctx);
  /*
  * Create Global context SSL and use it in every new session
  * # Load the trusted CAs
  * # Load the Private key & the certificate
  * # Set the Context options & Verify options
  */
  ACE_INT32 tls_handshake_recv(EAPFAST_session_t *ssn);
  ACE_INT32 tls_connection_encrypt(EAPFAST_session_t *ssn);
  ACE_INT32 tls_connection_decrypt(EAPFAST_session_t *ssn);
 // ACE_INT32 tls_handshake_recv(EAPFAST_session_t *ssn);
  static void fast_session_information(EAPFAST_session_t *fast_session);
  void fast_check_state(SSL_data *s, ACE_INT32 ret);

	enum EapFASTMngState
	{
		StOk,
		StAlertReceive,
		StAlertSend
	};
};

class EAPFAST_fast_mng_peer : public EAPFAST_fast_mng
{
  public:
    EAPFAST_fast_mng_peer():EAPFAST_fast_mng(){};
    FAST_context_peer *init_fast_ctx(EAPFAST_config &conf);  
};

class EAPFAST_fast_mng_auth : public EAPFAST_fast_mng
{
  public:
    EAPFAST_fast_mng_auth():EAPFAST_fast_mng(){};
    FAST_context_auth *init_fast_ctx(EAPFAST_config &conf,std::string session_id_context);
};


#endif

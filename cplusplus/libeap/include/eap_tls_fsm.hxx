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
                          eap_tls_fsm.hxx  -  description
                             -------------------
    begin                : jue mar 18 2004
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
#ifndef  __EAP_TLS_FSM_HXX__
#define  __EAP_TLS_FSM_HXX__


#include <ace/Reactor.h>
#include <stdio.h>
#include "eap_tls.hxx"
#include "eap_fsm.hxx"
#include "eap_peerfsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_method_registrar.hxx"
#include "eap_tls_session.hxx"
#include "eap_tls_parser.hxx"
#include "eap_tls_mng.hxx"
#include "eap_tls_xml_data.hxx"

/// Peer state machine for EAP EAP-Tls authentication method.
class EapPeerTlsStateMachine
  :  public EapMethodStateMachine,
     public EapStateMachine<EapPeerTlsStateMachine>
{
  friend class EapMethodStateMachineCreator<EapPeerTlsStateMachine>;
  friend class EapPeerTlsStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
	std::string fname = InputConfigFile ();
	std::string cfgRoot = EAPTLS_CFG_XML_ROOT_CLIENT;

	EAPTLS_XMLTreeParser parser;
	EAPTLS_XMLDataRootClient rootData (cfgRoot, *EAPTLS_CFG_ROOT_CLIENT ());

	if (parser.open(fname, rootData) != 0)
	{
		// TODO: Error message
		return;
	}

	EAPTLS_config *config_peer = new EAPTLS_config(EAPTLS_CFG_ENCRYPTION_CLIENT()->pass_phrase,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->key_file,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->cert_file,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->random,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->ca_path,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->ca_cert,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->dh,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->rsa_key,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->dh_key,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->rsa_key_length,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->dh_key_length,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->verify_depth,
												EAPTLS_CFG_ENCRYPTION_CLIENT()->file_type,
												EAPTLS_CFG_FRAGMENTATION_CLIENT()->include_length,
												EAPTLS_CFG_FRAGMENTATION_CLIENT()->fragment_size);

   

    ctx_peer=tls_mng_peer.init_tls_ctx(*config_peer);
    tls_conf_ctx_peer = new EAPTLS_tls_t(config_peer,ctx_peer);
    EapStateMachine<EapPeerTlsStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapPeerTlsStateMachine>::Notify(ev);
  }

  /// This pure virtual function is a callback used when a Peer's identity
  /// needs to be obtained.
  virtual std::string& InputIdentity()=0;

  /// This function is used for obtaining a reference to TLS session.
  EAPTLS_session_t_peer* get_tls_session() {return ssn;};
  EAPTLS_tls_mng_peer& get_mng_peer(){return tls_mng_peer;};
  /// This function is used for setting a TLS session.
  void set_tls_session(EAPTLS_session_t_peer *ssn)
  {
        if (this->ssn != NULL) delete this->ssn;this->ssn = ssn;
  }
  /// This function returns TLS MK.
  /*AAAMessageBlock *get_master_key()
  {
    TLS_data *data = ssn->get_tls_data();
    ACE_UINT32 msk_length = (ACE_UINT32)(data->session->master_key_length); //Recover master key length
    AAAMessageBlock *msk = AAAMessageBlock::Acquire(msk_length);
    msk->copy((const char *)data->session->master_key,msk_length);
    return msk;
  }*/

  EAPTLS_tls_t * get_ctx_peer() {return tls_conf_ctx_peer;};

// Modified by Santiago Zapata Hernandez for UMU
// New
    std::string & MSK ()
    {
	masterSessionKey = std::string ("");
	if (IsDone ())
	{
	    AAAMessageBlock * b = EAPTLSCrypto_callbacks::eaptls_gen_mppe_keys (ssn->get_master_key (),
										ssn->get_client_random (),
										ssn->get_server_random ());
	    /* Get masterkey from MessageBlock */
	    masterSessionKey = std::string (b->base (), b->length ());
	}
	return masterSessionKey;
    }
// End New

protected:
  EAPTLS_config *config_peer;
  EAPTLS_tls_mng_peer tls_mng_peer;
  TLS_context *ctx_peer;
  EAPTLS_tls_t  *tls_conf_ctx_peer;
  
  EapPeerTlsStateMachine(EapSwitchStateMachine &s);
  ~EapPeerTlsStateMachine() {};
  
	virtual std::string& InputConfigFile () = 0;
  
  /// TLS session
  EAPTLS_session_t_peer *ssn;
  /// Retains received messages with concatinating them.
  std::string history;

// Modified by Santiago Zapata Hernandez for UMU
// New
    std::string masterSessionKey;
// End New
};

/// Authenticator state machine for EAP-Tls authentication method.
class EapAuthTlsStateMachine
  :  public EapMethodStateMachine,
     public EapStateMachine<EapAuthTlsStateMachine>
{
  friend class EapMethodStateMachineCreator<EapAuthTlsStateMachine>;
  friend class EapAuthTlsStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
	std::string fname = InputConfigFile ();

	std::string cfgRoot = EAPTLS_CFG_XML_ROOT_SERVER;

	EAPTLS_XMLTreeParser parser;
	EAPTLS_XMLDataRootServer rootData (cfgRoot, *EAPTLS_CFG_ROOT_SERVER ());

	if (parser.open(fname, rootData) != 0)
	{
		// TODO: Error message
		return;
	}

	EAPTLS_config *config_auth = new EAPTLS_config(EAPTLS_CFG_ENCRYPTION_SERVER()->pass_phrase,
												EAPTLS_CFG_ENCRYPTION_SERVER()->key_file,
												EAPTLS_CFG_ENCRYPTION_SERVER()->cert_file,
												EAPTLS_CFG_ENCRYPTION_SERVER()->random,
												EAPTLS_CFG_ENCRYPTION_SERVER()->ca_path,
												EAPTLS_CFG_ENCRYPTION_SERVER()->ca_cert,
												EAPTLS_CFG_ENCRYPTION_SERVER()->dh,
												EAPTLS_CFG_ENCRYPTION_SERVER()->rsa_key,
												EAPTLS_CFG_ENCRYPTION_SERVER()->dh_key,
												EAPTLS_CFG_ENCRYPTION_SERVER()->rsa_key_length,
												EAPTLS_CFG_ENCRYPTION_SERVER()->dh_key_length,
												EAPTLS_CFG_ENCRYPTION_SERVER()->verify_depth,
												EAPTLS_CFG_ENCRYPTION_SERVER()->file_type,
												EAPTLS_CFG_FRAGMENTATION_SERVER()->include_length,
												EAPTLS_CFG_FRAGMENTATION_SERVER()->fragment_size);
	ctx_auth=tls_mng_auth.init_tls_ctx (*config_auth, EAPTLS_CFG_SERVER_SERVER()->id_context);
    tls_conf_ctx_auth = new EAPTLS_tls_t(config_auth,ctx_auth);
    

    EapStateMachine<EapAuthTlsStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapAuthTlsStateMachine>::Notify(ev);
  }
  /// This function is used for obtaining a reference to TLS session.
  EAPTLS_session_t_auth* get_tls_session() {return ssn;};
  EAPTLS_tls_mng_auth& get_mng_auth(){return tls_mng_auth;};
  /// This function is used for setting a TLS session.
  void set_tls_session(EAPTLS_session_t_auth *ssn) {if (this->ssn != NULL) delete this->ssn;this->ssn = ssn;};
  /// This function returns TLS MK.
  /*AAAMessageBlock *get_master_key()
  {
    TLS_data *data = ssn->get_tls_data();
    ACE_UINT32 msk_length = (ACE_UINT32)(data->session->master_key_length); //Recover master key length
    AAAMessageBlock *msk = AAAMessageBlock::Acquire((char *)data->session->master_key,msk_length);
    return msk;
  }*/

  EAPTLS_tls_t * get_ctx_auth(){ return tls_conf_ctx_auth;}
  
  /// This function is used for obtaining a reference to history;
  std::string& History() { return history; }

// Modified by Santiago Zapata Hernandez for UMU
// New
    std::string & MSK ()
    {
	masterSessionKey = std::string ("");
	if (IsDone ())
	{
	    AAAMessageBlock * b = EAPTLSCrypto_callbacks::eaptls_gen_mppe_keys (ssn->get_master_key (),
										ssn->get_client_random (),
										ssn->get_server_random ());
	    /* Get masterkey from MessageBlock */
	    masterSessionKey = std::string (b->base (), b->length ());
	}
	return masterSessionKey;
    }
// End New

protected:

  EAPTLS_config *config_auth;
  EAPTLS_tls_mng_auth tls_mng_auth;
  TLS_context *ctx_auth;
  EAPTLS_tls_t  *tls_conf_ctx_auth;

  EapAuthTlsStateMachine(EapSwitchStateMachine &s);
  ~EapAuthTlsStateMachine() {}
  
  	virtual std::string& InputConfigFile () = 0;

  /// TLS session
  EAPTLS_session_t_auth *ssn;
  /// Retains received messages with concatinating them.
  std::string history;

// Modified by Santiago Zapata Hernandez for UMU
// New
    std::string masterSessionKey;
// End New
};
#endif

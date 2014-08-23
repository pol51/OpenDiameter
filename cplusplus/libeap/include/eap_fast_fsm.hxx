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
                          eap_fast_fsm.hxx  -  description
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
#ifndef  __EAP_FAST_FSM_HXX__
#define  __EAP_FAST_FSM_HXX__


#include <ace/Reactor.h>
#include <stdio.h>
#include "eap_fast.hxx"
#include "eap_fsm.hxx"
#include "eap_peerfsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_method_registrar.hxx"
#include "eap_fast_session.hxx"
#include "eap_fast_parser.hxx"
#include "eap_fast_mng.hxx"
#include "eap_fast_xml_data.hxx"

/// Peer state machine for EAP EAP-Fast authentication method.
class EapPeerFastStateMachine
  :  public EapMethodStateMachine,
     public EapStateMachine<EapPeerFastStateMachine>
{
  friend class EapMethodStateMachineCreator<EapPeerFastStateMachine>;
  friend class EapPeerFastStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
	std::string fname = InputConfigFile ();
	std::string cfgRoot = EAPFAST_CFG_XML_ROOT_CLIENT;

	EAPFAST_XMLTreeParser parser;
	EAPFAST_XMLDataRootClient rootData (cfgRoot, *EAPFAST_CFG_ROOT_CLIENT ());

	if (parser.open(fname, rootData) != 0)
	{
		// TODO: Error message
		return;
	}

	EAPFAST_config *config_peer = new EAPFAST_config(EAPFAST_CFG_ENCRYPTION_CLIENT()->pass_phrase,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->key_file,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->cert_file,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->random,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->ca_path,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->ca_cert,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->pac_file,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->credential_file,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->pac_opaque_encr,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->dh,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->rsa_key,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->dh_key,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->rsa_key_length,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->dh_key_length,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->verify_depth,
							EAPFAST_CFG_ENCRYPTION_CLIENT()->file_type,
							EAPFAST_CFG_FRAGMENTATION_CLIENT()->include_length,
							EAPFAST_CFG_FRAGMENTATION_CLIENT()->fragment_size);

   
    //struct eap_fast_pac *pac_root;
    ctx_peer=fast_mng_peer.init_fast_ctx(*config_peer);
    fast_conf_ctx_peer = new EAPFAST_fast_t(config_peer,NULL,ctx_peer);
    EapStateMachine<EapPeerFastStateMachine>::Start();
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev)
  {
    EapStateMachine<EapPeerFastStateMachine>::Notify(ev);
  }

  /// This pure virtual function is a callback used when a Peer's identity
  /// needs to be obtained.
  virtual std::string& InputIdentity()=0;

  /// This function is used for obtaining a reference to FAST session.
  EAPFAST_session_t_peer* get_fast_session() {return ssn;};
  EAPFAST_fast_mng_peer& get_mng_peer(){return fast_mng_peer;};
  /// This function is used for setting a FAST session.
  void set_fast_session(EAPFAST_session_t_peer *ssn)
  {
        if (this->ssn != NULL) delete this->ssn;this->ssn = ssn;
  }
  /// This function returns FAST MK.
  /*AAAMessageBlock *get_master_key()
  {
    FAST_data *data = ssn->get_fast_data();
    ACE_UINT32 msk_length = (ACE_UINT32)(data->session->master_key_length); //Recover master key length
    AAAMessageBlock *msk = AAAMessageBlock::Acquire(msk_length);
    msk->copy((const char *)data->session->master_key,msk_length);
    return msk;
  }*/

  EAPFAST_fast_t * get_ctx_peer() {return fast_conf_ctx_peer;};

// Modified by Santiago Zapata Hernandez for UMU
// New
    std::string & MSK ()
    {
	masterSessionKey = std::string ("");
	if (IsDone ())
	{
	    AAAMessageBlock * b = EAPFASTCrypto_callbacks::eapfast_gen_mppe_keys (ssn->get_master_key (),
										ssn->get_client_random (),
										ssn->get_server_random ());
	    /* Get masterkey from MessageBlock */
	    masterSessionKey = std::string (b->base (), b->length ());
	}
	return masterSessionKey;
    }
// End New

protected:
  EAPFAST_config *config_peer;
  EAPFAST_fast_mng_peer fast_mng_peer;
  FAST_context_peer *ctx_peer;
  EAPFAST_fast_t  *fast_conf_ctx_peer;
  
  EapPeerFastStateMachine(EapSwitchStateMachine &s);
  ~EapPeerFastStateMachine() {};
  
	virtual std::string& InputConfigFile () = 0;
  
  /// FAST session
  EAPFAST_session_t_peer *ssn;
  /// Retains received messages with concatinating them.
  std::string history;

// Modified by Santiago Zapata Hernandez for UMU
// New
    std::string masterSessionKey;
// End New
};

/// Authenticator state machine for EAP-Fast authentication method.
class EapAuthFastStateMachine
  :  public EapMethodStateMachine,
     public EapStateMachine<EapAuthFastStateMachine>
{
  friend class EapMethodStateMachineCreator<EapAuthFastStateMachine>;
  friend class EapAuthFastStateTable_S;
public:

  /// Reimplemented from EapMethodStateMachine
  void Start() throw(AAA_Error)
  {
	//std::cout<<"[EapAuthFastStateMachine]start\n";
	std::string fname = InputConfigFile ();
	//std::cout<<"config file name:"<<fname<<"\n";

	std::string cfgRoot = EAPFAST_CFG_XML_ROOT_SERVER;

	EAPFAST_XMLTreeParser parser;
	EAPFAST_XMLDataRootServer rootData (cfgRoot, *EAPFAST_CFG_ROOT_SERVER ());
	//std::cout<<"flag 1\n";

	if (parser.open(fname, rootData) != 0)
	{
		std::cout<<"not open config file\n";// TODO: Error message
		return;
	}

	EAPFAST_config *config_auth = new EAPFAST_config(EAPFAST_CFG_ENCRYPTION_SERVER()->pass_phrase,
							EAPFAST_CFG_ENCRYPTION_SERVER()->key_file,
							EAPFAST_CFG_ENCRYPTION_SERVER()->cert_file,
							EAPFAST_CFG_ENCRYPTION_SERVER()->random,
							EAPFAST_CFG_ENCRYPTION_SERVER()->ca_path,
							EAPFAST_CFG_ENCRYPTION_SERVER()->ca_cert,
							EAPFAST_CFG_ENCRYPTION_SERVER()->pac_file,
							EAPFAST_CFG_ENCRYPTION_SERVER()->credential_file,
							EAPFAST_CFG_ENCRYPTION_SERVER()->pac_opaque_encr,
							EAPFAST_CFG_ENCRYPTION_SERVER()->dh,
							EAPFAST_CFG_ENCRYPTION_SERVER()->rsa_key,
							EAPFAST_CFG_ENCRYPTION_SERVER()->dh_key,
							EAPFAST_CFG_ENCRYPTION_SERVER()->rsa_key_length,
							EAPFAST_CFG_ENCRYPTION_SERVER()->dh_key_length,
							EAPFAST_CFG_ENCRYPTION_SERVER()->verify_depth,
							EAPFAST_CFG_ENCRYPTION_SERVER()->file_type,
							EAPFAST_CFG_FRAGMENTATION_SERVER()->include_length,
							EAPFAST_CFG_FRAGMENTATION_SERVER()->fragment_size);

	config_auth->set_auth_id(EAPFAST_CFG_SERVER_SERVER()->auth_id);
	ctx_auth=fast_mng_auth.init_fast_ctx (*config_auth, EAPFAST_CFG_SERVER_SERVER()->auth_id);
	//ctx_auth_fast->pac_root=NULL;
    fast_conf_ctx_auth = new EAPFAST_fast_t(config_auth,ctx_auth,NULL);
	//std::cout<<"flag 2\n";
    

    EapStateMachine<EapAuthFastStateMachine>::Start(); 
  }

  /// Reimplemented from EapMethodStateMachine
  inline void Notify(AAA_Event ev) 
  {
    EapStateMachine<EapAuthFastStateMachine>::Notify(ev);
  }
  /// This function is used for obtaining a reference to FAST session.
  EAPFAST_session_t_auth* get_fast_session() {return ssn;};
  EAPFAST_fast_mng_auth& get_mng_auth(){return fast_mng_auth;};
  /// This function is used for setting a FAST session. 
  void set_fast_session(EAPFAST_session_t_auth *ssn) {if (this->ssn != NULL) delete this->ssn;this->ssn = ssn;};
  /// This function returns FAST MK.
  /*AAAMessageBlock *get_master_key()
  {
    FAST_data *data = ssn->get_fast_data();
    ACE_UINT32 msk_length = (ACE_UINT32)(data->session->master_key_length); //Recover master key length
    AAAMessageBlock *msk = AAAMessageBlock::Acquire((char *)data->session->master_key,msk_length);
    return msk;
  }*/

  EAPFAST_fast_t * get_ctx_auth(){ return fast_conf_ctx_auth;}
  
  /// This function is used for obtaining a reference to history;
  std::string& History() { return history; }

// Modified by Santiago Zapata Hernandez for UMU
// New
    std::string & MSK ()
    {
	masterSessionKey = std::string ("");
	if (IsDone ())
	{
	    AAAMessageBlock * b = EAPFASTCrypto_callbacks::eapfast_gen_mppe_keys (ssn->get_master_key (),
										ssn->get_client_random (),
										ssn->get_server_random ());
	    /* Get masterkey from MessageBlock */
	    masterSessionKey = std::string (b->base (), b->length ());
	}
	return masterSessionKey;
    }
// End New

protected:

  EAPFAST_config *config_auth;
  EAPFAST_fast_mng_auth fast_mng_auth;
  FAST_context_auth *ctx_auth;
  EAPFAST_fast_t  *fast_conf_ctx_auth;

  EapAuthFastStateMachine(EapSwitchStateMachine &s);
  ~EapAuthFastStateMachine() {}
  
  	virtual std::string& InputConfigFile () = 0;

  /// FAST session
  EAPFAST_session_t_auth *ssn;
  /// Retains received messages with concatinating them.
  std::string history;

// Modified by Santiago Zapata Hernandez for UMU
// New
    std::string masterSessionKey;
// End New
};
#endif

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
                          eap_fast_fsm.cxx  -  description
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

#include "eap_fast_fsm.hxx"

#define VALUE_TIME 1
/// Action class.
class EapPeerFastAction : public AAA_Action<EapPeerFastStateMachine>
{
 public:
  virtual void operator()(EapPeerFastStateMachine&) {}
 protected:
  EapPeerFastAction() {}
  virtual ~EapPeerFastAction() {}
};

/// Action class.
class EapAuthFastAction : public AAA_Action<EapAuthFastStateMachine>
{
 public:
  virtual void operator()(EapAuthFastStateMachine&) {}
 protected:
  EapAuthFastAction() {}
  virtual ~EapAuthFastAction() {}
};

/// State table used by EapAuthFastStateMachine.
class EAP_FAST_EXPORTS EapAuthFastStateTable_S: public AAA_StateTable<EapAuthFastStateMachine>
{
  friend class ACE_Singleton<EapAuthFastStateTable_S, ACE_Recursive_Thread_Mutex>;

private:
    
  class AcBuildStart : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "<----------- AuthFast: Building a Start message.\n");
      
      EAPFAST_session_t_auth *session_auth = new EAPFAST_session_t_auth(msm.get_ctx_auth());
      std::string auth_id = session_auth->get_auth_id();
      int auth_id_len = auth_id.length();
      char * buf = new char[auth_id_len+FAST_DATA_HEADER_LEN];
      memset(buf, 0 , auth_id_len+FAST_DATA_HEADER_LEN);
      buf[1] = FAST_DATA_TYPE_AUTH_ID;
      buf[3] = auth_id_len;
      memcpy(buf+FAST_DATA_HEADER_LEN, auth_id.c_str(), auth_id_len);

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(5+1+auth_id_len+FAST_DATA_HEADER_LEN); //[Code(1) + identifier (1) + length(2) + type(1)] + flags(1)

      ACE_OS::memset(msg->base(), 0, 5+1+auth_id_len+FAST_DATA_HEADER_LEN);
      
      EapRequestFast request(SET_START(0x00)|SET_VERSION(0x01)); //A request with S bit enabled.
      
      AAAMessageBlock *request_data = AAAMessageBlock::Acquire (buf,auth_id_len+FAST_DATA_HEADER_LEN);
      request.set_data(request_data);
      EapRequestFastParser  parser;
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    } 
      session_auth->session_init(false);
      msm.set_fast_session(session_auth); //Build a new session.
      //msm.History().append(msg->base() + 4, 1);
      // Set the message to the session.
      ssm.SetTxMessage(msg);
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcProcessResponseClientHello : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAPFAST_fast_mng_auth &fast_mng_auth = msm.get_mng_auth();
      AAAMessageBlock *msg = ssm.GetRxMessage();   
	/*Msg contains complete message.rd_ptr() points data section. 
	(from byte after Type) and wr_ptr() at the end of Message*/
      msg->wr_ptr(msg->end());
      EAP_LOG(LM_DEBUG, "------------->AuthFast: Processing EAP-FAST client hello (MSG lEN = %d).\n",msg->length());
      // Check the EAP-FAST response.
      EapResponseFast response(SET_VERSION(0x01)); 
      EapResponseFastParser parser;
      parser.setAppData(&response); 
      parser.setRawData(msg);
      try { parser.parseRawToApp(); } 
      catch	(...) { 
        EAP_LOG(LM_ERROR, "AuthFast: Parse error.\n");
        msm.Event(EvSgInvalid);
        return; 
      }
      
      EAPFAST_session_t_auth *session_auth = msm.get_fast_session();
      session_auth->set_dirty_in(response.get_data());

      session_auth->get_fast_data()->recv_ver = (response.get_flags()&0x7);

      ACE_INT32 err; 

      err = fast_mng_auth.tls_handshake_recv(session_auth);

      if((err) == EAPFAST_fast_mng::StAlertReceive)
      {

	EAP_LOG(LM_ERROR, "AuthFast: AcProcessResponseClienteHello: AlertReceive.\n");
     	msm.Event(EvSgAlertReceive);
      }
      else if (err == EAPFAST_fast_mng::StAlertSend)
      {
	EAP_LOG(LM_ERROR, "AuthFast: AcProcessResponseClienteHello: AlertSend.\n");
      	msm.Event(EvSgAlertSend);
      }
      else
      {
	msm.Event(EvSgValid);
      }
    }
  };

  class AcVerifyAuthMoreFragments : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      ACE_Byte flags=0x00;
      ACE_UINT32 state=0;

      EAP_LOG(LM_DEBUG, "AuthFast: Verify Auth more fragments.\n");
    
      EAPFAST_session_t_auth *session_auth = msm.get_fast_session();

      //Getting FAST records to be encapsulated in request
      AAAMessageBlock *data = session_auth->get_dirty_out(); 

      //+6 (Header length) =//Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+Data(n)
      ACE_INT32 length_to_send=data->length()+6;
      ACE_INT32 length_fragment=session_auth->get_fragment_size();
      
      static bool first_fragment=true;
      
      if ((length_fragment != 0) && (length_fragment < length_to_send))
      {
          EAP_LOG(LM_DEBUG,"AUTH:FRAGMENTS\n");
          length_to_send = length_fragment; //We need to send more fragments
          flags = SET_MORE_FRAGMENTS(flags);
          if (first_fragment)
          {
            EAP_LOG(LM_DEBUG,"AUTH: FIRST FRAGMENT\n");
            flags = SET_LENGTH_INCLUDED(flags);
            first_fragment = false;
          }
          state = EvSgMoreFragments;
      }
      else
      {
        state = EvSgNoMoreFragments;
        first_fragment=true; //Restore first fragment value; 
        EAP_LOG(LM_DEBUG,"AUTH: NO FRAGMENTS\n");
      }
      session_auth->set_length_to_send(length_to_send); //Length will be sent by next state.
      session_auth->set_flags_to_send(flags);
      msm.Event(state);
    }
  };

  class AcSendFragment : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "<--------------AuthFast: Send Fragment.\n");

      EAPFAST_session_t_auth *session_auth = msm.get_fast_session();
      AAAMessageBlock *data = session_auth->get_dirty_out(); //Getting FAST records to be encapsulated in request

      ACE_UINT32 length_to_send=session_auth->get_length_to_send();
      ACE_Byte flags=session_auth->get_flags_to_send();

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(length_to_send);
      ACE_OS::memset(msg->base(), 0, length_to_send);

      EapRequestFast request(flags|SET_VERSION(0x01));
      request.set_data(data);
      EapRequestFastParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    }
      if (FAST_LENGTH_INCLUDED(flags)) data->rd_ptr(length_to_send-6-4);
      else data->rd_ptr(length_to_send-6);      //Move pointer to next bytes (next future fragment)
      EAP_LOG(LM_DEBUG,"LENGTH NEXT FRAGMENT %d\n",data->length());
      // Set the message to the session.
      ssm.SetTxMessage(msg);
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
     }
  };
  
  class AcRecvAck : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();

      //Msg contains complete message.rd_ptr() points data section. 
      //(from byte after Type) and wr_ptr() at the end of Message
      AAAMessageBlock *msg = ssm.GetRxMessage();   
      EAP_LOG(LM_DEBUG, "AuthFast: Processing ACK.\n");

      // Check the EAP-FAST response.
      EapResponseFast response(SET_VERSION(0x01));
      EapResponseFastParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch	(...) {
        EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }
      if (response.get_is_ack()) msm.Event(EvSgValid);
      else msm.Event(EvSgInvalid);
    }
  };

  class AcSendAck: public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "<----------- AuthFast: Send ACK.\n");
      //[Code(1) + identifier (1) + length(2) + type(1)] + flags(1)
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(5+1); 
      ACE_OS::memset(msg->base(), 0, 5+1);
      EapRequestFast request(SET_VERSION(0x01)); //A request with S bit enabled.
      EapRequestFastParser  parser;
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	      EAP_LOG(LM_ERROR, "AuthFast: Parse error.\n");
	      msm.Event(EvSgInvalid);
	      return;
	    }
      // Set the message to the session.
      ssm.SetTxMessage(msg);
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcVerifyPeerMoreFragments : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAPFAST_session_t_auth *session_auth = msm.get_fast_session();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      msg->wr_ptr(msg->end());
      EAP_LOG(LM_DEBUG, "-------------->AutFast: Verify Peer More Fragments.\n");

      ACE_UINT32 state = 0;
      ACE_UINT32 length_fragment = 0;

      EapResponseFast response(SET_VERSION(0x01));
      response.set_is_fragment(true);
      EapResponseFastParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    }

      static bool first_fragment = true;
      AAAMessageBlock *data=response.get_data();
      if (data != NULL) length_fragment=data->length();
      else length_fragment = 0;

      //This variable could be true if it is not fragmented or it is first fragmented
      if (FAST_MORE_FRAGMENTS(response.get_flags())) 
      {
         EAP_LOG(LM_DEBUG,"AuthFast : MORE FRAGMENTS \n");
         if (FAST_LENGTH_INCLUDED(response.get_flags()) && first_fragment) //There is no fragments
         {
           EAP_LOG(LM_DEBUG,"AuthFast : LENGTH INCLUDED \n");
            length_fragment = response.get_fast_message_length();
            if (response.get_fast_message_length() > 0)
            {
              session_auth->set_dirty_in(response.get_data());
              first_fragment = false;
              state=EvSgMoreFragments;
            }
            else state=EvSgInvalid;
         }
         else if (!FAST_LENGTH_INCLUDED(response.get_flags()) && !first_fragment)
         {
            EAP_LOG(LM_DEBUG,"AuthFast : LENGTH INCLUDED \n");
            if (response.get_data()->length() > 0)
            {
              state=EvSgMoreFragments;
              session_auth->append_dirty_in(response.get_data());
            }
            else state=EvSgInvalid;
         }
         else state = EvSgInvalid;
      }
      else    
      {
          EAP_LOG(LM_DEBUG,"AuthFast : NO MORE FRAGMENTS \n");
          first_fragment ? 
		session_auth->set_dirty_in(response.get_data()) : 
		session_auth->append_dirty_in(response.get_data());
          first_fragment = true;
	 
	  state = EvSgNoMoreFragments;
      }
      msm.Event(state);
    }
  };

  class AcProcessResponseSecondWay : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAPFAST_fast_mng_auth &fast_mng_auth = msm.get_mng_auth();
      EAPFAST_session_t_auth *session_auth = msm.get_fast_session();
      
      EAP_LOG(LM_DEBUG, "-------------->AuthFast: Process response second way message.\n");

	ACE_INT32 err;
      if((err = fast_mng_auth.tls_handshake_recv(session_auth)) == EAPFAST_fast_mng::StAlertReceive)
      {
        EAP_LOG(LM_ERROR, "AuthTls: AcProcessResponseSecondWay: AlertReceive.\n");
      	msm.Event(EvSgAlertReceive);
	return;
      }
      else if (err == EAPFAST_fast_mng::StAlertSend)
      {
        EAP_LOG(LM_ERROR, "AuthTls: AcProcessResponseSecondWay: AlertSend.\n");
      	msm.Event(EvSgAlertSend);
	return;
      }

      char cipher[64];
      FAST_data *fast = session_auth->get_fast_data(); 
      if (tls_get_cipher(fast, cipher, sizeof(cipher))
	    < 0) {
		printf("EAP-FAST: Failed to get cipher "
			   "information");
		//return -1;
	}

      eap_fast_derive_key_auth(fast);
      
      //ssm.DeleteRxMessage();
      ssm.Notify(EapAuthSwitchStateMachine::EvSgTunnelEstablished);

    }  
  };
  
  class AcNotifySuccess : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EAP_LOG(LM_DEBUG, "<-----------------------------AuthFast: Notify EAP success.\n");
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAPFAST_session_t_auth *session_auth = msm.get_fast_session();

      session_auth->session_close();

      std::string mk(session_auth->get_master_key()->base(), 
			session_auth->get_master_key()->size());
      msm.KeyData() = mk; 
      ssm.KeyAvailable() = true;

      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnSuccess);
      
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);

    }
  };


  class AcNotifyInvalid : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EAP_LOG(LM_DEBUG,"<-------------------------------AuthFast: AcNotifyInvalid\n");
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Notify(EapAuthSwitchStateMachine::EvSgInvalidResp);
    }
  };

  class AcBuildRequestAlert : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
	EAP_LOG(LM_DEBUG,"<-------------------------AuthFast: AcBuildRequestAlert\n");

	EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();

	EAPFAST_session_t_auth *session_auth = msm.get_fast_session();
	AAAMessageBlock *data = session_auth->get_dirty_out(); //Getting FAST records to be encapsulated in request
	AAAMessageBlock *msg = AAAMessageBlock::Acquire(6+data->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
	ACE_OS::memset(msg->base(), 0, 6+data->length());

	EapRequestFast request(SET_VERSION(0x01));

	request.set_data(data);

	EapRequestFastParser parser;
	parser.setAppData(&request);
	parser.setRawData(msg);
	try { parser.parseAppToRaw(); }
	catch (...) {
		EAP_LOG(LM_ERROR, "AuthFast: Parse error.\n");
		msm.Event(EvSgInvalid);
		return;
	}
      
	// Set the message to the session.
	ssm.SetTxMessage(msg);

	// Send a "valid" signal to the switch state machine.
	ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcProcessResponseAckAlert : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
	EAP_LOG(LM_DEBUG,"---------------------->AuthFast: AcProcessResponseAckAlert\n");

	EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
	AAAMessageBlock *msg = ssm.GetRxMessage();

	EapResponseFast response(SET_VERSION(0x01));
	EapResponseFastParser parser;
	parser.setAppData(&response);
	parser.setRawData(msg);

	try { parser.parseRawToApp(); }
	catch (...) {
		EAP_LOG(LM_ERROR, "AuthFast: Parse error.\n");
		msm.Event(EvSgInvalid);
		return;
	}

	EAPFAST_session_t_auth *session_auth = msm.get_fast_session();
	session_auth->set_dirty_in(response.get_data());
      
	// Proceed to the next step.
	msm.Event(EvSgValid);
    }
  };

  class AcNotifyFailure : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
	EAP_LOG(LM_DEBUG,"<--------------------------AuthFast: AcNotifyFailure\n");
	EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
	EAPFAST_session_t_auth *session_auth = msm.get_fast_session();
    
	session_auth->session_close();
	ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnFailure);
	msm.IsDone() = true;
	ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };
  enum{
	DecisionSuccess =0 ,
  };
  class AcReceiveInnerEap : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAPFAST_session_t_auth *session_auth = msm.get_fast_session();
      FAST_data *fast = session_auth->get_fast_data();
      EAP_LOG(LM_DEBUG, "-------------->AuthFast: Receive from Inner EAP.\n");
      char *buf = NULL;
      size_t len = 0;
      u8 err;

      if(ssm.InnerMethodStateMachine().IsDone())
      {
	if( ssm.Decision() == DecisionSuccess){
		err = eap_fast_build_phase2_req(fast, EAP_CODE_SUCCESS, &buf, &len);
	}
	else err = eap_fast_build_phase2_req(fast, EAP_CODE_FAILURE, &buf, &len);
      }
      else 
      {

     	AAAMessageBlock *innerMsg = ssm.GetTxMessage();

      	len = innerMsg->length();
      	buf = (char*)malloc(innerMsg->length());
      	memcpy(buf, innerMsg->base(), len);
      	ssm.DeleteTxMessage();
      
      	err = eap_fast_build_phase2_req(fast,EAP_CODE_REQUEST, &buf, &len);
      }
      if(err) {
	EAP_LOG(LM_ERROR, "AuthFast: inner EAP error.\n");
	msm.Event(EvSgInvalid);
	return;
      }
      
      AAAMessageBlock *tunnelData = AAAMessageBlock::Acquire(len);	
      tunnelData->copy(buf,len);

      free(buf);

      EAPFAST_fast_mng_auth &fast_mng_auth = msm.get_mng_auth();
      ACE_INT32 frg_len = session_auth->get_fragment_size();
      buf = (char*)malloc(frg_len);
      
	AAAMessageBlock *tlsData = session_auth->get_dirty_out();
	size_t tlsDataLen = tlsData->length();
        if(tlsDataLen > 0){
	  EAP_LOG(LM_DEBUG, "There is TLS handshake message. "
			"The length of the message is %d\n", tlsDataLen);
	  memcpy(buf,tlsData->rd_ptr(),tlsDataLen);
        }
       
      session_auth->set_dirty_in(tunnelData);

      int result = fast_mng_auth.tls_connection_encrypt (session_auth);
      if (result < 0){msm.Event(EvSgNone);}

      tunnelData = session_auth->get_dirty_out();
      size_t tunnelDataLen = tunnelData->length();
      
      AAAMessageBlock *data;

      memcpy(buf + tlsDataLen, tunnelData->rd_ptr(), tunnelDataLen);
      data = AAAMessageBlock::Acquire (tlsDataLen + tunnelDataLen);
      data->copy(buf,tlsDataLen + tunnelDataLen );

      session_auth->get_dirty_out()->reset();

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(6+data->length()); 
	//Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)

      ACE_OS::memset(msg->base(), 0, 6+data->length());

      EapRequestFast request(SET_VERSION(0x01)); 
      EapRequestFastParser  parser;
      request.set_data(data);
      
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      ssm.SetTxMessage(msg);
      msm.Event(EvSgValid);
    }
  };
 class AcSendTunnelMessage : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      
      EAP_LOG(LM_DEBUG, "<--------------AuthFast: Send Tunnel Message.\n");
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };
  class AcReceiveTunnelMessage : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();

      EAPFAST_fast_mng_auth &fast_mng_auth = msm.get_mng_auth();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "------------------->AuthFast: AcReceiveTunnelMessage.\n");

      EapResponseFast response(SET_VERSION(0x01));
      EapResponseFastParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      EAPFAST_session_t_auth *session_auth = msm.get_fast_session();
      FAST_data * fast = session_auth->get_fast_data();

      AAAMessageBlock * block = response.get_data();
      session_auth->get_dirty_in()->reset();
      session_auth->set_dirty_in (block);

      fast_mng_auth.tls_connection_decrypt(session_auth);

      AAAMessageBlock *  tunnelMsg = session_auth->get_dirty_out();

      char *resp = NULL;
      char *eapPayload = NULL;
      size_t respLen = 0; size_t eapPayloadLen = 0;

      int err = eap_fast_process_phase2_resp(fast, EAP_CODE_RESPONSE, 
			tunnelMsg->base(), tunnelMsg->length(),  
			&resp, &respLen, &eapPayload, &eapPayloadLen );

      session_auth->get_dirty_out()->reset();

      if(!err && !resp){
	switch(fast-> result){
	  case (EAP_TLV_RESULT_SUCCESS) :
		msm.Event(EvSgSuccess);
		return;
	  case (EAP_TLV_RESULT_FAILURE) :
		msm.Event(EvSgFailure);
		return;
	  default:
	    block = AAAMessageBlock::Acquire(eapPayloadLen);
      	    block->copy(eapPayload, eapPayloadLen);
	    block->rd_ptr(block->base()+5);
      	    ssm.SetRxMessage(block);
	    ssm.Notify(EapAuthSwitchStateMachine::EvSgInnerEap);
	    return;
	}
      }
      if(!err && resp){
	AAAMessageBlock *tlvs = AAAMessageBlock::Acquire(respLen);
        tlvs->copy(resp, respLen);
        free(resp);

	ssm.SetTxMessage(tlvs);
	msm.Event(EvSgTlvsBuild);
	return;
      }
    }
  };

  class AcReceiveTlvs : public EapAuthFastAction
  {
    void operator()(EapAuthFastStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAPFAST_session_t_auth *session_auth = msm.get_fast_session(); 
      EAP_LOG(LM_DEBUG, "-------------->AuthFast: Receive TLVs.\n");

      AAAMessageBlock *tlvs = ssm.GetTxMessage();

      EAPFAST_fast_mng_auth &fast_mng_auth = msm.get_mng_auth();
       
      session_auth->set_dirty_in(tlvs);

      int result = fast_mng_auth.tls_connection_encrypt (session_auth);
      if (result < 0){msm.Event(EvSgNone);}

      AAAMessageBlock *data = session_auth->get_dirty_out();   

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(6+data->length()); 
	//Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)

      ACE_OS::memset(msg->base(), 0, 6+data->length());

      EapRequestFast request(SET_VERSION(0x01)); 
      EapRequestFastParser  parser;
      request.set_data(data);
      
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }
      session_auth->get_dirty_out()->reset();

      ssm.SetTxMessage(msg);
      msm.Event(EvSgValid);
    }
  };

  
  enum {
    EvSgValid,
    EvSgInvalid,
    EvSgMoreFragments,
    EvSgNoMoreFragments,
    EvSgAlertReceive,
    EvSgAlertSend,
    EvSgNone,
    EvSgTlvsBuild,
    EvSgSuccess,
    EvSgFailure,
  };
  enum state {
    StInitialize = 1,
    StBuildStart = 2,
    StProcessResponseClientHello = 3,
    StVerifyAuthMoreFragments = 4 ,
    StRecvAck = 5,
    StVerifyPeerMoreFragments = 6,
    StProcessResponseSecondWay = 7,
    StSentRequestFinish = 8,
    StProcessResponseFinish = 9,
  
    StTunnelEstablish = 10,
    
    StSuccess = 13,
    StBuildRequestAlert = 14,
    StProcessResponseAckAlert = 15,
    StFailure = 16,
//tunnel
    StReceiverInnerEap = 17,
    StSendTunnelMessage = 18,
    StReceiveTunnelMessage = 19,
 }; 


  AcBuildStart 			acBuildStart;
  AcProcessResponseClientHello 	acProcessResponseClientHello;
  AcVerifyAuthMoreFragments 	acVerifyAuthMoreFragments;
  AcRecvAck 			acRecvAck;
  AcSendFragment 		acSendFragment; 
  AcSendAck 			acSendAck;
  AcProcessResponseSecondWay 	acProcessResponseSecondWay;
  AcVerifyPeerMoreFragments 	acVerifyPeerMoreFragments;
  AcNotifySuccess		acNotifySuccess;
  AcNotifyInvalid 		acNotifyInvalid;
  AcBuildRequestAlert 		acBuildRequestAlert;
  AcProcessResponseAckAlert 	acProcessResponseAckAlert;
  AcNotifyFailure 		acNotifyFailure;
//tunnel
  AcReceiveInnerEap		acReceiveInnerEap;
  AcSendTunnelMessage		acSendTunnelMessage;
  AcReceiveTunnelMessage	acReceiveTunnelMessage;
  AcReceiveTlvs			acReceiveTlvs;
 
  EapAuthFastStateTable_S()                        //Constructor.
  {
    	AddStateTableEntry(StInitialize,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StBuildStart, acBuildStart);
    	AddStateTableEntry(StInitialize, StInitialize, 0);

    	AddStateTableEntry(StBuildStart,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessResponseClientHello, acProcessResponseClientHello);
    	AddStateTableEntry(StBuildStart, StBuildStart, 0);

    	AddStateTableEntry(StProcessResponseClientHello, EvSgInvalid,
		       StInitialize, acNotifyInvalid);
    	AddStateTableEntry(StProcessResponseClientHello, EvSgValid,
		       StVerifyAuthMoreFragments, acVerifyAuthMoreFragments);
    	
	AddStateTableEntry(StProcessResponseClientHello, EvSgAlertReceive,
			StFailure, acNotifyFailure);
	AddStateTableEntry(StProcessResponseClientHello, EvSgAlertSend,
			StBuildRequestAlert, acBuildRequestAlert);

	AddStateTableEntry(StBuildRequestAlert,
			EapMethodStateMachine::EvSgIntegrityCheck,
			StProcessResponseAckAlert, acProcessResponseAckAlert);

	AddStateTableEntry(StBuildRequestAlert, StBuildRequestAlert, 0);
	AddStateTableEntry(StProcessResponseAckAlert, EvSgInvalid,
			StInitialize, acNotifyInvalid);
	AddStateTableEntry(StProcessResponseAckAlert, EvSgValid,
			StFailure, acNotifyFailure);

    	AddStateTableEntry(StVerifyAuthMoreFragments,EvSgMoreFragments,
		       StRecvAck, acSendFragment);
   	AddStateTableEntry(StVerifyAuthMoreFragments,EvSgNoMoreFragments,
		       StProcessResponseSecondWay, acSendFragment);
     	AddStateTableEntry(StVerifyAuthMoreFragments, EvSgInvalid,
		       StInitialize, acNotifyInvalid);
                   
    	AddStateTableEntry(StRecvAck,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessResponseClientHello, acRecvAck);
    	AddStateTableEntry(StRecvAck,StRecvAck, 0);

    	AddStateTableEntry(StProcessResponseSecondWay,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StVerifyPeerMoreFragments, acVerifyPeerMoreFragments);
    	AddStateTableEntry(StProcessResponseSecondWay,StProcessResponseSecondWay, 0);

   	AddStateTableEntry(StVerifyPeerMoreFragments, EvSgMoreFragments,
		       StProcessResponseSecondWay, acSendAck);
    	AddStateTableEntry(StVerifyPeerMoreFragments, EvSgNoMoreFragments,
		       StTunnelEstablish, acProcessResponseSecondWay);
     
    	AddStateTableEntry(StVerifyPeerMoreFragments, EvSgInvalid,
		       StInitialize, acNotifyInvalid);

//tunnel
	AddStateTableEntry(StTunnelEstablish, EapMethodStateMachine::EvSgTunnelProcess,
			StReceiverInnerEap, acReceiveInnerEap);
	AddStateTableEntry(StReceiverInnerEap, EvSgValid,
			StSendTunnelMessage, acSendTunnelMessage);
	AddStateTableEntry(StReceiverInnerEap, EvSgInvalid,
			StInitialize, acNotifyInvalid);
	AddStateTableEntry(StSendTunnelMessage, EapMethodStateMachine::EvSgIntegrityCheck,
			StReceiveTunnelMessage, acReceiveTunnelMessage);
	AddStateTableEntry(StReceiveTunnelMessage, EapMethodStateMachine::EvSgTunnelProcess,
			StReceiverInnerEap, acReceiveInnerEap);
	AddStateTableEntry(StReceiveTunnelMessage, EvSgTlvsBuild,
			StReceiverInnerEap, acReceiveTlvs);
	AddStateTableEntry(StReceiveTunnelMessage, EvSgSuccess,
			StSuccess, acNotifySuccess);
	AddStateTableEntry(StReceiveTunnelMessage, EvSgFailure,
			StFailure, acNotifyFailure);


	AddStateTableEntry(StFailure, StFailure, 0);
           
    	AddStateTableEntry(StSuccess, StSuccess, 0);
    
    	InitialState(StInitialize);
  } // leaf class
  ~EapAuthFastStateTable_S() {}
};

///---------------------------------------------------------------EapPeerFastState--------------------------------------------------------------------

/// State table used by EapPeerFastStateMachine.
class EAP_FAST_EXPORTS EapPeerFastStateTable_S :
  public AAA_StateTable<EapPeerFastStateMachine>

{
  friend class ACE_Singleton<EapPeerFastStateTable_S, ACE_Recursive_Thread_Mutex>;

private:

  class AcProcessStart : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      unsigned char * auth_id_data;
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAPFAST_fast_mng_peer &fast_mng_peer = msm.get_mng_peer();
      AAAMessageBlock *msg = ssm.GetRxMessage();   //Msg contains complete message.rd_ptr() points data section. (from byte after Type) and wr_ptr() at the end of Message
      EAP_LOG(LM_DEBUG, "PeerFast: Processing EAP-FAST Start.<-----------------\n");

      // Check the EAP-FAST response.
      EapRequestFast request(SET_START(0x00)|SET_VERSION(0x01));
      EapRequestFastParser parser;
      parser.setAppData(&request);  
      parser.setRawData(msg);  
      try { parser.parseRawToApp(); } 
      catch	(...) {
        EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
        msm.Event(EvSgInvalid); 
        return; 
      }
      u8 flags = request.get_flags();

      EAPFAST_session_t_peer *session_peer = new EAPFAST_session_t_peer(msm.get_ctx_peer());
      session_peer->session_init(false);
      
      AAAMessageBlock * block = request.get_data();
      if(block ==NULL) {msm.Event(EvSgInvalid);}
      else {
      	auth_id_data = (unsigned char *) request.get_data()->base();

       	int tmp= eap_fast_peer_process_start(
		session_peer->get_peer_context(),
		session_peer->get_fast_data(), 
		flags,auth_id_data);
	
      	msm.set_fast_session(session_peer); //Build a new session.
	
      	session_peer->set_dirty_in(NULL);
	
      	fast_mng_peer.tls_handshake_recv(session_peer);
      	msm.Event(EvSgValid);
      }
    }
  };

  class AcBuildResponseClientHello : public EapPeerFastAction 
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "PeerFast: Building a Client Hello Message.--------------->\n");

      EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
      AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting FAST records to be encapsulated in request
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(6+data->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)

      ACE_OS::memset(msg->base(), 0, 6+data->length());

      EapResponseFast response(SET_VERSION(0x01)); 
      EapResponseFastParser  parser;
      response.set_data(data);
      
      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      // Set the message to the session.
      ssm.SetTxMessage(msg);
           // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::CONT;

      // Send a "valid" signal to the switch state machine.
      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);

    }
  };

  class AcVerifyAuthMoreFragments : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
      EAPFAST_fast_mng_peer &fast_mng_peer = msm.get_mng_peer();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      //msg->wr_ptr(msg->end());
      EAP_LOG(LM_DEBUG, "PeerFast: Verify Auth More Fragments.<--------------\n");

      ACE_UINT32 state = 0;
      EapResponseFast response(SET_VERSION(0x01));
      response.set_is_fragment(true);
      EapResponseFastParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;                                          
	    }

      static bool first_fragment = true;


      if (FAST_MORE_FRAGMENTS(response.get_flags())) //This variable could be true if it is not fragmented or it is first fragmented
      {
        EAP_LOG(LM_DEBUG,"PeerFast : AUTH MORE_FRAGMENTS\n");
         if (FAST_LENGTH_INCLUDED(response.get_flags()) && first_fragment) //There is no fragments
         {
            if (response.get_fast_message_length() > 0)
            {
              session_peer->set_dirty_in(response.get_data());
              first_fragment = false;
              state = EvSgMoreFragments;
            }
            else state = EvSgInvalid;
         }
         else if (!FAST_LENGTH_INCLUDED(response.get_flags()) && !first_fragment)
         {
            if (response.get_data()->length() > 0)
            {
              state = EvSgMoreFragments;
              session_peer->append_dirty_in(response.get_data());
            }
            else state = EvSgInvalid;
         }
         else state = EvSgInvalid;
      }
      else
      {
          first_fragment ? session_peer->set_dirty_in(response.get_data()) : session_peer->append_dirty_in(response.get_data());
          first_fragment = true;

	  ACE_INT32 err;
          
          if((err = fast_mng_peer.tls_handshake_recv(session_peer)) == EAPFAST_fast_mng::StAlertReceive)
          {
		EAP_LOG(LM_ERROR, "PeerFast: AcVerifyAuthMoreFragments: AlertReceive.\n");
      		msm.Event(EvSgAlertReceive);
		return;
          }
          else if (err == EAPFAST_fast_mng::StAlertSend)
          {
		EAP_LOG(LM_ERROR, "PeerFast: AcVerifyAuthMoreFragments: AlertSend.\n");
      		msm.Event(EvSgAlertSend);
		return;
      	  }
          else{

          //It is a packet without fragmentation.
          state = EvSgNoMoreFragments;
	  }
      }
      msm.Event(state);
    }
  };

  class AcSendAck: public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "PeerFast: Send ACK.-----------> \n");
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(5+1); //[Code(1) + identifier (1) + length(2) + type(1)] + flags(1)
      ACE_OS::memset(msg->base(), 0, 5+1);
      EapRequestFast request(SET_VERSION(0x01)); //A request with S bit enabled.
      EapRequestFastParser  parser;
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	      EAP_LOG(LM_ERROR, "AuthFast: Parse error.\n");
	      msm.Event(EvSgInvalid);
	      return;
	    }
      // Set the message to the session.
      ssm.SetTxMessage(msg);
             // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::CONT;
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapPeerSwitchStateMachine::EvSgValidReq);
    }
  };

  class AcVerifyPeerMoreFragments : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      ACE_Byte flags=0x00;
      ACE_UINT32 state=0;

      EAP_LOG(LM_DEBUG, "PeerFast: Verify Peer More Fragments.-------------->\n");

      EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
      AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting FAST records to be encapsulated in request

      ACE_INT32 length_to_send=data->length()+6; //+6 (Header length) =//Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+Data(n)
      ACE_INT32 length_fragment=session_peer->get_fragment_size();
      static bool first_fragment=true;

      if ((length_fragment != 0) && (length_fragment < length_to_send))
      {
        EAP_LOG(LM_DEBUG,"PeerFast : MORE FRAGMENTS \n");
        length_to_send = length_fragment; //We need to send more fragments
        flags = SET_MORE_FRAGMENTS(flags);
        if (first_fragment)
        {
          EAP_LOG(LM_DEBUG,"PeerFast : LENGTH INCLUDED\n");
          flags = SET_LENGTH_INCLUDED(flags);
          first_fragment=false;
        }
        state = EvSgMoreFragments;
      }
      else {
        EAP_LOG(LM_DEBUG,"PeerFast : NO MORE FRAGMENTS \n");

      // if(session_peer->get_fast_data()->ssl_data->enc_read_ctx)
	//state = EvSgOpaqueValid;
       //else 
        state = EvSgNoMoreFragments;
        first_fragment=true; //Restore first fragment value;
      }
      session_peer->set_length_to_send(length_to_send); //Length will be sent by next state.
      session_peer->set_flags_to_send(flags);
      msm.Event(state);
    }
  };

  class AcSendFragment : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "PeerFast: Send Fragment.-------------->\n");

      EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
      AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting FAST records to be encapsulated in request

      ACE_INT32 length_to_send=session_peer->get_length_to_send();
      ACE_Byte flags=session_peer->get_flags_to_send();
       EAP_LOG(LM_ERROR, "PeerFast: LENGTH TO SEND %d.\n",length_to_send);
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(length_to_send);
      ACE_OS::memset(msg->base(), 0, length_to_send);

      EapRequestFast request(flags|SET_VERSION(0x01));
      request.set_data(data);
      EapRequestFastParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    }
      if (FAST_LENGTH_INCLUDED(flags)) data->rd_ptr(length_to_send-6-4);
      else data->rd_ptr(length_to_send-6);      //Move pointer to next bytes (next future fragment)
      // Set the message to the session.
      ssm.SetTxMessage(msg);
      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::CONT;
      // Send a "valid" signal to the switch state machine.
      ssm.Event(EapAuthSwitchStateMachine::EvSgValidResp);
     }
  };

  class AcRecvAck : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();   //Msg contains complete message.rd_ptr() points data section. (from byte after Type) and wr_ptr() at the end of Message
      EAP_LOG(LM_DEBUG, "PeerFast: Processing ACK.<--------------\n");

      // Check the EAP-FAST response.
      EapResponseFast response(SET_VERSION(0x01));
      EapResponseFastParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch	(...) {
        EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }
      if (response.get_is_ack()) msm.Event(EvSgNoMoreFragments);
      else msm.Event(EvSgInvalid);
    }
  };

  class AcProcessRequestFinish : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAPFAST_fast_mng_peer &fast_mng_peer = msm.get_mng_peer();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerFast: Process Request Finish.<-------------------\n");

      EapRequestFast request(SET_VERSION(0x01));
      EapRequestFastParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
      if(request.get_is_application()){
	goto APPLICATION;
      }
      
      session_peer->set_dirty_in(request.get_data());

	ACE_INT32 err;

      if((err = fast_mng_peer.tls_handshake_recv(session_peer)) 
		== EAPFAST_fast_mng::StAlertReceive)
      {
	msm.Event(EvSgAlertReceive);
      }
      else if (err == EAPFAST_fast_mng::StAlertSend)
      {
	msm.Event(EvSgAlertSend);
      }
      else
            
      // Proceed to the next step.
      {

APPLICATION:
	char cipher[64];
        FAST_data *fast = session_peer->get_fast_data(); 
        if (tls_get_cipher(fast, cipher, sizeof(cipher))
	    < 0) {
		EAP_LOG(LM_ERROR, "EAP-FAST: Failed to get cipher "
			   "information");
		return;
	}

        eap_fast_derive_key_auth(fast);
	
	if(!request.get_is_piggyback()&& !request.get_is_application())
      	{
          EAP_LOG(LM_DEBUG, "PeerFast: ACK prepared.\n");
	  msm.Event(EvSgValid);
	  return;
 	}

        if(request.get_is_piggyback())
	{
	  AAAMessageBlock *tunnelMsg = request.get_piggy();
	
	  msg = AAAMessageBlock::Acquire(6 + tunnelMsg->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
	  ACE_OS::memset(msg->base(), 0, 6 + tunnelMsg->length());

	  EapRequestFast newRequest(SET_VERSION(0x01));
	  newRequest.set_data(tunnelMsg);

	  parser.setAppData(&newRequest);
	  parser.setRawData(msg);

	  try { parser.parseAppToRaw(); }
	  catch (...) {
		EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
		msm.Event(EvSgInvalid);
		return;
	  }
	 
	  msg->rd_ptr(5);//EAP header occupy 4 bytes and 5th byte is the type. Now the rd_ptr is point to the flag

	  ssm.SetRxMessage(msg);
	}

	msm.Event(EvSgApplicationData);
	  
      }
    }
  };

  class AcNotifySuccess : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
        EAP_LOG(LM_DEBUG, "PeerFast: Notify EAP success.------------------------->\n");
	EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
	EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
	session_peer->session_close();
        std::string mk(session_peer->get_master_key()->base(), 
			session_peer->get_master_key()->size());
        msm.KeyData() = mk; 
	ssm.KeyAvailable() = true;
	ssm.Decision() = EapPeerSwitchStateMachine::UNCOND_SUCC;
	ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
	ssm.MethodState() = EapPeerSwitchStateMachine::DONE;
    }
  };
  class AcNotifyFailure : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
	EAP_LOG(LM_DEBUG, "PeerFast: Notify EAP failure.------------------------->\n");
	EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
 	EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
	ssm.Decision() = EapPeerSwitchStateMachine::FAIL;
	ssm.MethodState() = EapPeerSwitchStateMachine::DONE;
	session_peer->session_close();
    }
  };

 
 
  class AcNotifyInvalid : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EAP_LOG(LM_DEBUG,"PeerFast: AcNotifyInvalid.------------------------->\n");
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      ssm.Event(EapPeerSwitchStateMachine::EvSgInvalidReq);
    }
  };

  class AcBuildResponseAckAlert : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
	EAP_LOG(LM_DEBUG,"PeerFast: AcBuildResponseAckAlert------------------------->\n");	
	EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

	EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
	AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting FAST records to be encapsulated in request
	AAAMessageBlock *msg = AAAMessageBlock::Acquire(6 + data->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
	ACE_OS::memset(msg->base(), 0, 6 + data->length());

	EapResponseFast response(SET_VERSION(0x01));
	EapResponseFastParser parser;
	response.set_data(data);

	parser.setAppData(&response);
	parser.setRawData(msg);

	try { parser.parseAppToRaw(); }
	catch (...) {
		EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
		msm.Event(EvSgInvalid);
		return;
	}

	session_peer->session_close(); //This session is finished.
	// Set the message to the session.
	ssm.SetTxMessage(msg);
	ssm.Decision() = EapPeerSwitchStateMachine::FAIL;

	// Update external method state.
	ssm.MethodState() = EapPeerSwitchStateMachine::DONE;
	ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
    }
  };

  class AcBuildResponseAlert : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
	EAP_LOG(LM_DEBUG,"PeerFast: AcBuildResponseAlert.------------------------->\n");
	EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

	EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
	AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting FAST records to be encapsulated in request
	AAAMessageBlock *msg = AAAMessageBlock::Acquire(6 + data->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
	ACE_OS::memset(msg->base(), 0, 6 + data->length());

	EapResponseFast response(SET_VERSION(0x01));
	EapResponseFastParser parser;
	response.set_data(data);

	parser.setAppData(&response);
	parser.setRawData(msg);

	try { parser.parseAppToRaw(); }
	catch (...) {
		EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
		msm.Event(EvSgInvalid);
		return;
	}

	session_peer->session_close(); //This session is finished.
	// Set the message to the session.
	ssm.SetTxMessage(msg);
	ssm.Decision() = EapPeerSwitchStateMachine::FAIL;

	// Update external method state.
	ssm.MethodState() = EapPeerSwitchStateMachine::DONE;
	ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
    }
  };

//tunnel
  class AcSendTunnelMessage : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EAP_LOG(LM_DEBUG,"PeerFast: AcSendTunnelMessage.------------------------->\n");
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
      FAST_data * fast = session_peer->get_fast_data();

      switch(fast->result){
	case (EAP_TLV_RESULT_SUCCESS): 
	  msm.Event(EvSgSuccess);
	  break;
 	case (EAP_TLV_RESULT_FAILURE):
	  msm.Event(EvSgFailure);
	  break;
	default:
	  ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
      	  EAP_LOG(LM_DEBUG,"PeerFast: Wait tunneled Message\n");
      }
      
    }
  };
  class AcReceiveTunnelMessage : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

      EAPFAST_fast_mng_peer &fast_mng_peer = msm.get_mng_peer();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerFast: AcReceiveTunnelMessage.<-------------------\n");

      EapRequestFast request(SET_VERSION(0x01));
      EapRequestFastParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
      FAST_data * fast = session_peer->get_fast_data();

      AAAMessageBlock * block = request.get_data();

      session_peer->get_dirty_in()->reset();
      session_peer->set_dirty_in (block);

      fast_mng_peer.tls_connection_decrypt(session_peer);
      AAAMessageBlock *  tunnelMsg = session_peer->get_dirty_out();
      char *resp = NULL;
      char *eapPayload = NULL;

      size_t respLen = 0; size_t eapPayloadLen = 0;

      int err = eap_fast_process_phase2_req(fast,EAP_CODE_REQUEST, 
			tunnelMsg->base(), tunnelMsg->length(),  
			&resp, &respLen, &eapPayload, &eapPayloadLen );

      session_peer->get_dirty_out()->reset();


      if(!err && !resp){
	block = AAAMessageBlock::Acquire(eapPayloadLen);
      	block->copy(eapPayload, eapPayloadLen);
	block->rd_ptr(block->base()+5);

      	ssm.SetRxMessage(block);
	ssm.Event(EapPeerSwitchStateMachine::EvSgTunnelEstablished);
	
      }
      else{
	if(resp && !err){
	  AAAMessageBlock *tlvs = AAAMessageBlock::Acquire(respLen);
          tlvs->copy(resp, respLen);
          free(resp);
	  ssm.SetTxMessage(tlvs);
	  msm.Event(EvSgTlvsBuild);;	  
	}
      }

    }
  };
  class AcReceiveInnerEap : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
      FAST_data *fast = session_peer->get_fast_data();
      EAP_LOG(LM_DEBUG, "PeerFast: Receive from Inner EAP.<--------------\n");

      AAAMessageBlock *innerMsg = ssm.GetTxMessage();

      size_t len = innerMsg->length();
      char *buf = (char*)malloc(innerMsg->length());
      memcpy(buf, innerMsg->base(), len);
      ssm.DeleteTxMessage();
      
      int err = eap_fast_build_phase2_resp(fast, EAP_CODE_RESPONSE , &buf, &len);

      if(err){
	 EAP_LOG(LM_ERROR, "PeerFast: inner EAP error.\n");
	 msm.Event(EvSgInvalid);
	 return;
      }

      
      AAAMessageBlock *tunnelData = AAAMessageBlock::Acquire(len);	
      tunnelData->copy(buf,len);

      free(buf);

      EAPFAST_fast_mng_peer &fast_mng_peer = msm.get_mng_peer();
      ACE_INT32 frg_len = session_peer->get_fragment_size();
      buf = (char*)malloc(frg_len);


	AAAMessageBlock *tlsData = session_peer->get_dirty_out();
	size_t tlsDataLen = tlsData->length();
        if(tlsDataLen > 0){
	  EAP_LOG(LM_DEBUG, "There is TLS handshake message." 
			"The length of the message is %d\n", tlsDataLen);
	  memcpy(buf,tlsData->rd_ptr(),tlsDataLen);
        }


       
      session_peer->set_dirty_in(tunnelData);

      int result = fast_mng_peer.tls_connection_encrypt (session_peer);
      //if (result < 0){msm.Event(EvSgNone);}

      tunnelData = session_peer->get_dirty_out();
      size_t tunnelDataLen = tunnelData->length();
      AAAMessageBlock *data;

      memcpy( buf + tlsDataLen, tunnelData->rd_ptr(), tunnelDataLen);
      data = AAAMessageBlock::Acquire (tlsDataLen + tunnelDataLen);
      data->copy( buf, tlsDataLen + tunnelDataLen );

	session_peer->get_dirty_out()->reset();

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(6+data->length()); 
	//Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)

      ACE_OS::memset(msg->base(), 0, 6+data->length());

      EapResponseFast response(SET_VERSION(0x01)); 
      EapResponseFastParser  parser;
      response.set_data(data);
      
      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      ssm.SetTxMessage(msg);
      msm.Event(EvSgValid);
      //ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);

    }
  };

class AcReceiveTlvs : public EapPeerFastAction
  {
    void operator()(EapPeerFastStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAPFAST_session_t_peer *session_peer = msm.get_fast_session();
      EAP_LOG(LM_DEBUG, "PeerFast: Receive TLVs from itself.<--------------\n");

      AAAMessageBlock *tlvs = ssm.GetTxMessage();

      EAPFAST_fast_mng_peer &fast_mng_peer = msm.get_mng_peer();

      session_peer->set_dirty_in(tlvs);

      int result = fast_mng_peer.tls_connection_encrypt (session_peer);
      //if (result < 0){msm.Event(EvSgNone);}

      AAAMessageBlock *data = session_peer->get_dirty_out();
      
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(6+data->length()); 
	//Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)

      ACE_OS::memset(msg->base(), 0, 6+data->length());

      EapResponseFast response(SET_VERSION(0x01)); 
      EapResponseFastParser  parser;
      response.set_data(data);
      
      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerFast: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      ssm.SetTxMessage(msg);
      msm.Event(EvSgValid);
    }
  };


  enum {
    EvSgValid,
    EvSgInvalid,
    EvSgMoreFragments,
    EvSgNoMoreFragments,
    EvSgAlertReceive,
    EvSgAlertSend,
   
    EvSgTlvsBuild,
    EvSgSuccess,
    EvSgFailure,
    EvSgApplicationData,
  };
  enum state {
    StInitialize = 1,
    StProcessStart = 2,
    StBuildResponseClientHello = 3,
    StVerifyAuthMoreFragments = 4,
    StVerifyPeerMoreFragments = 5,
    StRecvAck = 6,
    StWaitRequestFinish = 7,
    StProcessRequestFinish = 8,
  
    StSuccess = 10,
    StFailure = 11,
//tunnel
    StWaitTunnelMessage = 12,
    StReceiveTunnelMessage = 13,
    StReceiveInnerEap = 14,
    StReceiveTlvs = 15,
  };

  AcProcessStart 		acProcessStart;
  AcBuildResponseClientHello 	acBuildResponseClientHello;
  AcVerifyAuthMoreFragments 	acVerifyAuthMoreFragments;
  AcSendAck 			acSendAck;
  AcVerifyPeerMoreFragments 	acVerifyPeerMoreFragments;
  AcSendFragment 		acSendFragment;
  AcRecvAck 			acRecvAck;
  AcProcessRequestFinish 	acProcessRequestFinish;
  AcNotifyInvalid 		acNotifyInvalid;
  AcBuildResponseAckAlert 	acBuildResponseAckAlert;
  AcBuildResponseAlert 		acBuildResponseAlert;
  AcNotifySuccess		acNotifySuccess;
  AcNotifyFailure		acNotifyFailure;
//tunnel
  AcSendTunnelMessage		acSendTunnelMessage;
  AcReceiveTunnelMessage	acReceiveTunnelMessage;
  AcReceiveInnerEap 		acReceiveInnerEap;
  AcReceiveTlvs 		acReceiveTlvs;

  EapPeerFastStateTable_S()                        //Constructor.
  {
    AddStateTableEntry(StInitialize,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessStart, acProcessStart);
    AddWildcardStateTableEntry(StInitialize, StInitialize);

    AddStateTableEntry(StProcessStart, EvSgInvalid,
		       StInitialize, acNotifyInvalid);
    AddStateTableEntry(StProcessStart, EvSgValid,
		       StBuildResponseClientHello, acBuildResponseClientHello);

    AddStateTableEntry(StBuildResponseClientHello,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StVerifyAuthMoreFragments, acVerifyAuthMoreFragments);    
    AddWildcardStateTableEntry(StBuildResponseClientHello, StBuildResponseClientHello);

    AddStateTableEntry(StVerifyAuthMoreFragments, EvSgMoreFragments,
		       StBuildResponseClientHello, acSendAck);    
    AddStateTableEntry(StVerifyAuthMoreFragments, EvSgNoMoreFragments,
		       StVerifyPeerMoreFragments, acVerifyPeerMoreFragments);
    AddStateTableEntry(StVerifyAuthMoreFragments, EvSgInvalid,
		       StInitialize, acNotifyInvalid);

    AddStateTableEntry(StVerifyAuthMoreFragments, EvSgAlertReceive,
			StFailure, acBuildResponseAckAlert);
    AddStateTableEntry(StVerifyAuthMoreFragments, EvSgAlertSend,
			StFailure, acBuildResponseAlert);

    AddStateTableEntry(StVerifyPeerMoreFragments, EvSgMoreFragments,
		       StRecvAck, acSendFragment);
    AddStateTableEntry(StVerifyPeerMoreFragments, EvSgNoMoreFragments,
		       StWaitRequestFinish, acSendFragment);
    AddStateTableEntry(StRecvAck, EapMethodStateMachine::EvSgIntegrityCheck,
		       StVerifyAuthMoreFragments, acRecvAck);

    AddStateTableEntry(StWaitRequestFinish,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessRequestFinish, acProcessRequestFinish);
    AddStateTableEntry(StWaitRequestFinish, StWaitRequestFinish, 0);

    AddStateTableEntry(StProcessRequestFinish, EvSgInvalid,
		       StInitialize, acNotifyInvalid);
    AddStateTableEntry(StProcessRequestFinish, EvSgValid,
		       StWaitTunnelMessage, acSendAck);
     
    AddStateTableEntry(StProcessRequestFinish, EvSgAlertReceive,
			StFailure, acBuildResponseAckAlert);
    AddStateTableEntry(StProcessRequestFinish, EvSgAlertSend,
			StFailure, acBuildResponseAlert);

   
//tuunel
  
    AddStateTableEntry(StWaitTunnelMessage,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StReceiveTunnelMessage, acReceiveTunnelMessage);
    AddStateTableEntry(StReceiveTunnelMessage,
		       EapMethodStateMachine::EvSgTunnelProcess,
		       StReceiveInnerEap, acReceiveInnerEap);
    AddStateTableEntry(StReceiveInnerEap, EvSgValid,
		       StWaitTunnelMessage, acSendTunnelMessage);
    AddStateTableEntry(StReceiveTunnelMessage, EvSgTlvsBuild,
		       StReceiveTlvs, acReceiveTlvs);
     AddStateTableEntry(StReceiveTlvs, EvSgValid,
		       StWaitTunnelMessage, acSendTunnelMessage);
    AddStateTableEntry(StWaitTunnelMessage, EvSgSuccess,
		       StSuccess, acNotifySuccess);
    AddStateTableEntry(StWaitTunnelMessage, EvSgFailure,
		       StFailure, acNotifyFailure);

   AddStateTableEntry(StProcessRequestFinish, EvSgApplicationData,
		       StReceiveTunnelMessage, acReceiveTunnelMessage);


    
   

    
    AddStateTableEntry(StRecvAck, StRecvAck, 0);    

    AddWildcardStateTableEntry(StFailure, StFailure);

    AddWildcardStateTableEntry(StSuccess, StSuccess);

    InitialState(StInitialize);
  } // leaf class
  ~EapPeerFastStateTable_S() {}
};

typedef ACE_Singleton<EapPeerFastStateTable_S, ACE_Recursive_Thread_Mutex>
EapPeerFastStateTable;

typedef ACE_Singleton<EapAuthFastStateTable_S, ACE_Recursive_Thread_Mutex>
EapAuthFastStateTable;

EapPeerFastStateMachine::EapPeerFastStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapPeerFastStateMachine>
  (*this, *EapPeerFastStateTable::instance(), s.Reactor(), s, (char *)"FAST(peer)")
{
  this->ssn=NULL;
  history.assign("");
}

EapAuthFastStateMachine::EapAuthFastStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapAuthFastStateMachine>
  (*this, *EapAuthFastStateTable::instance(),
   s.Reactor(), s, (char*)"FAST(authenticator)")
{
  this->ssn=NULL;
  history.assign("");  
}

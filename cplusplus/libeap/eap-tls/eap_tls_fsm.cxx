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
                          eap_tls_fsm.cxx  -  description
                             -------------------
    begin                : jue mar 18 2004
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

#include "eap_tls_fsm.hxx"

#define VALUE_TIME 1
/// Action class.
class EapPeerTlsAction : public AAA_Action<EapPeerTlsStateMachine>
{
 public:
  virtual void operator()(EapPeerTlsStateMachine&) {}
 protected:
  EapPeerTlsAction() {}
  virtual ~EapPeerTlsAction() {}
};

/// Action class.
class EapAuthTlsAction : public AAA_Action<EapAuthTlsStateMachine>
{
 public:
  virtual void operator()(EapAuthTlsStateMachine&) {}
 protected:
  EapAuthTlsAction() {}
  virtual ~EapAuthTlsAction() {}
};

/// State table used by EapAuthTlsStateMachine.
class EAP_TLS_EXPORTS EapAuthTlsStateTable_S: public AAA_StateTable<EapAuthTlsStateMachine>
{
  friend class ACE_Singleton<EapAuthTlsStateTable_S, ACE_Recursive_Thread_Mutex>;

private:
    
  class AcBuildStart : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "<----------- AuthTls: Building a Start message.\n");

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(5+1); //[Code(1) + identifier (1) + length(2) + type(1)] + flags(1)

      ACE_OS::memset(msg->base(), 0, 5+1);

      
      EapRequestTls request(SET_START(0x00)); //A request with S bit enabled.
      EapRequestTlsParser  parser;
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    }

      EAPTLS_session_t_auth *session_auth = new EAPTLS_session_t_auth(msm.get_ctx_auth());
      session_auth->session_init(false);
      msm.set_tls_session(session_auth); //Build a new session.
      //sleep(VALUE_TIME);
      //msm.History().append(msg->base() + 4, 1);
      // Set the message to the session.
      ssm.SetTxMessage(msg);
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcProcessResponseClientHello : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAPTLS_tls_mng_auth &tls_mng_auth = msm.get_mng_auth();
      AAAMessageBlock *msg = ssm.GetRxMessage();   //Msg contains complete message.rd_ptr() points data section. (from byte after Type) and wr_ptr() at the end of Message
      msg->wr_ptr(msg->end());
      EAP_LOG(LM_DEBUG, "------------->AuthTls: Processing EAP-TLS client hello.%d\n",msg->length());
      // Check the EAP-TLS response.
      EapResponseTls response((ACE_Byte)0x00);
      EapResponseTlsParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch	(...) {
        EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }

      EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
      session_auth->set_dirty_in(response.get_data());
      //sleep(VALUE_TIME);

	ACE_INT32 err;
      if((err = tls_mng_auth.tls_handshake_recv(session_auth)) == EAPTLS_tls_mng::StAlertReceive)
      {
EAP_LOG(LM_ERROR, "AuthTls: AcProcessResponseClienteHello: AlertReceive.\n");
     	msm.Event(EvSgAlertReceive);
      }
      else if (err == EAPTLS_tls_mng::StAlertSend)
      {
EAP_LOG(LM_ERROR, "AuthTls: AcProcessResponseClienteHello: AlertSend.\n");
      	msm.Event(EvSgAlertSend);
      }
      else

      msm.Event(EvSgValid);
    }
  };

  class AcVerifyAuthMoreFragments : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      ACE_Byte flags=0x00;
      ACE_UINT32 state=0;

      EAP_LOG(LM_DEBUG, "<--------------AuthTls: Verify Auth more fragments.\n");
    
      EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
      AAAMessageBlock *data = session_auth->get_dirty_out(); //Getting TLS records to be encapsulated in request

      ACE_INT32 length_to_send=data->length()+6;//+6 (Header length) =//Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+Data(n)
      ACE_INT32 length_fragment=session_auth->get_fragment_size();
      //bool length_included=session_auth->if_length_included();
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

  class AcSendFragment : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "<--------------AuthTls: Send Fragment.\n");

      EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
      AAAMessageBlock *data = session_auth->get_dirty_out(); //Getting TLS records to be encapsulated in request

      ACE_UINT32 length_to_send=session_auth->get_length_to_send();
      ACE_Byte flags=session_auth->get_flags_to_send();

      AAAMessageBlock *msg = AAAMessageBlock::Acquire(length_to_send);
      ACE_OS::memset(msg->base(), 0, length_to_send);

      EapRequestTls request(flags);
      request.set_data(data);
      EapRequestTlsParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    }
      if (TLS_LENGTH_INCLUDED(flags)) data->rd_ptr(length_to_send-6-4);
      else data->rd_ptr(length_to_send-6);      //Move pointer to next bytes (next future fragment)
      EAP_LOG(LM_DEBUG,"LENGTH NEXT FRAGMENT %d\n",data->length());
      // Set the message to the session.
      ssm.SetTxMessage(msg);
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
     }
  };
  
  class AcRecvAck : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();   //Msg contains complete message.rd_ptr() points data section. (from byte after Type) and wr_ptr() at the end of Message
      EAP_LOG(LM_DEBUG, "AuthTls: Processing ACK.\n");

      // Check the EAP-TLS response.
      EapResponseTls response((ACE_Byte)0x00);
      EapResponseTlsParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch	(...) {
        EAP_LOG(LM_ERROR, "PeerTls: Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }
      if (response.get_is_ack()) msm.Event(EvSgValid);
      else msm.Event(EvSgInvalid);
    }
  };

  class AcSendAck: public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "<----------- AuthTls: Send ACK.\n");
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(5+1); //[Code(1) + identifier (1) + length(2) + type(1)] + flags(1)
      ACE_OS::memset(msg->base(), 0, 5+1);
      EapRequestTls request(0x00); //A request with S bit enabled.
      EapRequestTlsParser  parser;
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	      EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
	      msm.Event(EvSgInvalid);
	      return;
	    }
      // Set the message to the session.
      ssm.SetTxMessage(msg);
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcVerifyPeerMoreFragments : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      msg->wr_ptr(msg->end());
      EAP_LOG(LM_DEBUG, "-------------->AutTls: Verify Peer More Fragments.\n");

      ACE_UINT32 state = 0;
      ACE_UINT32 length_fragment = 0;

      EapResponseTls response((ACE_Byte)0x00);
      EapResponseTlsParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    }

      static bool first_fragment = true;
      AAAMessageBlock *data=response.get_data();
      if (data != NULL) length_fragment=data->length();
      else length_fragment = 0;

      //if (data == NULL) EAP_LOG(LM_DEBUG,"Without data\n");
      
      if (TLS_MORE_FRAGMENTS(response.get_flags())) //This variable could be true if it is not fragmented or it is first fragmented
      {
         EAP_LOG(LM_DEBUG,"AuthTls : MORE FRAGMENTS \n");
         if (TLS_LENGTH_INCLUDED(response.get_flags()) && first_fragment) //There is no fragments
         {
           EAP_LOG(LM_DEBUG,"AuthTls : LENGTH INCLUDED \n");
            length_fragment = response.get_tls_message_length();
            if (response.get_tls_message_length() > 0)
            {
              session_auth->set_dirty_in(response.get_data());
              first_fragment = false;
              state=EvSgMoreFragments;
            }
            else state=EvSgInvalid;
         }
         else if (!TLS_LENGTH_INCLUDED(response.get_flags()) && !first_fragment)
         {
            EAP_LOG(LM_DEBUG,"AuthTls : LENGTH INCLUDED \n");
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
          EAP_LOG(LM_DEBUG,"AuthTls : NO MORE FRAGMENTS \n");
          first_fragment ? session_auth->set_dirty_in(response.get_data()) : session_auth->append_dirty_in(response.get_data());
          first_fragment = true;
          //It is a packet without fragmentation.
          state = EvSgNoMoreFragments;
      }
      msm.Event(state);
    }
  };

  class AcProcessResponseSecondWay : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAPTLS_tls_mng_auth &tls_mng_auth = msm.get_mng_auth();
      EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
      ACE_Byte flags = 0x00;
      EAP_LOG(LM_DEBUG, "-------------->AuthTls: Process response second way message.\n");

	ACE_INT32 err;
      if((err = tls_mng_auth.tls_handshake_recv(session_auth)) == EAPTLS_tls_mng::StAlertReceive)
      {
EAP_LOG(LM_ERROR, "AuthTls: AcProcessResponseSecondWay: AlertReceive.\n");
      	msm.Event(EvSgAlertReceive);
	return;
      }
      else if (err == EAPTLS_tls_mng::StAlertSend)
      {
EAP_LOG(LM_ERROR, "AuthTls: AcProcessResponseSecondWay: AlertSend.\n");
      	msm.Event(EvSgAlertSend);
	return;
      }

      AAAMessageBlock *data = session_auth->get_dirty_out();

      //Now to send final packet.
      ACE_UINT32 header_length = 6;
      if (session_auth->if_length_included())
      {
        header_length += 4;
        flags=SET_LENGTH_INCLUDED(flags);
      }
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(header_length+data->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
      ACE_OS::memset(msg->base(), 0, header_length+data->length());

      EapRequestTls request(flags);
      request.set_data(data);
      request.set_tls_message_length(data->length());

      EapRequestTlsParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    }      
      // Set the message to the session.
      ssm.SetTxMessage(msg);
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }  
  };


  class AcBuildRequestFinish : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "<-------------AuthTls: Building a request second way message.\n");

      EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
      AAAMessageBlock *data = session_auth->get_dirty_out(); //Getting TLS records to be encapsulated in request
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(6+data->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
      ACE_OS::memset(msg->base(), 0, 6+data->length());

      EapRequestTls request(0x00);

      request.set_data(data);

      EapRequestTlsParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    }
      // Update the history.
      // msm.History().append(msg->base() + 4, 4+32+40+516);
      //sleep(VALUE_TIME);

      // Set the message to the session.
      ssm.SetTxMessage(msg);

      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcProcessResponseFinish : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "------------>AuthTls: Process response second way message.\n");

      EapResponseTls response((ACE_Byte)0x00);
      EapResponseTlsParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    }
      /*EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
      session_auth->set_dirty_in(response.get_data());
      tls_mng_auth.tls_handshake_recv(session_auth);*/
      //sleep(VALUE_TIME);

	ACE_INT32 err;
      EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
      EAPTLS_tls_mng_auth &tls_mng_auth = msm.get_mng_auth();
      if((err = tls_mng_auth.tls_handshake_recv(session_auth)) == EAPTLS_tls_mng::StAlertReceive)
      {
EAP_LOG(LM_ERROR, "AuthTls: AcProcessResponseFinish: AlertReceive.\n");
      	msm.Event(EvSgAlertReceive);
      }
      else if (err == EAPTLS_tls_mng::StAlertSend)
      {
EAP_LOG(LM_ERROR, "AuthTls: AcProcessResponseFinish: AlertSend.\n");
      	msm.Event(EvSgAlertSend);
      }
      else
      // Proceed to the next step.
      msm.Event(EvSgValid);
    }
  };
  
  class AcNotifySuccess : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {

      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
      EAP_LOG(LM_DEBUG,"AuthTls: AcNotifySuccess\n");
      session_auth->session_close();
      ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnSuccess);
      msm.IsDone() = true;
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
    }
  };

  class AcNotifyInvalid : public EapAuthTlsAction
  {
    void operator()(EapAuthTlsStateMachine &msm)
    {
      EAP_LOG(LM_DEBUG,"AuthTls: AcNotifyInvalid\n");
      EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
      ssm.Notify(EapAuthSwitchStateMachine::EvSgInvalidResp);
    }
  };

	class AcBuildRequestAlert : public EapAuthTlsAction
	{
		void operator()(EapAuthTlsStateMachine &msm)
		{
			EAP_LOG(LM_DEBUG,"AuthTls: AcBuildRequestAlert\n");

			EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();

			EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
			AAAMessageBlock *data = session_auth->get_dirty_out(); //Getting TLS records to be encapsulated in request
			AAAMessageBlock *msg = AAAMessageBlock::Acquire(6+data->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
			ACE_OS::memset(msg->base(), 0, 6+data->length());

			EapRequestTls request(0x00);

			request.set_data(data);

			EapRequestTlsParser parser;
			parser.setAppData(&request);
			parser.setRawData(msg);
			try { parser.parseAppToRaw(); }
			catch (...) {
				EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
				msm.Event(EvSgInvalid);
				return;
			}
      
			// Set the message to the session.
			ssm.SetTxMessage(msg);

			// Send a "valid" signal to the switch state machine.
			ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
		}
	};

	class AcProcessResponseAckAlert : public EapAuthTlsAction
	{
		void operator()(EapAuthTlsStateMachine &msm)
		{
			EAP_LOG(LM_DEBUG,"AuthTls: AcProcessResponseAckAlert\n");

			EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
			AAAMessageBlock *msg = ssm.GetRxMessage();

			EapResponseTls response((ACE_Byte)0x00);
			EapResponseTlsParser parser;
			parser.setAppData(&response);
			parser.setRawData(msg);

			try { parser.parseRawToApp(); }
			catch (...) {
				EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
				msm.Event(EvSgInvalid);
				return;
			}

			EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
			session_auth->set_dirty_in(response.get_data());
      
			// Proceed to the next step.
			msm.Event(EvSgValid);
		}
	};

	class AcNotifyFailure : public EapAuthTlsAction
	{
		void operator()(EapAuthTlsStateMachine &msm)
		{
			EAP_LOG(LM_DEBUG,"AuthTls: AcNotifyFailure\n");
			EapAuthSwitchStateMachine &ssm = msm.AuthSwitchStateMachine();
			EAPTLS_session_t_auth *session_auth = msm.get_tls_session();
			session_auth->session_close();
			ssm.Policy().Update(EapContinuedPolicyElement::PolicyOnFailure);
			msm.IsDone() = true;
			ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
		}
	};

  
  enum {
    EvSgValid,
    EvSgInvalid,
    EvSgMoreFragments,
    EvSgNoMoreFragments,
	EvSgAlertReceive,
	EvSgAlertSend
  };
  enum state {
    StInitialize,
    StBuildStart,
    StProcessResponseClientHello,
    StVerifyAuthMoreFragments,
    StRecvAck,
    StSentAck,
    StVerifyPeerMoreFragments,
    StProcessResponseSecondWay,
    StSentRequestFinish,
    StProcessResponseFinish,
    StSuccess,
	StBuildRequestAlert,
	StProcessResponseAckAlert,
	StFailure
 };


  AcBuildStart acBuildStart;
  AcProcessResponseClientHello acProcessResponseClientHello;
  AcVerifyAuthMoreFragments acVerifyAuthMoreFragments;
  AcRecvAck acRecvAck;
  AcSendFragment acSendFragment; 
  AcSendAck acSendAck;
  AcProcessResponseSecondWay acProcessResponseSecondWay;
  AcVerifyPeerMoreFragments acVerifyPeerMoreFragments; 
  AcBuildRequestFinish acBuildRequestFinish;
  AcProcessResponseFinish acProcessResponseFinish;
  AcNotifySuccess acNotifySuccess;
  AcNotifyInvalid acNotifyInvalid;
	AcBuildRequestAlert acBuildRequestAlert;
	AcProcessResponseAckAlert acProcessResponseAckAlert;
	AcNotifyFailure acNotifyFailure;
 
  EapAuthTlsStateTable_S()                        //Constructor.
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
		       StSentRequestFinish, acProcessResponseSecondWay);
    AddStateTableEntry(StVerifyPeerMoreFragments, EvSgInvalid,
		       StInitialize, acNotifyInvalid);
       
    AddStateTableEntry(StSentRequestFinish,
           EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessResponseFinish, acProcessResponseFinish);
    AddStateTableEntry(StSentRequestFinish, StSentRequestFinish, 0);

	AddStateTableEntry(StSentRequestFinish, EvSgAlertReceive,
			StFailure, acNotifyFailure);
	AddStateTableEntry(StSentRequestFinish, EvSgAlertSend,
			StBuildRequestAlert, acBuildRequestAlert);
    
    AddStateTableEntry(StProcessResponseFinish, EvSgInvalid,
		       StInitialize, acNotifyInvalid);
    AddStateTableEntry(StProcessResponseFinish, EvSgValid,
		       StSuccess, acNotifySuccess);

 	AddStateTableEntry(StProcessResponseFinish, EvSgAlertReceive,
			StFailure, acNotifyFailure);
	AddStateTableEntry(StProcessResponseFinish, EvSgAlertSend,
			StBuildRequestAlert, acBuildRequestAlert);

	AddStateTableEntry(StFailure, StFailure, 0);
           
    AddStateTableEntry(StSuccess, StSuccess, 0);
    
    InitialState(StInitialize);
  } // leaf class
  ~EapAuthTlsStateTable_S() {}
};

///---------------------------------------------------------------EapPeerTlsState--------------------------------------------------------------------

/// State table used by EapPeerTlsStateMachine.
class EAP_TLS_EXPORTS EapPeerTlsStateTable_S :
  public AAA_StateTable<EapPeerTlsStateMachine>

{
  friend class ACE_Singleton<EapPeerTlsStateTable_S, ACE_Recursive_Thread_Mutex>;

private:

  class AcProcessStart : public EapPeerTlsAction
  {
    void operator()(EapPeerTlsStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAPTLS_tls_mng_peer &tls_mng_peer = msm.get_mng_peer();
      AAAMessageBlock *msg = ssm.GetRxMessage();   //Msg contains complete message.rd_ptr() points data section. (from byte after Type) and wr_ptr() at the end of Message
      EAP_LOG(LM_DEBUG, "PeerTls: Processing EAP-TLS Start.<-----------------\n");

      // Check the EAP-TLS response.
      EapRequestTls request((ACE_Byte)0x00);
      EapRequestTlsParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch	(...) {
        EAP_LOG(LM_ERROR, "PeerTls: Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }
      EAPTLS_session_t_peer *session_peer = new EAPTLS_session_t_peer(msm.get_ctx_peer());
      session_peer->session_init(false);
      msm.set_tls_session(session_peer); //Build a new session.
      session_peer->set_dirty_in(request.get_data());
      tls_mng_peer.tls_handshake_recv(session_peer);
      msm.Event(EvSgValid);
    }
  };

  class AcBuildResponseClientHello : public EapPeerTlsAction
  {
    void operator()(EapPeerTlsStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "PeerTls: Building a Client Hello Message.--------------->\n");

      EAPTLS_session_t_peer *session_peer = msm.get_tls_session();
      AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting TLS records to be encapsulated in request
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(6+data->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
      EAP_LOG(LM_DEBUG, "PeeTls: data length %d\n",data->length());

      ACE_OS::memset(msg->base(), 0, 6+data->length());

      EapResponseTls response(0x00); 
      EapResponseTlsParser  parser;
      response.set_data(data);
      
      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerTls: Parse error.\n");
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

  class AcVerifyAuthMoreFragments : public EapPeerTlsAction
  {
    void operator()(EapPeerTlsStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAPTLS_session_t_peer *session_peer = msm.get_tls_session();
      EAPTLS_tls_mng_peer &tls_mng_peer = msm.get_mng_peer();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      //msg->wr_ptr(msg->end());
      EAP_LOG(LM_DEBUG, "-------------->PeerTls: Verify Auth More Fragments.\n");

      ACE_UINT32 state = 0;
      EapResponseTls response((ACE_Byte)0x00);
      EapResponseTlsParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerTls: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;                                          
	    }

      static bool first_fragment = true;

      if (TLS_MORE_FRAGMENTS(response.get_flags())) //This variable could be true if it is not fragmented or it is first fragmented
      {
        EAP_LOG(LM_DEBUG,"PeerTls : AUTH MORE_FRAGMENTS\n");
         if (TLS_LENGTH_INCLUDED(response.get_flags()) && first_fragment) //There is no fragments
         {
            if (response.get_tls_message_length() > 0)
            {
              session_peer->set_dirty_in(response.get_data());
              first_fragment = false;
              state=EvSgMoreFragments;
            }
            else state=EvSgInvalid;
         }
         else if (!TLS_LENGTH_INCLUDED(response.get_flags()) && !first_fragment)
         {
            if (response.get_data()->length() > 0)
            {
              state=EvSgMoreFragments;
              session_peer->append_dirty_in(response.get_data());
            }
            else state=EvSgInvalid;
         }
         else state = EvSgInvalid;
      }
      else
      {
          first_fragment ? session_peer->set_dirty_in(response.get_data()) : session_peer->append_dirty_in(response.get_data());
          first_fragment = true;

	ACE_INT32 err;
      if((err = tls_mng_peer.tls_handshake_recv(session_peer)) == EAPTLS_tls_mng::StAlertReceive)
      {
EAP_LOG(LM_ERROR, "PeerTls: AcProcessRequestSecondWay: AlertReceive.\n");
      	msm.Event(EvSgAlertReceive);
	return;
      }
      else if (err == EAPTLS_tls_mng::StAlertSend)
      {
EAP_LOG(LM_ERROR, "PeerTls: AcProcessRequestFinish: AlertSend.\n");
      	msm.Event(EvSgAlertSend);
	return;
      }
      else

          //It is a packet without fragmentation.
          state = EvSgNoMoreFragments;
      }
      msm.Event(state);
    }
  };

  class AcSendAck: public EapPeerTlsAction
  {
    void operator()(EapPeerTlsStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "<----------- PeerTls: Send ACK.\n");
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(5+1); //[Code(1) + identifier (1) + length(2) + type(1)] + flags(1)
      ACE_OS::memset(msg->base(), 0, 5+1);
      EapRequestTls request(0x00); //A request with S bit enabled.
      EapRequestTlsParser  parser;
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	      EAP_LOG(LM_ERROR, "AuthTls: Parse error.\n");
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

  class AcVerifyPeerMoreFragments : public EapPeerTlsAction
  {
    void operator()(EapPeerTlsStateMachine &msm)
    {
      ACE_Byte flags=0x00;
      ACE_UINT32 state=0;

      EAP_LOG(LM_DEBUG, "<--------------PeerTls: Verify Peer More Fragments.\n");

      EAPTLS_session_t_peer *session_peer = msm.get_tls_session();
      AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting TLS records to be encapsulated in request

      ACE_INT32 length_to_send=data->length()+6; //+6 (Header length) =//Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+Data(n)
      ACE_INT32 length_fragment=session_peer->get_fragment_size();
      static bool first_fragment=true;

      if ((length_fragment != 0) && (length_fragment < length_to_send))
      {
        EAP_LOG(LM_DEBUG,"PeerTls : MORE FRAGMENTS \n");
        length_to_send = length_fragment; //We need to send more fragments
        flags = SET_MORE_FRAGMENTS(flags);
        if (first_fragment)
        {
          EAP_LOG(LM_DEBUG,"PeerTls : LENGTH INCLUDED\n");
          flags = SET_LENGTH_INCLUDED(flags);
          first_fragment=false;
        }
        state = EvSgMoreFragments;
      }
      else {
        EAP_LOG(LM_DEBUG,"PeerTls : NO MORE FRAGMENTS \n");
        state = EvSgNoMoreFragments;
        first_fragment=true; //Restore first fragment value;
      }
      session_peer->set_length_to_send(length_to_send); //Length will be sent by next state.
      session_peer->set_flags_to_send(flags);
      msm.Event(state);
    }
  };

  class AcSendFragment : public EapPeerTlsAction
  {
    void operator()(EapPeerTlsStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "<--------------PeerTls: Send Fragment.\n");

      EAPTLS_session_t_peer *session_peer = msm.get_tls_session();
      AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting TLS records to be encapsulated in request

      ACE_INT32 length_to_send=session_peer->get_length_to_send();
      ACE_Byte flags=session_peer->get_flags_to_send();
       EAP_LOG(LM_ERROR, "PeerTls: LENGTH TO SEND %d.\n",length_to_send);
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(length_to_send);
      ACE_OS::memset(msg->base(), 0, length_to_send);

      EapRequestTls request(flags);
      request.set_data(data);
      EapRequestTlsParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);
      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerTls: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	    }
      if (TLS_LENGTH_INCLUDED(flags)) data->rd_ptr(length_to_send-6-4);
      else data->rd_ptr(length_to_send-6);      //Move pointer to next bytes (next future fragment)
      // Set the message to the session.
      ssm.SetTxMessage(msg);
      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::CONT;
      // Send a "valid" signal to the switch state machine.
      ssm.Notify(EapAuthSwitchStateMachine::EvSgValidResp);
     }
  };

  class AcRecvAck : public EapPeerTlsAction
  {
    void operator()(EapPeerTlsStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      AAAMessageBlock *msg = ssm.GetRxMessage();   //Msg contains complete message.rd_ptr() points data section. (from byte after Type) and wr_ptr() at the end of Message
      EAP_LOG(LM_DEBUG, "PeerTls: Processing ACK.\n");

      // Check the EAP-TLS response.
      EapResponseTls response((ACE_Byte)0x00);
      EapResponseTlsParser parser;
      parser.setAppData(&response);
      parser.setRawData(msg);
      try { parser.parseRawToApp(); }
      catch	(...) {
        EAP_LOG(LM_ERROR, "PeerTls: Parse error.\n");
        msm.Event(EvSgInvalid);
        return;
      }
      if (response.get_is_ack()) msm.Event(EvSgValid);
      else msm.Event(EvSgInvalid);
    }
  };

  class AcProcessRequestFinish : public EapPeerTlsAction
  {
    void operator()(EapPeerTlsStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAPTLS_tls_mng_peer &tls_mng_peer = msm.get_mng_peer();
      AAAMessageBlock *msg = ssm.GetRxMessage();
      EAP_LOG(LM_DEBUG, "PeerTls: Process Request Finish.\n");

      EapRequestTls request((ACE_Byte)0x00);
      EapRequestTlsParser parser;
      parser.setAppData(&request);
      parser.setRawData(msg);

      try { parser.parseRawToApp(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerTls: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }

      EAPTLS_session_t_peer *session_peer = msm.get_tls_session();
      session_peer->set_dirty_in(request.get_data());

	ACE_INT32 err;
      if((err = tls_mng_peer.tls_handshake_recv(session_peer)) == EAPTLS_tls_mng::StAlertReceive)
      {
	msm.Event(EvSgAlertReceive);
      }
      else if (err == EAPTLS_tls_mng::StAlertSend)
      {
	msm.Event(EvSgAlertSend);
      }
      else
            
      // Proceed to the next step.
      msm.Event(EvSgValid);
    }
  };


  class AcNotifySuccess : public EapPeerTlsAction
  {
    void operator()(EapPeerTlsStateMachine &msm)
    {
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      EAP_LOG(LM_DEBUG, "PeerTls: Send Ack.\n");

      EAPTLS_session_t_peer *session_peer = msm.get_tls_session();
      AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting TLS records to be encapsulated in request
      AAAMessageBlock *msg = AAAMessageBlock::Acquire(6); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
      ACE_OS::memset(msg->base(), 0, 6);

      EapResponseTls response(0x00);
      EapResponseTlsParser  parser;
      response.set_data(data);

      parser.setAppData(&response);
      parser.setRawData(msg);

      try { parser.parseAppToRaw(); }
      catch (...) {
	    EAP_LOG(LM_ERROR, "PeerTls: Parse error.\n");
	    msm.Event(EvSgInvalid);
	    return;
	  }
      session_peer->session_close(); //This session is finished.
      // Set the message to the session.
      ssm.SetTxMessage(msg);
      ssm.Decision() = EapPeerSwitchStateMachine::UNCOND_SUCC;
      EAP_LOG(LM_DEBUG, "PeerTls: ACK prepared.\n");

      // Update external method state.
      ssm.MethodState() = EapPeerSwitchStateMachine::DONE;
      ssm.Event(EapPeerSwitchStateMachine::EvSgValidReq);
    }
  };

  class AcNotifyInvalid : public EapPeerTlsAction
  {
    void operator()(EapPeerTlsStateMachine &msm)
    {
      EAP_LOG(LM_DEBUG,"PeerTls: AcNotifyInvalid\n");
      EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();
      ssm.Event(EapPeerSwitchStateMachine::EvSgInvalidReq);
    }
  };

	class AcBuildResponseAckAlert : public EapPeerTlsAction
	{
		void operator()(EapPeerTlsStateMachine &msm)
		{
			EAP_LOG(LM_DEBUG,"PeerTls: AcBuildResponseAckAlert\n");
			EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

			EAPTLS_session_t_peer *session_peer = msm.get_tls_session();
			AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting TLS records to be encapsulated in request
			AAAMessageBlock *msg = AAAMessageBlock::Acquire(6 + data->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
			ACE_OS::memset(msg->base(), 0, 6 + data->length());

			EapResponseTls response(0x00);
			EapResponseTlsParser parser;
			response.set_data(data);

			parser.setAppData(&response);
			parser.setRawData(msg);

			try { parser.parseAppToRaw(); }
			catch (...) {
				EAP_LOG(LM_ERROR, "PeerTls: Parse error.\n");
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

	class AcBuildResponseAlert : public EapPeerTlsAction
	{
		void operator()(EapPeerTlsStateMachine &msm)
		{
			EAP_LOG(LM_DEBUG,"PeerTls: AcBuildResponseAlert\n");
			EapPeerSwitchStateMachine &ssm = msm.PeerSwitchStateMachine();

			EAPTLS_session_t_peer *session_peer = msm.get_tls_session();
			AAAMessageBlock *data = session_peer->get_dirty_out(); //Getting TLS records to be encapsulated in request
			AAAMessageBlock *msg = AAAMessageBlock::Acquire(6 + data->length()); //Code(1)+Identifier(1)+Length(2)+Type(1)+Flags(1)+ Data(n)
			ACE_OS::memset(msg->base(), 0, 6 + data->length());

			EapResponseTls response(0x00);
			EapResponseTlsParser parser;
			response.set_data(data);

			parser.setAppData(&response);
			parser.setRawData(msg);

			try { parser.parseAppToRaw(); }
			catch (...) {
				EAP_LOG(LM_ERROR, "PeerTls: Parse error.\n");
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

  enum {
    EvSgValid,
    EvSgInvalid,
    EvSgMoreFragments,
    EvSgNoMoreFragments,
	EvSgAlertReceive,
	EvSgAlertSend
  };
  enum state {
    StInitialize,
    StProcessStart,
    StBuildResponseClientHello,
    StVerifyAuthMoreFragments,
    StVerifyPeerMoreFragments,
    StRecvAck,
    StWaitRequestFinish,
    StProcessRequestFinish,
    StSuccess,
	StFailure
  };

  AcProcessStart acProcessStart;
  AcBuildResponseClientHello acBuildResponseClientHello;
  AcVerifyAuthMoreFragments acVerifyAuthMoreFragments;
  AcSendAck acSendAck;
  AcVerifyPeerMoreFragments acVerifyPeerMoreFragments;
  AcSendFragment acSendFragment;
  AcRecvAck acRecvAck;
  AcProcessRequestFinish acProcessRequestFinish;
  AcNotifySuccess acNotifySuccess;
  AcNotifyInvalid acNotifyInvalid;
	AcBuildResponseAckAlert acBuildResponseAckAlert;
	AcBuildResponseAlert acBuildResponseAlert;

  

  EapPeerTlsStateTable_S()                        //Constructor.
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
    AddStateTableEntry(StVerifyAuthMoreFragments, EvSgValid,
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

    AddStateTableEntry(StRecvAck,
                       EapMethodStateMachine::EvSgIntegrityCheck,
		                   StVerifyAuthMoreFragments, acRecvAck);
    AddStateTableEntry(StRecvAck, StRecvAck, 0);
    
  
    AddStateTableEntry(StWaitRequestFinish,
		       EapMethodStateMachine::EvSgIntegrityCheck,
		       StProcessRequestFinish, acProcessRequestFinish);
    AddStateTableEntry(StWaitRequestFinish, StWaitRequestFinish, 0);

    AddStateTableEntry(StProcessRequestFinish, EvSgInvalid,
		       StInitialize, acNotifyInvalid);
    AddStateTableEntry(StProcessRequestFinish, EvSgValid,
		       StSuccess, acNotifySuccess);
           
	AddStateTableEntry(StProcessRequestFinish, EvSgAlertReceive,
			StFailure, acBuildResponseAckAlert);
	AddStateTableEntry(StProcessRequestFinish, EvSgAlertSend,
			StFailure, acBuildResponseAlert);

	AddWildcardStateTableEntry(StFailure, StFailure);

    AddWildcardStateTableEntry(StSuccess, StSuccess);

    InitialState(StInitialize);
  } // leaf class
  ~EapPeerTlsStateTable_S() {}
};

typedef ACE_Singleton<EapPeerTlsStateTable_S, ACE_Recursive_Thread_Mutex>
EapPeerTlsStateTable;

typedef ACE_Singleton<EapAuthTlsStateTable_S, ACE_Recursive_Thread_Mutex>
EapAuthTlsStateTable;

EapPeerTlsStateMachine::EapPeerTlsStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapPeerTlsStateMachine>
  (*this, *EapPeerTlsStateTable::instance(), s.Reactor(), s, "TLS(peer)")
{
  this->ssn=NULL;
}

EapAuthTlsStateMachine::EapAuthTlsStateMachine
(EapSwitchStateMachine &s)
  : EapMethodStateMachine(s),
    EapStateMachine<EapAuthTlsStateMachine>
  (*this, *EapAuthTlsStateTable::instance(),
   s.Reactor(), s, "TLS(authenticator)")
{
  this->ssn=NULL;
  
}

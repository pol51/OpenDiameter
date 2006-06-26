/* BEGIN_COPYRIGHT
Open Diameter: Open-source software for the Diameter protocol

Copyright (C) 2004 Open Diameter Project

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
USA.

In addition, when you copy and redistribute some or the entire part of
the source code of this software with or without modification, you
MUST include this copyright notice in each copy.

If you make any changes that are appeared to be useful, please send
sources that include the changed part to
diameter-developers@lists.sourceforge.net so that we can reflect your
changes to one unified version of this software.
END_COPYRIGHT */
//diameter_mip4_aaas_client_fsm.cxx:MIP "AAA server" Client sess. state machine
// Written by Miriam Tauil

#include <ace/Singleton.h>
#include <ace/Atomic_Op_T.h>
#include "resultcodes.h"
#include "diameter_mip4_parser.hxx"
#include "diameter_mip4_aaas_client_fsm.hxx"
#include "diameter_mip4_aaas_client_session.hxx"

/// This state machine is very simple since most of the data processing
/// is being done in the AAAS Server Session, that activates this AAAS Client
/// session. 
/// The server session prepares the content of the HAR message to be send,
/// This client session mainly sends the HAR receives the HAA, checks the
/// result code and triggers the relevant event in the server session. 
/// The server session use the HAA  content + the trigger event in 
/// order to send an AMA. 

class DiameterMip4AaaSClientAction 
  : public AAA_Action<DiameterMip4AaaSClientStateMachine>
{
  virtual void operator()(DiameterMip4AaaSClientStateMachine &sm)=0;
 protected:
  DiameterMip4AaaSClientAction() {}
  ~DiameterMip4AaaSClientAction() {}
};

/// State table used by DiameterMip4AaaSClientStateMachine.
class DiameterMip4AaaSClientStateTable_S 
  : public AAA_StateTable<DiameterMip4AaaSClientStateMachine>
{
  friend class 
  ACE_Singleton<DiameterMip4AaaSClientStateTable_S, ACE_Recursive_Thread_Mutex>;
 private:

  class AcSendHAR : public DiameterMip4AaaSClientAction 
  {
    void operator()(DiameterMip4AaaSClientStateMachine& sm) 
    { 
      ACE_DEBUG((LM_DEBUG, "[%N] sending HAR.\n"));

      sm.SendHAR(); 
    }
  };

  class AcCheckHAA_ResultCode : public DiameterMip4AaaSClientAction 
  {
    void operator()(DiameterMip4AaaSClientStateMachine& sm)
    {
 
      HAA_Data &haaData = sm.Session().HAA();

      AAAResultCode resultCode = haaData.ResultCode();

      switch (resultCode)
	{
	case AAA_SUCCESS :
	  AAA_LOG(LM_DEBUG, "[%N] DIAMETER_SUCCESS received.\n");
	  sm.Notify(DiameterMip4AaaSClientStateMachine::EvSgSuccess);
	  break;

	default:
	  AAA_LOG(LM_DEBUG, "[%N] Error was received.\n");
	  AAA_LOG(LM_DEBUG, "[%N]   Result-Code=%d.\n", resultCode);
	  sm.Notify(DiameterMip4AaaSClientStateMachine::EvSgFailure);
	  break;
	}
    }
  };


  class AcAccessAccept : public DiameterMip4AaaSClientAction 
  {
    void operator()(DiameterMip4AaaSClientStateMachine& sm)
    {
       sm.Session().serverSession.ServerSessionNotify( 
		    DiameterMip4AaaSServerStateMachine::EvSgValidHAA);
       sm.Session().Update(AAASession::EVENT_AUTH_SUCCESS);  
    }
  };

  class AcAccessReject : public DiameterMip4AaaSClientAction 
  {
    void operator()(DiameterMip4AaaSClientStateMachine& sm)
    {

      HAA_Data& haaData = sm.Session().HAA();  
      
      std::string str;
       
      if (haaData.ErrorMessage.IsSet())
      {
	str = haaData.ErrorMessage();
	AAA_LOG(LM_DEBUG, "[%N] Error Message: %s \n", str.data());
      }
      if (haaData.ErrorReportingHost.IsSet())
      {
	str = haaData.ErrorReportingHost();
	AAA_LOG(LM_DEBUG, "[%N] Error Message: %s \n", str.data());
      }

       sm.Session().serverSession.ServerSessionNotify( 
		    DiameterMip4AaaSServerStateMachine::EvSgInvalidHAA);
      
       sm.Session().Update(AAASession::EVENT_AUTH_FAILED);

    }
  };

  class AcDisconnect : public DiameterMip4AaaSClientAction 
  {
    void operator()(DiameterMip4AaaSClientStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Disconnect issued.\n");
      //sm.SignalDisconnect();
    }
  };

  class AcHaaNotReceived : public DiameterMip4AaaSClientAction 
  {
    void operator()(DiameterMip4AaaSClientStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] HAA not received.\n");
      sm.Session().serverSession.ServerSessionNotify( 
		    DiameterMip4AaaSServerStateMachine::EvSgHaaNotReceived);
    }
  };

  class AcReset : public DiameterMip4AaaSClientAction 
  {
    void operator()(DiameterMip4AaaSClientStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Reset session object after session termination.\n");
      //sm.Reset();
    }
  };

  enum state {
    StInitialize,
    StWaitHAA,
    StCheckResultCode,
    StAccepted,
    StRejected,
    StTerminated
  };
  /**
  enum {
    EvSendHAR,
    EvRxHAA,
    EvSgSuccess,        // HAA result
    EvSgLimitedSuccess, //  
    EvSgFailure,        // HAA result 
    EvSgSessionTimeout,
    EvSgAuthLifetimeTimeout,
    EvSgDisconnect,
    EvSgReset           //Reset session to reuse object 
  };
  ***/

  AcSendHAR acSendHAR;
  AcCheckHAA_ResultCode acCheckHAA_ResultCode;
  AcAccessAccept acAccessAccept;
  AcAccessReject acAccessReject;
  AcHaaNotReceived acHaaNotReceived;
  AcDisconnect acDisconnect;
  AcReset acReset;

  // Defined as a leaf class
  DiameterMip4AaaSClientStateTable_S() 
  {

    AddStateTableEntry(StInitialize, 
		       DiameterMip4AaaSClientStateMachine::EvSendHAR,
		       StWaitHAA, acSendHAR);
    AddStateTableEntry(StInitialize, 
		       DiameterMip4AaaSClientStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StInitialize, StInitialize);

    AddStateTableEntry(StWaitHAA,
		       DiameterMip4AaaSClientStateMachine::EvRxHAA,
		       StCheckResultCode, acCheckHAA_ResultCode);

    AddStateTableEntry(StWaitHAA,
		       DiameterMip4AaaSClientStateMachine::EvSgDisconnect,
		       StTerminated, acHaaNotReceived);
    AddStateTableEntry(StWaitHAA,
		       DiameterMip4AaaSClientStateMachine::EvSgSessionTimeout,
		       StTerminated, acHaaNotReceived);
    AddStateTableEntry(StWaitHAA,
	       DiameterMip4AaaSClientStateMachine::EvSgAuthLifetimeTimeout,
	       StTerminated, acHaaNotReceived);
    AddStateTableEntry(StWaitHAA,
	       DiameterMip4AaaSClientStateMachine::EvSgAuthGracePeriodTimeout,
	       StTerminated, acHaaNotReceived);

    AddWildcardStateTableEntry(StWaitHAA, StTerminated);

    AddStateTableEntry(StCheckResultCode, 
		       DiameterMip4AaaSClientStateMachine::EvSgSuccess,
		       StAccepted, acAccessAccept);

    AddStateTableEntry(StCheckResultCode, 
		       DiameterMip4AaaSClientStateMachine::EvSgFailure,
		       StRejected, acAccessReject);

    AddStateTableEntry(StAccepted, 
		       DiameterMip4AaaSClientStateMachine::EvSgDisconnect,
		       StTerminated, acDisconnect);
    AddStateTableEntry(StAccepted, 
		       DiameterMip4AaaSClientStateMachine::EvSgSessionTimeout,
		       StTerminated, acDisconnect);
 
    AddStateTableEntry(StAccepted, 
	       DiameterMip4AaaSClientStateMachine::EvSgAuthLifetimeTimeout,
	       StTerminated, acDisconnect);

    AddStateTableEntry(StAccepted, 
	       DiameterMip4AaaSClientStateMachine::EvSgAuthGracePeriodTimeout,
	       StTerminated, acDisconnect);

    AddStateTableEntry(StTerminated, 
		       DiameterMip4AaaSClientStateMachine::EvSgReset,
		       StInitialize, acReset);
 
    //ADD:     AddStateTableEntry(any state , EvSgTimeout, StTerminated

    AddWildcardStateTableEntry(StAccepted, StAccepted);
			
    AddWildcardStateTableEntry(StRejected, StTerminated);

    InitialState(StInitialize);

  }

  ~DiameterMip4AaaSClientStateTable_S() {}
};


typedef 
ACE_Singleton<DiameterMip4AaaSClientStateTable_S, ACE_Recursive_Thread_Mutex> 
DiameterMip4AaaSClientStateTable;


DiameterMip4AaaSClientStateMachine::DiameterMip4AaaSClientStateMachine( 
	 DiameterMip4AaaSClientSession &s,
	 DiameterJobHandle &h) :
  AAA_StateMachine<DiameterMip4AaaSClientStateMachine>(*this, *DiameterMip4AaaSClientStateTable::instance(), "DIAMETER_MIP4_AAAS_CLIENT"), session(s), handle(h) {}


void DiameterMip4AaaSClientStateMachine::SendHAR()
{
  DiameterMip4AaaSClientSession &session = Session();
  
  HAR_Data &harData = session.serverSession.HAR();

#ifdef PRINT_MSG_CONTENT
  printf("HAR:\nAuthorization Lifetime: %d\nMip-Reg-Request: %s\nUser Name: %s\nDestination Realm: %s\nDestination Host: %s\n\n", 
	 harData.AuthorizationLifetime(), 
	 harData.MipRegRequest().data(), 
	 harData.UserName().data(),
	 harData.DestinationRealm().data(),
	 harData.DestinationHost().data() );
#endif

  AAAMessage msg;

  HAR_Parser parser;

  parser.setAppData(&harData);   
  parser.setRawData(&msg);

  try {
    parser.parseAppToRaw();
  }
  catch (DiameterParserError) {
    AAA_LOG(LM_ERROR, "[%N] Parsing error.\n");
    return;
  }

  AAAMessageControl msgControl( &session );    
  if (msgControl.Send(msg) != AAA_ERR_SUCCESS) {
    AAA_LOG(LM_ERROR, "Failed sending message.\n");
  }
  else {
    AAA_LOG(LM_DEBUG, "Sent HAR Message.\n");
  }
}




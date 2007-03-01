/* BEGIN_COPYRIGHT
Open Diameter: Open-source software for the Diameter protocol

Copyright (C) 2007 Open Diameter Project

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
// diameter_mip4_ha_client_fsm.cxx:  MIP HA Client session handling
// Written by Miriam Tauil

#include <ace/Singleton.h>
#include <ace/Atomic_Op_T.h>
#include "diameter_mip4_ha_client_session.hxx"
#include "diameter_mip4_ha_client_fsm.hxx"
#include "diameter_mip4_parser.hxx"
#include "resultcodes.h"


class DiameterMip4HaClientAction 
  : public AAA_Action<DiameterMip4HaClientStateMachine>
{
  virtual void operator()(DiameterMip4HaClientStateMachine&)=0;
 protected:
  DiameterMip4HaClientAction() {}
  ~DiameterMip4HaClientAction() {}
};

/// State table used by DiameterMip4HaClientStateMachine.
class DiameterMip4HaClientStateTable_S 
  : public AAA_StateTable<DiameterMip4HaClientStateMachine>
{
  friend class 
  ACE_Singleton<DiameterMip4HaClientStateTable_S, ACE_Recursive_Thread_Mutex>;

 private:

  class AcSendAMR : public DiameterMip4HaClientAction 
  {
    void operator()(DiameterMip4HaClientStateMachine& sm) 
    { 
      AAA_LOG((LM_DEBUG, "[%N] sending AMR.\n"));

      AMR_Data& amr = sm.AMR();  
      sm.SetUserName( amr.UserName);
        
      // Generate authorization AVPs.
      sm.SetDestinationRealm( amr.DestinationRealm);
      if (!amr.DestinationRealm.IsSet())
	{
	  AAA_LOG((LM_ERROR, "Failed to set destination realm.\n"));
	  sm.Event( DiameterMip4HaClientStateMachine::EvSgDisconnect);
	  return;
	}

      // amr.MipRegRequest will be set by the fn RxMipregReq()
      sm.SetMipMnAaaAuth(amr.MipMnAaaAuth);

      sm.SetDestinationHost(amr.DestinationHost);

      sm.SetMipMobileNodeAddress( amr.MipMobileNodeAddress);

      sm.SetMipHomeAgentAddress( amr.MipHomeAgentAddress);

      sm.SetMipFeatureVector( amr.MipFeatureVector);
      // implement later -dynamic ha
      //sm.SetMipOriginatingForeignAaa( amr.MipOriginatingForeignAaa);

      // Set AuthLifetime by the client creates a library WARNING. 
      //sm.SetAuthorizationLifetime( amr.AuthorizationLifetime);

      //Not Needed - parameter is set in the configuration file
      // In case the user wants to implement it differently,
      // these are the 2 options:
      //amr.AuthSessionState.Set( 1); //something representing before auth
      //sm.SetAuthSessionState (amr.AuthSessionState);
      
      sm.SetMipHomeAgentHost( amr.MipHomeAgentHost);
      sm.SendAMR(); 
    }
  };

  class AcCheckAMA_ResultCode : public DiameterMip4HaClientAction 
  {
    void operator()(DiameterMip4HaClientStateMachine& sm)
    {
      AMA_Data &amaData = sm.AMA();

      DiameterResultCode resultCode = amaData.ResultCode();

      switch (resultCode)
	{
	case AAA_SUCCESS :
	  AAA_LOG((LM_DEBUG, "[%N] DIAMETER_SUCCESS received.\n"));
	  sm.Event(DiameterMip4HaClientStateTable_S::EvSgSuccess);
	  break;

	default:
	  AAA_LOG((LM_DEBUG, "[%N] Error was received.\n"));
	  AAA_LOG((LM_DEBUG, "[%N]   Result-Code=%d.\n", resultCode));
	  sm.Event(DiameterMip4HaClientStateTable_S::EvSgFailure);
	  break;
	}
    }
  };


  class AcAccessAccept : public DiameterMip4HaClientAction 
  {
    void operator()(DiameterMip4HaClientStateMachine& sm)
    {
 
      AMA_Data& ama = sm.AMA(); //session.amaData;

       sm.Session().Update(AAASession::EVENT_AUTH_SUCCESS);  

      if (ama.AuthorizationLifetime.IsSet())
	sm.EnforceAuthorizationLifetime(ama.AuthorizationLifetime());
  
      if (ama.AuthSessionState.IsSet())
	sm.EnforceAuthSessionState(ama.AuthSessionState());

      if (ama.ReAuthRequestType.IsSet())
	sm.EnforceReAuthRequestType(ama.ReAuthRequestType());

      if (ama.MipMnToFaMsa.IsSet())
	sm.EnforceMipMnToFaMsa ( ama.MipMnToFaMsa());

      if (ama.MipMnToHaMsa.IsSet())
	sm.EnforceMipMnToHaMsa ( ama.MipMnToHaMsa());

      if (ama.MipFaToMnMsa.IsSet())
	sm.EnforceMipFaToMnMsa ( ama.MipFaToMnMsa());

      if (ama.MipFaToHaMsa.IsSet())
	sm.EnforceMipFaToHaMsa ( ama.MipFaToHaMsa());

      if (ama.MipHaToMnMsa.IsSet())
	sm.EnforceMipHaToMnMsa ( ama.MipHaToMnMsa());

      if (ama.MipMsaLifetime.IsSet())
	sm.EnforceMipMsaLifetime ( ama.MipMsaLifetime());

      sm.SendMipRegReply( ama.ResultCode() ) ;

#ifdef PRINT_MSG_CONTENT 
      printf("AMA:\n ResultCode:%d\n\n",
 	     ama.ResultCode());
#endif
     
    }
  };

  class AcAccessReject : public DiameterMip4HaClientAction 
  {
    void operator()(DiameterMip4HaClientStateMachine& sm)
    {

      AMA_Data& amaData = sm.AMA();  
      
      if (amaData.ErrorMessage.IsSet())
      {
        AAA_LOG((LM_DEBUG, "[%N] Error Message: %s .\n", 
		amaData.ErrorMessage().data())); 
	sm.EnforceErrorMessage( amaData.ErrorMessage().data() );
      }
      if (amaData.ErrorReportingHost.IsSet())
      {
	AAA_LOG((LM_DEBUG, "[%N] Error Reporting Host: %s .\n", 
		amaData.ErrorReportingHost().data()));
      }

      sm.Session().Update(AAASession::EVENT_AUTH_FAILED);  

      sm.SendMipRegReply( amaData.ResultCode() ) ;
    }

  };

class AcSendMipReply : public DiameterMip4HaClientAction  
{
  void operator()(DiameterMip4HaClientStateMachine& sm)
  {
    // 4000 Diameter category
    diameter_unsigned32_t disconect_before_ama_response = 4000;

    sm.SendMipRegReply( disconect_before_ama_response );
				     
  }
};

  class AcDisconnect : public DiameterMip4HaClientAction 
  {
    void operator()(DiameterMip4HaClientStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "[%N] Disconnect issued.\n"));
      //sm.SignalDisconnect();
    }
  };

  class AcReset : public DiameterMip4HaClientAction 
  {
    void operator()(DiameterMip4HaClientStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "[%N] Reset session object after session termination.\n"));
      sm.Reset();
    }
  };

  enum state {
    StInitialize,
    StWaitAMA,
    StCheckResultCode,
    StAccepted,
    StRejected,
    StTerminated
  };

  enum {
    EvRxMipRegReq,
    EvRxAMA,
    EvSgSuccess,        // AMA result
    EvSgLimitedSuccess, //  
    EvSgFailure,        // AMA result 
    EvSgSessionTimeout,
    EvSgAuthLifetimeTimeout,
    EvSgReset           //Reset session to reuse object 
  };

  AcSendAMR acSendAMR;
  AcCheckAMA_ResultCode acCheckAMA_ResultCode;
  AcAccessAccept acAccessAccept;
  AcAccessReject acAccessReject;
  AcDisconnect acDisconnect;
  AcSendMipReply acSendMipReply;
  AcReset acReset;

  // Defined as a leaf class
  DiameterMip4HaClientStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       DiameterMip4HaClientStateMachine::EvRxMipRegReq,
		       StWaitAMA, acSendAMR);
    AddStateTableEntry(StInitialize, 
		       DiameterMip4HaClientStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StInitialize, StInitialize);

    AddStateTableEntry(StWaitAMA,
		       DiameterMip4HaClientStateMachine::EvRxAMA,
		       StCheckResultCode, acCheckAMA_ResultCode);
    AddStateTableEntry(StWaitAMA,
		       DiameterMip4HaClientStateMachine::EvSgDisconnect,
		       StTerminated, acSendMipReply);

    AddWildcardStateTableEntry(StWaitAMA, StTerminated);

    AddStateTableEntry(StCheckResultCode, EvSgSuccess,
		       StAccepted, acAccessAccept);

    AddStateTableEntry(StCheckResultCode, EvSgFailure,
		       StRejected, acAccessReject);

    AddStateTableEntry(StAccepted, 
		       DiameterMip4HaClientStateMachine::EvSgDisconnect,
		       StTerminated, acDisconnect);

    AddStateTableEntry
      (StAccepted, 
       DiameterMip4HaClientStateMachine::EvSgSessionTimeout,
       StTerminated, acDisconnect);
 
    AddStateTableEntry(StAccepted, 
	       DiameterMip4HaClientStateMachine::EvSgAuthLifetimeTimeout,
	       StTerminated, acDisconnect);

    AddStateTableEntry(StAccepted, 
	       DiameterMip4HaClientStateMachine::EvSgAuthGracePeriodTimeout,
	       StTerminated, acDisconnect);

    AddStateTableEntry(StTerminated, EvSgReset, StInitialize, acReset);
 
    //ADD:     AddStateTableEntry(any state , EvSgTimeout, StTerminated
    // WHAT should be the action and next state in the entry above????
    //		       StInitialize, acReauthenticate);
    AddWildcardStateTableEntry(StAccepted, StAccepted);
			
    AddWildcardStateTableEntry(StRejected, StTerminated);

    InitialState(StInitialize);
  }
  ~DiameterMip4HaClientStateTable_S() {}
};

typedef 
ACE_Singleton<DiameterMip4HaClientStateTable_S, ACE_Recursive_Thread_Mutex> 
DiameterMip4HaClientStateTable;

DiameterMip4HaClientStateMachine::DiameterMip4HaClientStateMachine
(AAAClientSession &s, DiameterJobHandle &h)
  : AAA_StateMachine<DiameterMip4HaClientStateMachine>
  (*this, *DiameterMip4HaClientStateTable::instance(), "DIAMETER_MIP4_HA_CLIENT"),
    session(s),
    handle(h)
{}

void 
DiameterMip4HaClientStateMachine::SendAMR()
{

  AAAClientSession &session = Session();
  AMR_Data &amrData = AMR(); 

  DiameterMsg msg; 

  AMR_Parser parser;
  parser.setAppData(&amrData);   
  parser.setRawData(&msg);
  amrData.AuthApplicationId = Mip4ApplicationId;


#ifdef PRINT_MSG_CONTENT

  printf("AMR:\nMip-Reg-Request: %s\nUser Name: %s\nDestination Realm: %s\nDestination Host: %s\n\n",
	 amrData.MipRegRequest().data(), 
	 amrData.UserName().data(),
	 amrData.DestinationRealm().data(),
	 amrData.DestinationHost().data() );  
#endif

  try {
    parser.parseAppToRaw();
  }
  catch (DiameterParserError) {
    AAA_LOG((LM_ERROR, "[%N] Parsing error.\n"));
    return;
  }

  AAAMessageControl msgControl( &session );    
  if (msgControl.Send(msg) != AAA_ERR_SUCCESS) {
    AAA_LOG((LM_ERROR, "Failed sending message.\n"));
  }
  else {
    AAA_LOG((LM_DEBUG, "Sent AMR Message.\n"));
  }
}




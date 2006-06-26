/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2004 Open Diameter Project                          */
/*                                                                        */
/* This library is free software; you can redistribute it and/or modify   */
/* it under the terms of the GNU Lesser General Public License as         */
/* published by the Free Software Foundation; either version 2.1 of the   */
/* License, or (at your option) any later version.                        */
/*                                                                        */
/* This library is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      */
/* Lesser General Public License for more details.                        */
/*                                                                        */
/* You should have received a copy of the GNU Lesser General Public       */
/* License along with this library; if not, write to the Free Software    */
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
// $Id: diameter_eap_client_fsm.cxx,v 1.17 2004/08/16 16:17:25 vfajardo Exp $

// diameter_eap_client_fsm.cxx:  EAP session handling
// Written by Yoshihiro Ohba

#include <ace/Singleton.h>
#include <ace/Atomic_Op_T.h>
#include "diameter_eap_client_session.hxx"
#include "diameter_eap_client_fsm.hxx"
#include "diameter_eap_parser.hxx"

class DiameterEapClientAction 
  : public AAA_Action<DiameterEapClientStateMachine>
{
  virtual void operator()(DiameterEapClientStateMachine&)=0;
 protected:
  DiameterEapClientAction() {}
  ~DiameterEapClientAction() {}
};

/// State table used by DiameterEapClientStateMachine.
class DiameterEapClientStateTable_S 
  : public AAA_StateTable<DiameterEapClientStateMachine>
{
  friend class 
  ACE_Singleton<DiameterEapClientStateTable_S, ACE_Recursive_Thread_Mutex>;

 private:
  class AcSendEapRequest : public DiameterEapClientAction 
  {
    void operator()(DiameterEapClientStateMachine& sm)
    {
      DiameterEapClientSession& session = sm.Session();
      AAA_LOG(LM_DEBUG, "[%N] forwarding EAP Request to passthrough.\n");
      session.SignalContinue(sm.DEA().EapPayload());

      // Multi round timeout value is applied per EAP Request.
      DEA_Data& dea = sm.DEA();
      if (dea.MultiRoundTimeOut.IsSet())
	sm.EnforceMultiRoundTimeOut(dea.MultiRoundTimeOut());
    }
  };

  class AcSendDER : public DiameterEapClientAction 
  {
    void operator()(DiameterEapClientStateMachine& sm) 
    { 
      ACE_DEBUG((LM_DEBUG, "[%N] sending DER.\n"));
      DER_Data& der = sm.DER();

      // Generate authorization AVPs.
      sm.SetDestinationRealm(der.DestinationRealm);
      if (!der.DestinationRealm.IsSet())
	{
	  AAA_LOG(LM_ERROR, "Failed to set destination realm.\n");
	  sm.Event(DiameterEapClientStateMachine::EvSgDisconnect);
	  return;
	}

      sm.SetAuthRequestType(der.AuthRequestType);
      if (!der.AuthRequestType.IsSet())
	{
	  AAA_LOG(LM_ERROR, "Failed to set auth request type.\n");
	  sm.Event(DiameterEapClientStateMachine::EvSgDisconnect);
	  return;
	}

      sm.SetNasPort(der.NasPort);

      sm.SetNasPortId(der.NasPortId);

      sm.SetOriginStateId(der.OriginStateId);

      sm.SetNasIdentifier(der.NasIdentifier);

      sm.SetNasIpAddress(der.NasIpAddress);

      sm.SetNasIpv6Address(der.NasIpv6Address);

      sm.SetDestinationHost(der.DestinationHost);

      sm.SetUserName(der.UserName);

      sm.SetServiceType(der.ServiceType);

      sm.SetIdleTimeout(der.IdleTimeout);

      sm.SetState(der.State);

      sm.SetAuthorizationLifetime(der.AuthorizationLifetime);

      sm.SetAuthGracePeriod(der.AuthGracePeriod);

      sm.SetAuthSessionState(der.AuthSessionState);

      sm.SetSessionTimeout(der.SessionTimeout);

      sm.SetClass(der.Class);

      sm.SetPortLimit(der.PortLimit);

      sm.SetCallbackNumber(der.CallbackNumber);

      sm.SetCalledStationId(der.CalledStationId);

      sm.SetCallingStationId(der.CallingStationId);

      sm.SetOriginatingLineInfo(der.OriginatingLineInfo);

      sm.SetConnectInfo(der.ConnectInfo);

      sm.SetFramedCompression(der.FramedCompression);

      sm.SetFramedInterfaceId(der.FramedInterfaceId);

      sm.SetFramedIpAddress(der.FramedIpAddress);

      sm.SetFramedIpv6Prefix(der.FramedIpv6Prefix);

      sm.SetFramedIpNetmask(der.FramedIpNetmask);

      sm.SetFramedMtu(der.FramedMtu);

      sm.SetFramedProtocol(der.FramedProtocol);

      sm.SetTunneling(der.Tunneling);

      sm.SendDER(); 
    }
  };

  class AcCheckDEA_ResultCode : public DiameterEapClientAction 
  {
    void operator()(DiameterEapClientStateMachine& sm)
    {
      AAAResultCode resultCode = sm.DEA().ResultCode();
      switch (resultCode)
	{
	case AAA_SUCCESS :
	  AAA_LOG(LM_DEBUG, "[%N] AAA_SUCCESS received.\n");
	  if (sm.DER().AuthRequestType == 
	      AUTH_REQUEST_TYPE_AUTHORIZE_AUTHENTICATE)
	    sm.Event(DiameterEapClientStateTable_S::EvSgSuccess);
	  else
	    sm.Event(DiameterEapClientStateTable_S::EvSgLimitedSuccess);
	  break;
	case AAA_MULTI_ROUND_AUTH :
	  AAA_LOG(LM_DEBUG, "[%N] AAA_MULTI_ROUND_AUTH received.\n");
	  sm.Event(DiameterEapClientStateTable_S::EvSgContinue);
	  break;
	default:
	  AAA_LOG(LM_DEBUG, "[%N] Error was received.\n");
	  AAA_LOG(LM_DEBUG, "[%N]   Result-Code=%d.\n", resultCode);
	  sm.Event(DiameterEapClientStateTable_S::EvSgFailure);
	  break;
	}
    }
  };

  class AcCheckAA_AnswerResultCode : public DiameterEapClientAction 
  {
    void operator()(DiameterEapClientStateMachine& sm)
    {
      AAAResultCode resultCode = sm.DEA().ResultCode();
      switch (resultCode)
	{
	case AAA_SUCCESS :
	  AAA_LOG(LM_DEBUG, "[%N] AAA_SUCCESS received.\n");
	  sm.Event(DiameterEapClientStateTable_S::EvSgSuccess);
	  break;
	case AAA_AUTHORIZATION_REJECTED :
	  AAA_LOG(LM_DEBUG, 
		       "[%N] AAA_AUTHORIZATION_REJECTED received.\n");
	  sm.Event(DiameterEapClientStateTable_S::EvSgFailure);
	  break;
	default:
	  AAA_LOG(LM_DEBUG, "[%N] Error was received.\n");
	  AAA_LOG(LM_DEBUG, "[%N]   Result-Code=%d.\n", resultCode);
	  sm.Event(DiameterEapClientStateTable_S::EvSgFailure);
	  break;
	}
    }
  };

  class AcAccessAccept : public DiameterEapClientAction 
  {
    void operator()(DiameterEapClientStateMachine& sm)
    {
      DiameterEapClientSession& session = sm.Session();
      DEA_Data& dea = sm.DEA();
      DER_Data& der = sm.DER();
      sm.SignalSuccess(dea.EapPayload());
      session.Update(AAASession::EVENT_AUTH_SUCCESS);  

      // Enforce authorization data.
      if (dea.Class.IsSet())
	{
	  der.Class = dea.Class;
	  sm.EnforceClass(dea.Class);
	}

      if (dea.AcctInterimInterval.IsSet())
	sm.EnforceAcctInterimInterval(dea.AcctInterimInterval());


      if (dea.ServiceType.IsSet())
	{
	  der.ServiceType = dea.ServiceType();
	  sm.EnforceServiceType(dea.ServiceType());
	}

      if (dea.IdleTimeout.IsSet())
	sm.EnforceIdleTimeout(dea.IdleTimeout());

      if (dea.AuthorizationLifetime.IsSet())
	sm.EnforceAuthorizationLifetime(dea.AuthorizationLifetime());

      if (dea.AuthGracePeriod.IsSet())
	sm.EnforceAuthGracePeriod(dea.AuthGracePeriod());

      if (dea.AuthSessionState.IsSet())
	sm.EnforceAuthSessionState(dea.AuthSessionState());

      if (dea.ReAuthRequestType.IsSet())
	sm.EnforceReAuthRequestType(dea.ReAuthRequestType());

      if (dea.SessionTimeout.IsSet())
	sm.EnforceSessionTimeout(dea.SessionTimeout());

      if (dea.State.IsSet())
	der.State = dea.State();

      if (dea.FilterId.IsSet())
	sm.EnforceFilterId(dea.FilterId);

      if (dea.PortLimit.IsSet())
	{
	  der.PortLimit = dea.PortLimit();
	  sm.EnforcePortLimit(dea.PortLimit());
	}

      if (dea.CallbackId.IsSet())
	sm.EnforceCallbackId(dea.CallbackId());

      if (dea.CallbackNumber.IsSet())
	sm.EnforceCallbackNumber(dea.CallbackNumber());

      if (dea.FramedAppletalkLink.IsSet())
	sm.EnforceFramedAppletalkLink(dea.FramedAppletalkLink());

      if (dea.FramedAppletalkZone.IsSet())
	sm.EnforceFramedAppletalkZone(dea.FramedAppletalkZone());

      if (dea.FramedAppletalkNetwork.IsSet())
	sm.EnforceFramedAppletalkNetwork(dea.FramedAppletalkNetwork);

      if (dea.FramedCompression.IsSet())
	{
	  der.FramedCompression = dea.FramedCompression;
	  sm.EnforceFramedCompression(dea.FramedCompression);
	}

      if (dea.FramedIpAddress.IsSet())
	{
	  der.FramedIpAddress = dea.FramedIpAddress();
	  sm.EnforceFramedIpAddress(dea.FramedIpAddress());
	}

      if (dea.FramedIpv6Prefix.IsSet())
	{
	  der.FramedIpv6Prefix = dea.FramedIpv6Prefix;
	  sm.EnforceFramedIpv6Prefix(dea.FramedIpv6Prefix);
	}
      
      if (dea.FramedPool.IsSet())
	sm.EnforceFramedPool(dea.FramedPool());

      if (dea.FramedIpv6Pool.IsSet())
	sm.EnforceFramedIpv6Pool(dea.FramedIpv6Pool());

      if (dea.FramedIpv6Route.IsSet())
	sm.EnforceFramedIpv6Route(dea.FramedIpv6Route);

      if (dea.FramedIpNetmask.IsSet())
	{
	  der.FramedIpNetmask = dea.FramedIpNetmask();
	  sm.EnforceFramedIpNetmask(dea.FramedIpNetmask());
	}

      if (dea.FramedIpxNetwork.IsSet())
	sm.EnforceFramedIpxNetwork(dea.FramedIpxNetwork());

      if (dea.FramedMtu.IsSet())
	{
	  der.FramedMtu = dea.FramedMtu();
	  sm.EnforceFramedMtu(dea.FramedMtu());
	}

      if (dea.FramedProtocol.IsSet())
	{
	  der.FramedProtocol = dea.FramedProtocol();
	  sm.EnforceFramedProtocol(dea.FramedProtocol());
	}

      if (dea.FramedRouting.IsSet())
	sm.EnforceFramedRouting(dea.FramedRouting());

      if (dea.NasFilterRule.IsSet())
	sm.EnforceNasFilterRule(dea.NasFilterRule);

      der.Tunneling = dea.Tunneling;
      if (dea.Tunneling.size()>0)
	sm.EnforceTunneling(dea.Tunneling);

      if (dea.EapMasterSessionKey.IsSet())
	sm.EnforceEapMasterSessionKey(dea.EapMasterSessionKey());

      if (dea.AccountingEapAuthMethod.IsSet())
	sm.EnforceAccountingEapAuthMethod(dea.AccountingEapAuthMethod);
    }
  };

  class AcAccessReject : public DiameterEapClientAction 
  {
    void operator()(DiameterEapClientStateMachine& sm)
    {
      DiameterEapClientSession& session = sm.Session();
      std::string str;
      if (sm.DEA().EapPayload.IsSet())
	str = sm.DEA().EapPayload();
      sm.SignalFailure(str);
      session.Update(AAASession::EVENT_AUTH_FAILED);  
    }
  };

  class AcSendAA_Request : public DiameterEapClientAction 
  {
    void operator()(DiameterEapClientStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Unsupported feature.\n");
      sm.SendAA_Request();
    }
  };

  class AcReauthenticate : public DiameterEapClientAction 
  {
    void operator()(DiameterEapClientStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Reauthentication issued.\n");
      sm.SignalReauthentication();
    }
  };

  class AcDisconnect : public DiameterEapClientAction 
  {
    void operator()(DiameterEapClientStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Disconnect issued.\n");
      sm.SignalDisconnect();
    }
  };

  enum state {
    StInitialize,
    StWaitDEA,
    StCheckResultCode,
    StAccepted,
    StRejected,
    StWaitEapResponse,
    StWaitAA_Answer,
    StTerminated
  };

  enum {
    EvSgSuccess,
    EvSgLimitedSuccess,
    EvSgFailure,
    EvSgContinue
  };

  AcSendDER acSendDER;
  AcSendEapRequest acSendEapRequest;
  AcCheckDEA_ResultCode acCheckDEA_ResultCode;
  AcCheckAA_AnswerResultCode acCheckAA_AnswerResultCode;
  AcAccessAccept acAccessAccept;
  AcAccessReject acAccessReject;
  AcSendAA_Request acSendAA_Request;
  AcReauthenticate acReauthenticate;
  AcDisconnect acDisconnect;

  // Defined as a leaf class
  DiameterEapClientStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       DiameterEapClientStateMachine::EvRxEapResponse,
		       StWaitDEA, acSendDER);
    AddStateTableEntry(StInitialize, 
		       DiameterEapClientStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StInitialize, StInitialize);

    AddStateTableEntry(StWaitDEA,
		       DiameterEapClientStateMachine::EvRxDEA,
		       StCheckResultCode, acCheckDEA_ResultCode);
    AddStateTableEntry(StWaitDEA,
		       DiameterEapClientStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StWaitDEA, StTerminated);

    AddStateTableEntry(StCheckResultCode, EvSgSuccess,
		       StAccepted, acAccessAccept);
    AddStateTableEntry(StCheckResultCode, EvSgLimitedSuccess, 
		       StWaitAA_Answer, acSendAA_Request); 
    AddStateTableEntry(StCheckResultCode, EvSgFailure,
		       StRejected, acAccessReject);
    AddStateTableEntry(StCheckResultCode, EvSgContinue,
		       StWaitEapResponse, acSendEapRequest);

    AddStateTableEntry(StAccepted, 
		       DiameterEapClientStateMachine::EvSgDisconnect,
		       StTerminated, acDisconnect);
    AddStateTableEntry
      (StAccepted, 
       DiameterEapClientStateMachine::EvSgSessionTimeout,
       StTerminated, acDisconnect);
    AddStateTableEntry
      (StAccepted, 
       DiameterEapClientStateMachine::EvSgAuthGracePeriodTimeout,
       StTerminated, acDisconnect);
    AddStateTableEntry(StAccepted, 
		       DiameterEapClientStateMachine::EvSgAuthLifetimeTimeout,
		       StInitialize, acReauthenticate);
    AddWildcardStateTableEntry(StAccepted, StAccepted);
			
    AddWildcardStateTableEntry(StRejected, StTerminated);

    AddStateTableEntry(StWaitAA_Answer, 
		       DiameterEapClientStateMachine::EvRxAA_Answer,
		       StCheckResultCode, acCheckAA_AnswerResultCode);

    AddStateTableEntry(StWaitEapResponse,
		       DiameterEapClientStateMachine::EvRxEapResponse,
		       StWaitDEA, acSendDER);
    AddStateTableEntry(StWaitEapResponse,
		       DiameterEapClientStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StWaitEapResponse, StTerminated);

    AddWildcardStateTableEntry(StTerminated, StTerminated);

    InitialState(StInitialize);
  }
  ~DiameterEapClientStateTable_S() {}
};

typedef 
ACE_Singleton<DiameterEapClientStateTable_S, ACE_Recursive_Thread_Mutex> 
DiameterEapClientStateTable;

DiameterEapClientStateMachine::DiameterEapClientStateMachine
(DiameterEapClientSession& s, DiameterJobHandle &h)
  : AAA_StateMachine<DiameterEapClientStateMachine>
  (*this, *DiameterEapClientStateTable::instance(), "AAA_EAP_CLIENT"),
    session(s),
    handle(h)
{}

void 
DiameterEapClientStateMachine::SendDER()
{
  DiameterEapClientSession& session = Session();
  AAAMessage msg;

  DER_Parser parser;
  parser.setAppData(&derData);
  parser.setRawData(&msg);

  derData.AuthApplicationId = EapApplicationId;

  try {
    parser.parseAppToRaw();
  }
  catch (DiameterParserError) {
    AAA_LOG(LM_ERROR, "[%N] Parsing error.\n");
    return;
  }

  AAAMessageControl msgControl(session.Self());
  if (msgControl.Send(msg) != AAA_ERR_SUCCESS) {
    AAA_LOG(LM_ERROR, "Failed sending message.\n");
  }
  else {
    AAA_LOG(LM_DEBUG, "Sent DER Message.\n");
  }
}

void 
DiameterEapClientStateMachine::ForwardResponse(std::string &eapMsg)
{ 
  AAA_LOG(LM_ERROR, "[%N] EAP-Response received from passthrough.\n");
  derData.EapPayload = eapMsg;
  Notify(EvRxEapResponse);
}



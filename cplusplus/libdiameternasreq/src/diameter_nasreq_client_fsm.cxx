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
// $Id: diameter_nasreq_client_fsm.cxx,v 1.8 2004/08/16 16:17:26 vfajardo Exp $

// diameter_nasreq_client_fsm.cxx:  EAP session handling
// Written by Yoshihiro Ohba
// Created May 3, 2004

#include <ace/Singleton.h>
#include <ace/Atomic_Op_T.h>
#include "diameter_nasreq_client_session.hxx"
#include "diameter_nasreq_client_fsm.hxx"
#include "diameter_nasreq_parser.hxx"
#include "diameter_nasreq_authinfo.hxx"

class DiameterNasreqClientAction 
  : public AAA_Action<DiameterNasreqClientStateMachine>
{
  virtual void operator()(DiameterNasreqClientStateMachine&)=0;
 protected:
  DiameterNasreqClientAction() {}
  ~DiameterNasreqClientAction() {}
};

/// State table used by DiameterNasreqClientStateMachine.
class DiameterNasreqClientStateTable_S 
  : public AAA_StateTable<DiameterNasreqClientStateMachine>
{
  friend class 
  ACE_Singleton<DiameterNasreqClientStateTable_S, ACE_Recursive_Thread_Mutex>;

 private:
  class AcContinueAuthentication : public DiameterNasreqClientAction 
  {
    void operator()(DiameterNasreqClientStateMachine& sm)
    {
      AA_AnswerData& aaAnswer = sm.AA_Answer();
      ARAP_Info& arapInfo = (ARAP_Info&)sm.AuthenticationInfo();
      AAA_LOG(LM_DEBUG, "[%N] passing authinfo to the application.\n");

      // If the Auth-Request-Type is AUTHORIZE_ONLY, validation
      // completes with success.
      if (aaAnswer.AuthRequestType() == 
	  AUTH_REQUEST_TYPE_AUTHORIZE_ONLY)
	goto finish;

      // Check ARAP AVPs.
      if (sm.AuthenticationInfo().AuthenticationType() != 
	  NASREQ_AUTHENTICATION_TYPE_ARAP)
	goto finish;

      // Set ARAP-specific authentication info.
      if (aaAnswer.ArapChallengeResponse.IsSet())
	arapInfo.ArapChallengeResponse() = aaAnswer.ArapChallengeResponse();
      if (aaAnswer.ArapSecurity.IsSet())
	arapInfo.ArapSecurity() = aaAnswer.ArapSecurity();
      if (aaAnswer.ArapSecurityData.IsSet())
	arapInfo.ArapSecurityData() = aaAnswer.ArapSecurityData();
      if (aaAnswer.PasswordRetry.IsSet())
	arapInfo.PasswordRetry() = aaAnswer.PasswordRetry();

    finish:
	sm.SignalContinue(sm.AuthenticationInfo());
    }
  };

  class AcSendAA_Request : public DiameterNasreqClientAction 
  {
    void operator()(DiameterNasreqClientStateMachine& sm) 
    { 
      ACE_DEBUG((LM_DEBUG, "[%N] sending AA-Request.\n"));
      AA_RequestData& aaRequest = sm.AA_Request();

      // Generate authorization AVPs.
      sm.SetDestinationRealm(aaRequest.DestinationRealm);
      if (!aaRequest.DestinationRealm.IsSet())
	{
	  AAA_LOG(LM_ERROR, "Failed to set destination realm.\n");
	  sm.Event(DiameterNasreqClientStateMachine::EvSgDisconnect);
	  return;
	}

      sm.SetDestinationHost(aaRequest.DestinationHost);

      sm.SetAuthRequestType(aaRequest.AuthRequestType);
      if (!aaRequest.AuthRequestType.IsSet())
	{
	  AAA_LOG(LM_ERROR, "Failed to set auth request type.\n");
	  sm.Event(DiameterNasreqClientStateMachine::EvSgDisconnect);
	  return;
	}

      sm.SetNasIdentifier(aaRequest.NasIdentifier);

      sm.SetNasIpAddress(aaRequest.NasIpAddress);

      sm.SetNasIpv6Address(aaRequest.NasIpv6Address);

      sm.SetNasPort(aaRequest.NasPort);

      sm.SetNasPortId(aaRequest.NasPortId);

      sm.SetNasPortType(aaRequest.NasPortType);

      sm.SetOriginStateId(aaRequest.OriginStateId);

      sm.SetPortLimit(aaRequest.PortLimit);

      sm.SetServiceType(aaRequest.ServiceType);

      sm.SetState(aaRequest.State);

      sm.SetAuthorizationLifetime(aaRequest.AuthorizationLifetime);

      sm.SetAuthGracePeriod(aaRequest.AuthGracePeriod);

      sm.SetAuthSessionState(aaRequest.AuthSessionState);

      sm.SetCallbackNumber(aaRequest.CallbackNumber);

      sm.SetCalledStationId(aaRequest.CalledStationId);

      sm.SetCallingStationId(aaRequest.CallingStationId);

      sm.SetOriginatingLineInfo(aaRequest.OriginatingLineInfo);

      sm.SetConnectInfo(aaRequest.ConnectInfo);

      sm.SetFramedCompression(aaRequest.FramedCompression);

      sm.SetFramedInterfaceId(aaRequest.FramedInterfaceId);

      sm.SetFramedIpAddress(aaRequest.FramedIpAddress);

      sm.SetFramedIpv6Prefix(aaRequest.FramedIpv6Prefix);

      sm.SetFramedIpNetmask(aaRequest.FramedIpNetmask);

      sm.SetFramedMtu(aaRequest.FramedMtu);

      sm.SetFramedProtocol(aaRequest.FramedProtocol);

      // Deal with authinfo.
      DiameterNasreqAuthenticationInfo &authInfo =sm.AuthenticationInfo();
      if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_NONE)
	{
	  if (aaRequest.AuthRequestType() != AUTH_REQUEST_TYPE_AUTHORIZE_ONLY)
	    {
	      AAA_LOG(LM_ERROR, "Failed to set authinfo type.\n");
	      sm.Event(DiameterNasreqClientStateMachine::EvSgDisconnect);
	      return;
	    }
	  else
	    goto next;
	}

      aaRequest.UserName = authInfo.UserName();

      if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_ARAP &&
	  !aaRequest.FramedProtocol.IsSet())
	goto next;
      
      if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_PAP)
	{
	  PAP_Info& papInfo = (PAP_Info&)authInfo;
	  aaRequest.UserPassword = papInfo.UserPassword();
	}
      else if (authInfo.AuthenticationType() == 
	       NASREQ_AUTHENTICATION_TYPE_CHAP)
	{
	  CHAP_Info& chapInfo = (CHAP_Info&)authInfo;
	  aaRequest.ChapAuth = chapInfo.ChapAuth();
	  aaRequest.ChapChallenge = chapInfo.ChapChallenge();
	}
      else if (authInfo.AuthenticationType() == 
	       NASREQ_AUTHENTICATION_TYPE_ARAP)
	{
	  ARAP_Info& arapInfo = (ARAP_Info&)authInfo;
	  if (arapInfo.IsFirst())
	    {
	      aaRequest.ArapPassword = arapInfo.ArapPassword();
	      aaRequest.ArapChallengeResponse = 
		arapInfo.ArapChallengeResponse();
	    }
	  else
	    {
	      aaRequest.ArapChallengeResponse = 
		arapInfo.ArapChallengeResponse();
	      aaRequest.ArapSecurity = arapInfo.ArapSecurity();
	      aaRequest.ArapSecurityData = arapInfo.ArapSecurityData();
	    }
	}

    next:      

      sm.SetLoginIpHost(aaRequest.LoginIpHost);

      sm.SetLoginIpHost(aaRequest.LoginIpv6Host);

      sm.SetLoginLatGroup(aaRequest.LoginLatGroup);

      sm.SetLoginLatNode(aaRequest.LoginLatNode);

      sm.SetLoginLatPort(aaRequest.LoginLatPort);

      sm.SetLoginLatService(aaRequest.LoginLatService);

      sm.SetTunneling(aaRequest.Tunneling);

      sm.SendAA_Request(); 
    }
  };

  class AcCheckAA_AnswerResultCode : public DiameterNasreqClientAction 
  {
    void operator()(DiameterNasreqClientStateMachine& sm)
    {
      AA_RequestData& aaRequest = sm.AA_Request();
      AA_AnswerData& aaAnswer = sm.AA_Answer();
      AAAResultCode resultCode = aaAnswer.ResultCode();

      // Enforce Prompt AVP attribute.
      if (aaAnswer.Prompt.IsSet())
	sm.EnforcePrompt(aaAnswer.Prompt());

      switch (resultCode)
	{
	case AAA_SUCCESS :
	  AAA_LOG(LM_DEBUG, "[%N] AAA_SUCCESS received.\n");
	  if (aaRequest.AuthRequestType() != aaAnswer.AuthRequestType())
	    {
	      AAA_LOG(LM_ERROR, "[%N] request type mismatch.\n");
	      sm.Event(DiameterNasreqClientStateTable_S::EvSgFailure);
	      break;
	    }
	  if (aaRequest.AuthRequestType() != 
	      AUTH_REQUEST_TYPE_AUTHORIZE_AUTHENTICATE && 
	      aaRequest.AuthRequestType() != AUTH_REQUEST_TYPE_AUTHORIZE_ONLY)
	    {
	      AAA_LOG(LM_ERROR, "[%N] request type invalid.\n");
	      sm.Event(DiameterNasreqClientStateTable_S::EvSgFailure);
	      break;
	    }
	  sm.Event(DiameterNasreqClientStateTable_S::EvSgSuccess);
	  break;
	case AAA_MULTI_ROUND_AUTH :
	  AAA_LOG(LM_DEBUG, "[%N] AAA_MULTI_ROUND_AUTH received.\n");
	  if (aaRequest.AuthRequestType() != aaAnswer.AuthRequestType())
	    {
	      AAA_LOG(LM_ERROR, "[%N] request type mismatch.\n");
	      sm.Event(DiameterNasreqClientStateTable_S::EvSgFailure);
	      break;
	    }
	  if (aaRequest.AuthRequestType() != 
	      AUTH_REQUEST_TYPE_AUTHORIZE_AUTHENTICATE && 
	       aaRequest.AuthRequestType() != 
	      AUTH_REQUEST_TYPE_AUTHENTICATION_ONLY)
	    {
	      AAA_LOG(LM_ERROR, "[%N] request type invalid.\n");
	      sm.Event(DiameterNasreqClientStateTable_S::EvSgFailure);
	      break;
	    }
	  if (aaAnswer.FramedProtocol.IsSet() &&
	      aaAnswer.FramedProtocol() == 3 && 
	      aaRequest.FramedProtocol() == 3 &&
	      aaRequest.ArapSecurity.IsSet() && 
	      aaRequest.ArapSecurityData.IsSet())
	    sm.Event(DiameterNasreqClientStateTable_S::EvSgContinue);
	  else
	    sm.Event(DiameterNasreqClientStateTable_S::EvSgFailure);
	  break;
	default:
	  AAA_LOG(LM_DEBUG, "[%N] Error was received.\n");
	  AAA_LOG(LM_DEBUG, "[%N]   Result-Code=%d.\n", resultCode);
	  sm.Event(DiameterNasreqClientStateTable_S::EvSgFailure);
	  break;
	}
    }
  };

  class AcAccessAccept : public DiameterNasreqClientAction 
  {
    void operator()(DiameterNasreqClientStateMachine& sm)
    {
      AA_AnswerData& aaAnswer = sm.AA_Answer();
      AA_RequestData& aaRequest = sm.AA_Request();
      sm.SignalSuccess();
      sm.Session().Update(AAASession::EVENT_AUTH_SUCCESS);  

      // Enforce authorization data.
      
      if (aaAnswer.Class.IsSet())
	sm.EnforceClass(aaAnswer.Class);

      if (aaAnswer.AcctInterimInterval.IsSet())
	sm.EnforceAcctInterimInterval(aaAnswer.AcctInterimInterval());


      if (aaAnswer.ServiceType.IsSet())
	{
	  aaRequest.ServiceType = aaAnswer.ServiceType();
	  sm.EnforceServiceType(aaAnswer.ServiceType());
	}

      if (aaAnswer.IdleTimeout.IsSet())
	sm.EnforceIdleTimeout(aaAnswer.IdleTimeout());

      if (aaAnswer.AuthorizationLifetime.IsSet())
	sm.EnforceAuthorizationLifetime(aaAnswer.AuthorizationLifetime());

      if (aaAnswer.AuthGracePeriod.IsSet())
	sm.EnforceAuthGracePeriod(aaAnswer.AuthGracePeriod());

      if (aaAnswer.AuthSessionState.IsSet())
	sm.EnforceAuthSessionState(aaAnswer.AuthSessionState());

      if (aaAnswer.ReAuthRequestType.IsSet())
	sm.EnforceReAuthRequestType(aaAnswer.ReAuthRequestType());

      if (aaAnswer.SessionTimeout.IsSet())
	sm.EnforceSessionTimeout(aaAnswer.SessionTimeout());

      if (aaAnswer.State.IsSet())
	aaRequest.State = aaAnswer.State();

      if (aaAnswer.FilterId.IsSet())
	sm.EnforceFilterId(aaAnswer.FilterId);

      if (aaAnswer.PortLimit.IsSet())
	{
	  aaRequest.PortLimit = aaAnswer.PortLimit();
	  sm.EnforcePortLimit(aaAnswer.PortLimit());
	}

      if (aaAnswer.CallbackId.IsSet())
	sm.EnforceCallbackId(aaAnswer.CallbackId());

      if (aaAnswer.CallbackNumber.IsSet())
	sm.EnforceCallbackNumber(aaAnswer.CallbackNumber());

      if (aaAnswer.FramedAppletalkLink.IsSet())
	sm.EnforceFramedAppletalkLink(aaAnswer.FramedAppletalkLink());

      if (aaAnswer.FramedAppletalkZone.IsSet())
	sm.EnforceFramedAppletalkZone(aaAnswer.FramedAppletalkZone());

      if (aaAnswer.FramedAppletalkNetwork.IsSet())
	sm.EnforceFramedAppletalkNetwork(aaAnswer.FramedAppletalkNetwork);

      if (aaAnswer.FramedCompression.IsSet())
	{
	  aaRequest.FramedCompression = aaAnswer.FramedCompression;
	  sm.EnforceFramedCompression(aaAnswer.FramedCompression);
	}

      if (aaAnswer.FramedIpAddress.IsSet())
	{
	  aaRequest.FramedIpAddress = aaAnswer.FramedIpAddress();
	  sm.EnforceFramedIpAddress(aaAnswer.FramedIpAddress());
	}

      if (aaAnswer.FramedIpv6Prefix.IsSet())
	{
	  aaRequest.FramedIpv6Prefix = aaAnswer.FramedIpv6Prefix;
	  sm.EnforceFramedIpv6Prefix(aaAnswer.FramedIpv6Prefix);
	}
      
      if (aaAnswer.FramedPool.IsSet())
	sm.EnforceFramedPool(aaAnswer.FramedPool());

      if (aaAnswer.FramedIpv6Pool.IsSet())
	sm.EnforceFramedIpv6Pool(aaAnswer.FramedIpv6Pool());

      if (aaAnswer.FramedIpv6Route.IsSet())
	sm.EnforceFramedIpv6Route(aaAnswer.FramedIpv6Route);

      if (aaAnswer.FramedIpNetmask.IsSet())
	{
	  aaRequest.FramedIpNetmask = aaAnswer.FramedIpNetmask();
	  sm.EnforceFramedIpNetmask(aaAnswer.FramedIpNetmask());
	}

      if (aaAnswer.FramedIpxNetwork.IsSet())
	sm.EnforceFramedIpxNetwork(aaAnswer.FramedIpxNetwork());

      if (aaAnswer.FramedMtu.IsSet())
	{
	  aaRequest.FramedMtu = aaAnswer.FramedMtu();
	  sm.EnforceFramedMtu(aaAnswer.FramedMtu());
	}

      if (aaAnswer.FramedProtocol.IsSet())
	{
	  aaRequest.FramedProtocol = aaAnswer.FramedProtocol();
	  sm.EnforceFramedProtocol(aaAnswer.FramedProtocol());
	}

      if (aaAnswer.FramedRouting.IsSet())
	sm.EnforceFramedRouting(aaAnswer.FramedRouting());

      if (aaAnswer.NasFilterRule.IsSet())
	sm.EnforceNasFilterRule(aaAnswer.NasFilterRule);

      if (aaAnswer.LoginIpHost.IsSet())
	{
	  aaRequest.LoginIpHost = aaAnswer.LoginIpHost();
	  sm.EnforceLoginIpHost(aaAnswer.LoginIpHost);
	}

      if (aaAnswer.LoginIpv6Host.IsSet())
	{
	  aaRequest.LoginIpv6Host = aaAnswer.LoginIpv6Host();
	  sm.EnforceLoginIpv6Host(aaAnswer.LoginIpv6Host);
	}

      if (aaAnswer.LoginIpv6Host.IsSet())
	{
	  aaRequest.LoginIpv6Host = aaAnswer.LoginIpv6Host();
	  sm.EnforceLoginIpv6Host(aaAnswer.LoginIpv6Host);
	}

      if (aaAnswer.LoginLatGroup.IsSet())
	{
	  aaRequest.LoginLatGroup = aaAnswer.LoginLatGroup();
	  sm.EnforceLoginLatGroup(aaAnswer.LoginLatGroup());
	}

      if (aaAnswer.LoginLatNode.IsSet())
	{
	  aaRequest.LoginLatNode = aaAnswer.LoginLatNode();
	  sm.EnforceLoginLatNode(aaAnswer.LoginLatNode());
	}

      if (aaAnswer.LoginLatPort.IsSet())
	{
	  aaRequest.LoginLatPort = aaAnswer.LoginLatPort();
	  sm.EnforceLoginLatPort(aaAnswer.LoginLatPort());
	}

      if (aaAnswer.LoginLatService.IsSet())
	{
	  aaRequest.LoginLatService = aaAnswer.LoginLatService();
	  sm.EnforceLoginLatService(aaAnswer.LoginLatService());
	}

      if (aaAnswer.LoginTcpPort.IsSet())
	{
	  sm.EnforceLoginTcpPort(aaAnswer.LoginTcpPort());
	}

      aaRequest.Tunneling = aaAnswer.Tunneling;
      if (aaAnswer.Tunneling.size()>0)
	sm.EnforceTunneling(aaAnswer.Tunneling);
    }
  };

  class AcAccessReject : public DiameterNasreqClientAction 
  {
    void operator()(DiameterNasreqClientStateMachine& sm)
    {
      sm.SignalFailure();
      sm.Session().Update(AAASession::EVENT_AUTH_FAILED);  
    }
  };

  class AcReauthenticate : public DiameterNasreqClientAction 
  {
    void operator()(DiameterNasreqClientStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Reauthentication issued.\n");
      sm.SignalReauthentication();
    }
  };

  class AcDisconnect : public DiameterNasreqClientAction 
  {
    void operator()(DiameterNasreqClientStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Disconnect issued.\n");
      sm.SignalDisconnect();
    }
  };

  enum state {
    StInitialize,
    StWaitAA_Answer,
    StCheckResultCode,
    StAccepted,
    StRejected,
    StWaitAuthInfo,
    StTerminated
  };

  enum {
    EvSgSuccess,
    EvSgFailure,
    EvSgContinue
  };

  AcSendAA_Request acSendAA_Request;
  AcContinueAuthentication acContinueAuthentication;
  AcCheckAA_AnswerResultCode acCheckAA_AnswerResultCode;
  AcAccessAccept acAccessAccept;
  AcAccessReject acAccessReject;
  AcReauthenticate acReauthenticate;
  AcDisconnect acDisconnect;

  // Defined as a leaf class
  DiameterNasreqClientStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       DiameterNasreqClientStateMachine::EvRxAuthInfo,
		       StWaitAA_Answer, acSendAA_Request);
    AddStateTableEntry(StInitialize, 
		       DiameterNasreqClientStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StInitialize, StInitialize);

    AddStateTableEntry(StWaitAA_Answer,
		       DiameterNasreqClientStateMachine::EvRxAA_Answer,
		       StCheckResultCode, acCheckAA_AnswerResultCode);
    AddStateTableEntry(StWaitAA_Answer,
		       DiameterNasreqClientStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StWaitAA_Answer, StTerminated);

    AddStateTableEntry(StCheckResultCode, EvSgSuccess,
		       StAccepted, acAccessAccept);
    AddStateTableEntry(StCheckResultCode, EvSgFailure,
		       StRejected, acAccessReject);
    AddStateTableEntry(StCheckResultCode, EvSgContinue,
		       StWaitAuthInfo, acContinueAuthentication);

    AddStateTableEntry(StAccepted, 
		       DiameterNasreqClientStateMachine::EvSgDisconnect,
		       StTerminated, acDisconnect);
    AddStateTableEntry
      (StAccepted, 
       DiameterNasreqClientStateMachine::EvSgSessionTimeout,
       StTerminated, acDisconnect);
    AddStateTableEntry
      (StAccepted, 
       DiameterNasreqClientStateMachine::EvSgAuthGracePeriodTimeout,
       StTerminated, acDisconnect);
    AddStateTableEntry(StAccepted, 
		       DiameterNasreqClientStateMachine
		       ::EvSgAuthLifetimeTimeout,
		       StInitialize, acReauthenticate);
    AddWildcardStateTableEntry(StAccepted, StAccepted);
			
    AddWildcardStateTableEntry(StRejected, StTerminated);

    AddStateTableEntry(StWaitAuthInfo,
		       DiameterNasreqClientStateMachine::EvRxAuthInfo,
		       StWaitAA_Answer, acSendAA_Request);
    AddStateTableEntry(StWaitAuthInfo,
		       DiameterNasreqClientStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StWaitAuthInfo, StTerminated);

    AddWildcardStateTableEntry(StTerminated, StTerminated);

    InitialState(StInitialize);
  }
  ~DiameterNasreqClientStateTable_S() {}
};

typedef 
ACE_Singleton<DiameterNasreqClientStateTable_S, ACE_Recursive_Thread_Mutex> 
DiameterNasreqClientStateTable;

DiameterNasreqClientStateMachine::DiameterNasreqClientStateMachine
(DiameterNasreqClientSession& s, DiameterJobHandle &h)
  : AAA_StateMachine<DiameterNasreqClientStateMachine>
  (*this, *DiameterNasreqClientStateTable::instance(), "AAA_NASREQ_CLIENT"),
    session(s),
    handle(h),
    authenticationInfo
  (boost::shared_ptr<DiameterNasreqAuthenticationInfo>())
{}

void 
DiameterNasreqClientStateMachine::SendAA_Request()
{
  AAAMessage msg;
  AA_RequestData aaRequest = aaRequestData;
  AA_RequestParser parser;
  parser.setAppData(&aaRequest);
  parser.setRawData(&msg);

  aaRequest.AuthApplicationId = NasreqApplicationId;

  try {
    parser.parseAppToRaw();
  }
  catch (DiameterParserError) {
    AAA_LOG(LM_ERROR, "[%N] Parsing error.\n");
    return;
  }

  AAAMessageControl msgControl(Session().Self());
  if (msgControl.Send(msg) != AAA_ERR_SUCCESS) {
    AAA_LOG(LM_ERROR, "Failed sending message.\n");
  }
  else {
    AAA_LOG(LM_DEBUG, "Sent AA-Request Message.\n");
  }
}

void 
DiameterNasreqClientStateMachine::ForwardAuthenticationInfo
(DiameterNasreqAuthenticationInfo &authInfo)
{ 
  AAA_LOG(LM_ERROR, "[%N] Authinfo received from the application.\n");
  if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_NONE)
    {
      AAA_LOG(LM_ERROR, "Failed to set authinfo type.\n");
      return;
    }
  if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_PAP)
    {
      AAA_LOG(LM_DEBUG, "PAP info.\n");
      authenticationInfo = boost::shared_ptr<PAP_Info>
	(new PAP_Info((PAP_Info&)authInfo));
    }
  else if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_CHAP)
    {
      AAA_LOG(LM_DEBUG, "CHAP info.\n");
      authenticationInfo = boost::shared_ptr
	<CHAP_Info>(new CHAP_Info((CHAP_Info&)authInfo));
    }
  else 
    {
      AAA_LOG(LM_DEBUG, "ARAP info.\n");
      authenticationInfo = boost::shared_ptr<ARAP_Info>
	(new ARAP_Info((ARAP_Info&)authInfo));
    }
  Notify(EvRxAuthInfo);
}



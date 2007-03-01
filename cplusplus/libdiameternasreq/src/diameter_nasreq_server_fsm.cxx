/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* Open Diameter: Open-source software for the Diameter and               */
/*                Diameter related protocols                              */
/*                                                                        */
/* Copyright (C) 2002-2007 Open Diameter Project                          */
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
// $Id: diameter_nasreq_server_fsm.cxx,v 1.7 2004/08/16 16:17:26 vfajardo Exp $

// diameter_nasreq_server_fsm.cxx:  NASREQ session handling
// Written by Yoshihiro Ohba
// Created May 3, 2004

#include <ace/Singleton.h>
#include <ace/Atomic_Op_T.h>
#include "diameter_nasreq_server_session.hxx"
#include "diameter_nasreq_server_fsm.hxx"
#include "diameter_nasreq_parser.hxx"
#include "diameter_nasreq_authinfo.hxx"

class DiameterNasreqServerAction 
  : public AAA_Action<DiameterNasreqServerStateMachine>
{
  virtual void operator()(DiameterNasreqServerStateMachine&)=0;
 protected:
  DiameterNasreqServerAction() {}
  ~DiameterNasreqServerAction() {}
};

/// State table used by DiameterEapServerStateMachine.
class DiameterNasreqServerStateTable_S 
  : public AAA_StateTable<DiameterNasreqServerStateMachine>
{
  friend class 
  ACE_Singleton<DiameterNasreqServerStateTable_S, ACE_Recursive_Thread_Mutex>;

 private:
  class AcForwardAuthInfo : public DiameterNasreqServerAction 
  {
    void operator()(DiameterNasreqServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "[%N] forwarding authinfo to application.\n"));
      sm.ForwardAuthenticationInfo(sm.AuthenticationInfo());
    }
  };

  class AcCheckAA_Request : public DiameterNasreqServerAction 
  {
    void operator()(DiameterNasreqServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, "[%N] Copying AA-Request to AA-Answer.\n"));
      if (sm.CheckAA_Request())
	sm.Event(DiameterNasreqServerStateMachine::EvSgValidAA_Request);
      else
	sm.Event(DiameterNasreqServerStateMachine::EvSgInvalidAA_Request);
    }
  };

  class AcSendAA_AnswerWithContinue : public DiameterNasreqServerAction 
  {
    void operator()(DiameterNasreqServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
		   "[%N] Sending AA-Answer with AAA_MULTI_ROUND_AUTH.\n"));

      // Set Result-Code to AAA_MULTI_ROUND_AUTH.
      AA_AnswerData& aaAnswer = sm.AA_Answer();
      aaAnswer.ResultCode = AAA_MULTI_ROUND_AUTH;

      // Set reply message if needed.
      aaAnswer.ReplyMessage.Clear();
      sm.SetReplyMessage(aaAnswer.ReplyMessage, aaAnswer.ResultCode());

      // Set ARAP AVPs.  
      if (sm.AuthenticationInfo().AuthenticationType() 
	  != NASREQ_AUTHENTICATION_TYPE_ARAP)
	{
	  AAA_LOG((LM_DEBUG, "[%N] Unacceptable auth type in multi-round auth.\n"));
	  sm.Abort();
	  return;
	}
      ARAP_Info& arapInfo = (ARAP_Info&)sm.AuthenticationInfo();
      aaAnswer.ArapSecurity = arapInfo.ArapSecurity();
      aaAnswer.ArapSecurityData = arapInfo.ArapSecurityData();
      if (arapInfo.PasswordRetry() != 0)
	aaAnswer.PasswordRetry = arapInfo.PasswordRetry();
	
      sm.SendAA_Answer(); 

      // Update the session state.
      sm.Session().Update(AAASession::EVENT_AUTH_CONTINUE);
    }
  };

  class AcSendAA_AnswerWithSuccess : public DiameterNasreqServerAction 
  {
    void operator()(DiameterNasreqServerStateMachine& sm)
    {
      AAA_LOG((LM_DEBUG, 
		   "[%N] Sending AA-Answer with a success Result-Code.\n"));
      AA_AnswerData& aaAnswer = sm.AA_Answer();

      aaAnswer.ResultCode = AAA_SUCCESS;

      // Set ARAP-Challenge-Response AVP.
      if (sm.AuthenticationInfo().AuthenticationType() == 
	  NASREQ_AUTHENTICATION_TYPE_ARAP)
	{
	  ARAP_Info& arapInfo = (ARAP_Info&)sm.AuthenticationInfo();
	  aaAnswer.ArapChallengeResponse = arapInfo.ArapChallengeResponse();
	}

      sm.SendAA_Answer(); 
      sm.Session().Update(AAASession::EVENT_AUTH_SUCCESS);
    }
  };

  class AcSendAA_AnswerDueToAuthenticationFailure 
    : public DiameterNasreqServerAction 
  {
    void operator()(DiameterNasreqServerStateMachine& sm)
    {
      AA_AnswerData& aaAnswer = sm.AA_Answer();
      AAA_LOG((LM_DEBUG, 
		   "[%N] Sending AA-Answer due to authentication failure.\n"));

      aaAnswer.ResultCode = AAA_AUTHENTICATION_REJECTED;
      // Set ARAP-Challenge-Response AVP.
      if (sm.AuthenticationInfo().AuthenticationType() == 
	  NASREQ_AUTHENTICATION_TYPE_ARAP)
	{
	  ARAP_Info& arapInfo = (ARAP_Info&)sm.AuthenticationInfo();
	  if (arapInfo.PasswordRetry() != 0)
	    aaAnswer.PasswordRetry = arapInfo.PasswordRetry();
	}
      sm.SendAA_Answer(); 
      sm.Session().Update(AAASession::EVENT_AUTH_FAILED);
    }
  };

  class AcSendAA_AnswerDueToAuthorizationFailure 
    : public DiameterNasreqServerAction 
  {
    void operator()(DiameterNasreqServerStateMachine& sm)
    {
      AA_AnswerData& aaAnswer = sm.AA_Answer();
      AAA_LOG((LM_DEBUG, 
		   "[%N] Sending AA-Answer due to authorization failure.\n"));

      aaAnswer.ResultCode = AAA_AUTHORIZATION_REJECTED;
      sm.SendAA_Answer(); 
      sm.Session().Update(AAASession::EVENT_AUTH_FAILED);
    }
  };

  class AcSendAA_AnswerDueToInvalidAA_Request 
    : public DiameterNasreqServerAction 
  {
    void operator()(DiameterNasreqServerStateMachine& sm)
    {
      AA_AnswerData& aaAnswer = sm.AA_Answer();
      AAA_LOG((LM_DEBUG, 
		   "[%N] Sending AA-Answer due to invalid AA-Request.\n"));

      aaAnswer.ResultCode = AAA_INVALID_AVP_VALUE;
      sm.Session().Update(AAASession::EVENT_AUTH_CONTINUE);
      sm.SendAA_Answer(); 
    }
  };

  class AcAuthorize : public DiameterNasreqServerAction 
  {
    void operator()(DiameterNasreqServerStateMachine& sm)
    {
      if (sm.AuthorizationDone() || sm.Authorize())
	sm.Event(DiameterNasreqServerStateMachine::EvSgAuthorizationSuccess);
      else
	sm.Event(DiameterNasreqServerStateMachine::EvSgAuthorizationFailure);
    }
  };

  enum state {
    StInitialize,
    StWaitAA_Request,
    StCheckAA_Request,
    StWaitAuthorization,
    StAccepted,
    StRejected,
    StWaitAuthInfo,
    StTerminated
  };

  enum {
    EvSgSuccess,
    EvSgLimitedSuccess,
    EvSgFailure,
    EvSgContinue,
    EvSgValid,
    EvSgInvalid
  };

  AcSendAA_AnswerWithContinue acSendAA_AnswerWithContinue; 
  AcSendAA_AnswerWithSuccess acSendAA_AnswerWithSuccess; 
  AcSendAA_AnswerDueToAuthenticationFailure 
  acSendAA_AnswerDueToAuthenticationFailure;
  AcSendAA_AnswerDueToAuthorizationFailure 
  acSendAA_AnswerDueToAuthorizationFailure;
  AcSendAA_AnswerDueToInvalidAA_Request acSendAA_AnswerDueToInvalidAA_Request;
  AcForwardAuthInfo acForwardAuthInfo; 
  AcCheckAA_Request acCheckAA_Request; 
  AcAuthorize acAuthorize;

  // Defined as a leaf class
  DiameterNasreqServerStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       DiameterNasreqServerStateMachine::EvRxAA_Request,
		       StCheckAA_Request, acCheckAA_Request);
    AddStateTableEntry(StInitialize, 
		       DiameterNasreqServerStateMachine::EvSgDisconnect, 
		       StTerminated);
    AddWildcardStateTableEntry(StInitialize, StTerminated);

    AddStateTableEntry(StCheckAA_Request, 
		       DiameterNasreqServerStateMachine::EvSgValidAA_Request, 
		       StWaitAuthInfo, acForwardAuthInfo);
    AddStateTableEntry(StCheckAA_Request,
		       DiameterNasreqServerStateMachine::
		       EvSgInvalidAA_Request, 
		       StRejected, acSendAA_AnswerDueToInvalidAA_Request);

    AddStateTableEntry(StWaitAA_Request, 
		       DiameterNasreqServerStateMachine::EvRxAA_Request,
		       StCheckAA_Request, acCheckAA_Request);
    AddStateTableEntry(StWaitAA_Request,
		       DiameterNasreqServerStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StWaitAA_Request, StTerminated);

    AddStateTableEntry(StWaitAuthInfo,
		       DiameterNasreqServerStateMachine::EvRxAuthContinue,
		       StWaitAA_Request, acSendAA_AnswerWithContinue);
    AddStateTableEntry(StWaitAuthInfo,
		       DiameterNasreqServerStateMachine::EvRxAuthSuccess,
		       StWaitAuthorization, acAuthorize);
    AddStateTableEntry(StWaitAuthInfo,
		       DiameterNasreqServerStateMachine::EvRxAuthFailure,
		       StRejected, acSendAA_AnswerDueToAuthenticationFailure);
    AddStateTableEntry(StWaitAuthInfo,
		       DiameterNasreqServerStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StWaitAuthInfo, StTerminated);

    AddStateTableEntry(StWaitAuthorization, 
		       DiameterNasreqServerStateMachine::
		       EvSgAuthorizationSuccess,
		       StAccepted, acSendAA_AnswerWithSuccess);
    AddStateTableEntry(StWaitAuthorization, 
		       DiameterNasreqServerStateMachine::
		       EvSgAuthorizationFailure,
		       StRejected, acSendAA_AnswerDueToAuthorizationFailure);
    AddStateTableEntry(StWaitAuthorization,
		       DiameterNasreqServerStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StWaitAuthorization, StTerminated);

    // Re-authentication
    AddStateTableEntry(StAccepted, 
		       DiameterNasreqServerStateMachine::EvRxAA_Request,
		       StCheckAA_Request, acCheckAA_Request);
    AddStateTableEntry(StAccepted,
		       DiameterNasreqServerStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StAccepted, StTerminated);

    AddWildcardStateTableEntry(StRejected, StRejected);

    AddWildcardStateTableEntry(StTerminated, StTerminated);

    InitialState(StInitialize);
  }
  ~DiameterNasreqServerStateTable_S() {}
};

typedef 
ACE_Singleton<DiameterNasreqServerStateTable_S, ACE_Recursive_Thread_Mutex> 
DiameterNasreqServerStateTable;

DiameterNasreqServerStateMachine::DiameterNasreqServerStateMachine
(DiameterNasreqServerSession& s, DiameterNasreqJobHandle &h)
  : AAA_StateMachine<DiameterNasreqServerStateMachine>
  (*this, *DiameterNasreqServerStateTable::instance(), 
   "AAA_NASREQ_SERVER"),
    session(s),
    handle(h),
    authorizationDone(false)
{}

void 
DiameterNasreqServerStateMachine::SendAA_Answer(){
  DiameterMsg msg;

  aaAnswerData.AuthApplicationId = NasreqApplicationId;

  AA_AnswerParser parser;
  parser.setAppData(&aaAnswerData);
  parser.setRawData(&msg);

  try {
    parser.parseAppToRaw();
  }
  catch (DiameterParserError) {
    AAA_LOG((LM_ERROR, "[%N] Parsing error.\n"));
    return;
  }

  AAAMessageControl msgControl(Session().Self());
  if (msgControl.Send(msg) != AAA_ERR_SUCCESS) {
    AAA_LOG((LM_ERROR, "Failed sending message.\n"));
  }
  else {
    AAA_LOG((LM_DEBUG, "Sent AA-Answer Message.\n"));
  }
}

bool
DiameterNasreqServerStateMachine::CheckAA_Request()
{
  AA_RequestData& aaRequest = aaRequestData;
  AA_AnswerData& aaAnswer = aaAnswerData;

  // Validate Auth-Request-Type.
  if (!ValidateAuthRequestType(aaRequest.AuthRequestType()))
    {
      AAA_LOG((LM_ERROR, "[%N] Invalid auth request type.\n"));
      return false;
    }
  aaAnswer.AuthRequestType = aaRequest.AuthRequestType();

  // If the Auth-Request-Type is AUTHORIZATION_ONLY, validation
  // completes with success.
  if (aaRequest.AuthRequestType() == AUTH_REQUEST_TYPE_AUTHORIZE_ONLY)
    return true;

  // If the authentication type is already chosen, skip the
  // authentication type selection procedure.
  if (authenticationInfo.get() != 0)
    {
      // This should be multi-round ARAP authentication.
      if (authenticationInfo->AuthenticationType() !=
	  NASREQ_AUTHENTICATION_TYPE_ARAP)
	{
	  AAA_LOG((LM_ERROR, 
		       "[%N] Multi-round not allowed for PAP and CHAP.\n"));
	  return false;
	}
      goto arap_multi_round_check;
    }

  // Authentication type selection procedure.
  //
  // Only one type of authentication information is chosen.  The
  // authentication type is chosen in the following way:
  //
  // - If there is no User-Name AVP, then validation fails.
  //
  // - If there is a CHAP-Auth AVP, then CHAP is chosen as the
  // authentication type.
  //
  // - Else if there is a User-Password AVP, then PAP is chosen as the
  // authentication type.
  //
  // - Else if there is a ARAP-Password AVP, then ARAP is chosen as
  // the authentication type.
  // 
  if (!aaRequest.UserName.IsSet())
    {
      AAA_LOG((LM_DEBUG, "[%N] No username.\n"));
      return false;
    }

  if (aaRequest.ChapAuth.IsSet())
    {
      if (!aaRequest.ChapChallenge.IsSet())
	{
	  AAA_LOG((LM_ERROR, "[%N] Missing CHAP-Challenge AVP.\n"));
	  return false;
	}
      authenticationInfo = boost::shared_ptr<CHAP_Info>
	(new CHAP_Info(aaRequest.UserName(),
		       aaRequest.ChapAuth(), 
		       aaRequest.ChapChallenge()));
      return true;
    }

  if (aaRequest.UserPassword.IsSet())
    {
      authenticationInfo = boost::shared_ptr<PAP_Info>
	(new PAP_Info(aaRequest.UserName(), aaRequest.UserPassword()));
      return true;
    }

  if (aaRequest.ArapPassword.IsSet())
    {
      if (!aaRequest.ArapChallengeResponse.IsSet())
	{
	  AAA_LOG((LM_ERROR, 
		       "[%N] Missing ARAP-Challenge-Response AVP.\n"));
	  return false;
	}
      authenticationInfo = boost::shared_ptr<ARAP_Info>
	(new ARAP_Info(aaRequest.UserName(), 
		       aaRequest.ArapPassword(), 
		       aaRequest.ArapChallengeResponse()));
      return true;
    }

  // No authentication information is found.
  AAA_LOG((LM_ERROR, "[%N] No authentication information AVP.\n"));
  return false;

 arap_multi_round_check:

  do {
    if (!aaRequest.FramedProtocol.IsSet() || aaRequest.FramedProtocol()!= 3)
      goto state_check;

    if (!aaRequest.ArapSecurity.IsSet() || !aaRequest.ArapSecurityData.IsSet())
      goto state_check;

    ARAP_Info &arapInfo = (ARAP_Info&)authenticationInfo;
    arapInfo.ArapSecurity() = aaRequest.ArapSecurity();
    arapInfo.ArapSecurityData() = aaRequest.ArapSecurityData();
    arapInfo.IsFirst() = false;
  } while(0);

 state_check:      
  // Validate State AVP.
  if (aaAnswer.State.IsSet()) // Non-initial state.
    {
      if (!aaRequest.State.IsSet() || 
	  !ValidateState(aaRequest.State(), aaAnswer.State()))
	{
	  AAA_LOG((LM_DEBUG, "[%N] Invalid State AVP.\n"));
	  return false;
	}
      else // Try to set initial state
	{
	  SetState(aaAnswer.State);
	}
    }

  return true;
}

void 
DiameterNasreqServerStateMachine::SignalContinue
(DiameterNasreqAuthenticationInfo &authInfo)
{
  AAA_LOG((LM_ERROR, "[%N] Continue received from application.\n"));
  if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_PAP)
    authenticationInfo = boost::shared_ptr<PAP_Info>
      (new PAP_Info((PAP_Info&)authInfo));
  else if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_CHAP)
    authenticationInfo = boost::shared_ptr<CHAP_Info>
      (new CHAP_Info((CHAP_Info&)authInfo));
  else if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_ARAP)
    authenticationInfo = boost::shared_ptr<ARAP_Info>
      (new ARAP_Info((ARAP_Info&)authInfo));
  Notify(EvRxAuthContinue);
}

void
DiameterNasreqServerStateMachine::SignalSuccess()
{
  AAA_LOG((LM_ERROR, "[%N] Success received from application.\n"));
  Notify(EvRxAuthSuccess);
}

void
DiameterNasreqServerStateMachine::SignalFailure()
{
  AAA_LOG((LM_ERROR, "[%N] Failure received from application.\n"));
  Notify(EvRxAuthFailure);
}

bool
DiameterNasreqServerStateMachine::Authorize()
{
  AAA_LOG((LM_DEBUG, "[%N] Authorizing AAREQUEST.\n"));
  AA_AnswerData& aaAnswer = aaAnswerData;
  AA_RequestData& aaRequest = aaRequestData;
  bool r;

  // Authorization of mandatory AVPs.

  // If AuthRequestType indicates authentication only, do nothing.
  if (aaAnswer.AuthRequestType() == AUTH_REQUEST_TYPE_AUTHENTICATION_ONLY)
    {
      AAA_LOG((LM_DEBUG, "[%N] Authorization totally success.\n"));
      return true;
    }
  
  if (!AuthorizeOriginHost(aaRequest.OriginHost()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize origin host.\n"));
      return false;
    }

  if (!AuthorizeOriginRealm(aaRequest.OriginRealm()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize origin realm.\n"));
      return false;
    }
  // Authorization of optional AVPs.

  if (!AuthorizeNasIdentifier(aaRequest.NasIdentifier()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize nas identifier.\n"));
      return false;
    }
  if (!AuthorizeNasIpAddress(aaRequest.NasIpAddress()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize nas ip address.\n"));
      return false;
    }
  if (!AuthorizeNasIpv6Address(aaRequest.NasIpAddress()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize nas ipv6 address.\n"));
      return false;
    }
  if (!AuthorizeNasPort(aaRequest.NasPort()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize nas port.\n"));
      return false;
    }
  if (!AuthorizeNasPortId(aaRequest.NasPortId()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize nas port id.\n"));
      return false;
    }
  if (!AuthorizeNasPortType(aaRequest.NasPortType()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize nas port type.\n"));
      return false;
    }
  if (!AuthorizeOriginStateId(aaRequest.OriginStateId()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize origin state id.\n"));
      return false;
    }

  if (!AuthorizeFilterId(aaAnswer.FilterId))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize filter id.\n"));
      return false;
    }

  if (aaRequest.PortLimit.IsSet())
    r = AuthorizePortLimit(aaRequest.PortLimit(), aaAnswer.PortLimit);
  else
    r = AuthorizePortLimit(aaAnswer.PortLimit);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize port limit.\n"));
      return false;
    }

  if (aaRequest.ServiceType.IsSet())
    r = AuthorizeServiceType(aaRequest.ServiceType(), aaAnswer.ServiceType);
  else
    r = AuthorizeServiceType(aaAnswer.ServiceType);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize service type.\n"));
      return false;
    }

  if (!AuthorizeClass(aaAnswer.Class))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize Class.\n"));
      return false;
    }

  if (!AuthorizeConfigurationToken(aaAnswer.ConfigurationToken))
    {
      AAA_LOG((LM_DEBUG, 
		   "[%N] Failed to authorize configuration token.\n"));
      return false;
    }

  if (!AuthorizeAcctInterimInterval(aaAnswer.AcctInterimInterval))
    {
      AAA_LOG((LM_DEBUG, 
		   "[%N] Failed to authorize acct interim interval.\n"));
      return false;
    }

  if (!AuthorizeIdleTimeout(aaAnswer.IdleTimeout))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize idle timeout.\n"));
      return false;
    }

  if (!AuthorizeAuthorizationLifetime(aaAnswer.AuthorizationLifetime))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize authz lifetime.\n"));
      return false;
    }

  if (!AuthorizeAuthGracePeriod(aaAnswer.AuthGracePeriod))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize auth grace period.\n"));
      return false;
    }

  if (!AuthorizeAuthSessionState(aaAnswer.AuthSessionState))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize auth session state.\n"));
      return false;
    }

  if (!AuthorizeReAuthRequestType(aaAnswer.ReAuthRequestType))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize reauth req. type.\n"));
      return false;
    }

  if (!AuthorizeSessionTimeout(aaAnswer.SessionTimeout))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize session timeout.\n"));
      return false;
    }

  AuthorizeCallbackId(aaAnswer.CallbackId);

  if (aaRequest.CallbackNumber.IsSet())
    r = AuthorizeCallbackNumber
      (aaRequest.CallbackNumber(), aaAnswer.CallbackNumber);
  else
    r = AuthorizeCallbackNumber(aaAnswer.CallbackNumber);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize callback num.\n"));
      return false;
    }

  if (!AuthorizeCallingStationId(aaRequest.CallingStationId()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to validate calling station id.\n"));
      return false;
    }

  if (!AuthorizeCalledStationId(aaRequest.CalledStationId()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to validate called station id.\n"));
      return false;
    }

  if (!AuthorizeOriginatingLineInfo(aaRequest.OriginatingLineInfo()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize orig. line info.\n"));
      return false;
    }

  if (!AuthorizeConnectInfo(aaRequest.ConnectInfo()))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize connec info.\n"));
      return false;
    }

  if (!AuthorizeFramedAppletalkLink(aaAnswer.FramedAppletalkLink))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize appletalk link.\n"));
      return false;
    }

  if (!AuthorizeFramedAppletalkZone(aaAnswer.FramedAppletalkZone))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize appletalk zone.\n"));
      return false;
    }

  if (!AuthorizeFramedAppletalkNetwork(aaAnswer.FramedAppletalkNetwork))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize appletalk network.\n"));
      return false;
    }

  if (!aaRequest.FramedCompression.IsSet())
    r = AuthorizeFramedCompression
      (aaRequest.FramedCompression(), aaAnswer.FramedCompression);
  else 
    r = AuthorizeFramedCompression(aaAnswer.FramedCompression);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed compression.\n"));
      return false;
    }

  if (aaRequest.FramedInterfaceId.IsSet())
    r = AuthorizeFramedInterfaceId
      (aaRequest.FramedInterfaceId(), aaAnswer.FramedInterfaceId);
  else
    r= AuthorizeFramedInterfaceId(aaAnswer.FramedInterfaceId);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed ifid.\n"));
      return false;
    }

  if (aaRequest.FramedIpAddress.IsSet())
    r = AuthorizeFramedIpAddress(aaRequest.FramedIpAddress(), 
				 aaAnswer.FramedIpAddress);
  else 
    r = AuthorizeFramedIpAddress(aaAnswer.FramedIpAddress);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed ipaddr.\n"));
      return false;
    }

  if (aaRequest.FramedIpv6Prefix.IsSet())
    r = AuthorizeFramedIpv6Prefix
      (aaRequest.FramedIpv6Prefix(), aaAnswer.FramedIpv6Prefix);
  else
    r = AuthorizeFramedIpv6Prefix(aaAnswer.FramedIpv6Prefix);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed ipv6prx.\n"));
      return false;
    }
      
  if (!AuthorizeFramedIpv6Pool(aaAnswer.FramedIpv6Pool))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed ipv6 pool.\n"));
      return false;
    }

  if (!AuthorizeFramedPool(aaAnswer.FramedPool))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed pool.\n"));
      return false;
    }

  if (!AuthorizeFramedIpv6Route(aaAnswer.FramedIpv6Route))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed ipv6 route.\n"));
      return false;
    }

  if (!AuthorizeFramedRoute(aaAnswer.FramedRoute))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed route.\n"));
      return false;
    }

  if (aaRequest.FramedIpNetmask.IsSet())
    r = AuthorizeFramedIpNetmask(aaRequest.FramedIpNetmask(), 
				 aaAnswer.FramedIpNetmask);
  else
    r = AuthorizeFramedIpNetmask(aaAnswer.FramedIpNetmask);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed ipmask.\n"));
      return false;
    }

  if (!AuthorizeFramedIpxNetwork(aaAnswer.FramedIpxNetwork))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed ipx network.\n"));
      return false;
    }

  if (aaRequest.FramedMtu.IsSet())
    r = AuthorizeFramedMtu(aaRequest.FramedMtu(), aaAnswer.FramedMtu);
  else
    r = AuthorizeFramedMtu(aaAnswer.FramedMtu);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed mtu.\n"));
      return false;
    }

  if (aaRequest.FramedProtocol.IsSet())
    r = AuthorizeFramedProtocol
      (aaRequest.FramedProtocol(), aaAnswer.FramedProtocol);
  else
    r = AuthorizeFramedProtocol(aaAnswer.FramedProtocol);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed proto.\n"));
      return false;
    }

  if (!AuthorizeFramedRouting(aaAnswer.FramedRouting))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize framed routing.\n"));
      return false;
    }

  if (aaRequest.LoginIpHost.IsSet())
    r = AuthorizeLoginIpHost
      (aaRequest.LoginIpHost(), aaAnswer.LoginIpHost);
  else
    r = AuthorizeLoginIpHost(aaAnswer.LoginIpHost);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize login ip host.\n"));
      return false;
    }

  if (aaRequest.LoginIpv6Host.IsSet())
    r = AuthorizeLoginIpv6Host
      (aaRequest.LoginIpv6Host(), aaAnswer.LoginIpv6Host);
  else
    r = AuthorizeLoginIpv6Host(aaAnswer.LoginIpv6Host);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize login ipv6 host.\n"));
      return false;
    }

  if (aaRequest.LoginLatGroup.IsSet())
    r = AuthorizeLoginLatGroup
      (aaRequest.LoginLatGroup(), aaAnswer.LoginLatGroup);
  else
    r = AuthorizeLoginLatGroup(aaAnswer.LoginLatGroup);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize login lat group.\n"));
      return false;
    }

  if (aaRequest.LoginLatNode.IsSet())
    r = AuthorizeLoginLatNode
      (aaRequest.LoginLatNode(), aaAnswer.LoginLatNode);
  else
    r = AuthorizeLoginLatNode(aaAnswer.LoginLatNode);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize login lat node.\n"));
      return false;
    }

  if (aaRequest.LoginLatPort.IsSet())
    r = AuthorizeLoginLatPort
      (aaRequest.LoginLatPort(), aaAnswer.LoginLatPort);
  else
    r = AuthorizeLoginLatPort(aaAnswer.LoginLatPort);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize login lat port.\n"));
      return false;
    }

  if (aaRequest.LoginLatService.IsSet())
    r = AuthorizeLoginLatService
      (aaRequest.LoginLatService(), aaAnswer.LoginLatService);
  else
    r = AuthorizeLoginLatService(aaAnswer.LoginLatService);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize login lat service.\n"));
      return false;
    }

  if (!AuthorizeLoginTcpPort(aaAnswer.LoginTcpPort))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize login tcp port.\n"));
      return false;
    }

  if (!AuthorizeNasFilterRule(aaAnswer.NasFilterRule))
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize nas filter rule.\n"));
      return false;
    }

  if (aaRequest.Tunneling.IsSet())
    r = AuthorizeTunneling(aaRequest.Tunneling(), aaAnswer.Tunneling);
  else
    r = AuthorizeTunneling(aaAnswer.Tunneling);
  if (!r)
    {
      AAA_LOG((LM_DEBUG, "[%N] Failed to authorize tunneling.\n"));
      return false;
    }

  AAA_LOG((LM_DEBUG, "[%N] Authorization totally success.\n"));
  authorizationDone = true;
  return true;
}

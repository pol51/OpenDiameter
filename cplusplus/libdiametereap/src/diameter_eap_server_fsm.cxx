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
// $Id: diameter_eap_server_fsm.cxx,v 1.28 2004/08/16 16:17:25 vfajardo Exp $

// diameter_eap_server_fsm.cxx:  EAP session handling
// Written by Yoshihiro Ohba

#include <ace/Singleton.h>
#include <ace/Atomic_Op_T.h>
#include "diameter_eap_server_session.hxx"
#include "diameter_eap_server_fsm.hxx"
#include "diameter_eap_parser.hxx"

class DiameterEapServerAction 
  : public AAA_Action<DiameterEapServerStateMachine>
{
  virtual void operator()(DiameterEapServerStateMachine&)=0;
 protected:
  DiameterEapServerAction() {}
  ~DiameterEapServerAction() {}
};

/// State table used by DiameterEapServerStateMachine.
class DiameterEapServerStateTable_S 
  : public AAA_StateTable<DiameterEapServerStateMachine>
{
  friend class 
  ACE_Singleton<DiameterEapServerStateTable_S, ACE_Recursive_Thread_Mutex>;

 private:
  class AcForwardEapResponse : public DiameterEapServerAction 
  {
    void operator()(DiameterEapServerStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, 
		   "[%N] forwarding EAP Response to authenticator.\n");
      sm.ForwardEapResponse(sm.DER().EapPayload());
    }
  };

  class AcCheckDER : public DiameterEapServerAction 
  {
    void operator()(DiameterEapServerStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Copying DER to DEA.\n");
      if (sm.CheckDER())
	sm.Event(DiameterEapServerStateMachine::EvSgValidDER);
      else
	sm.Event(DiameterEapServerStateMachine::EvSgInvalidDER);
    }
  };

  class AcSendDEAwithContinue : public DiameterEapServerAction 
  {
    void operator()(DiameterEapServerStateMachine& sm)
    {
      DiameterEapServerSession& session = sm.Session();
      AAA_LOG(LM_DEBUG, "[%N] Sending DEA with continue result-code.\n");

      // Set Result-Code to AAA_MULTI_ROUND_AUTH.
      DEA_Data& dea = sm.DEA();
      dea.ResultCode = AAA_MULTI_ROUND_AUTH;

      // Set reply message if needed.
      dea.ReplyMessage.Clear();
      sm.SetReplyMessage(dea.ReplyMessage, dea.ResultCode());

      dea.EapReissuedPayload.Clear();
      sm.SetReissuedEapPayload(dea.EapReissuedPayload);

      sm.SendDEA(); 

      // Update the session state.
      session.Update(AAASession::EVENT_AUTH_CONTINUE);
    }
  };

  class AcSendDEAwithSuccess : public DiameterEapServerAction 
  {
    void operator()(DiameterEapServerStateMachine& sm)
    {
      AAA_LOG(LM_DEBUG, "[%N] Sending DEA with a success Result-Code.\n");
      DiameterEapServerSession& session = sm.Session();
      DEA_Data& dea = sm.Session().DEA();

      dea.ResultCode = AAA_SUCCESS;
      sm.SendDEA(); 
      session.Update(AAASession::EVENT_AUTH_SUCCESS);
    }
  };

  class AcSendDEA_DueToAuthenticationFailure : public DiameterEapServerAction 
  {
    void operator()(DiameterEapServerStateMachine& sm)
    {
      DiameterEapServerSession& session = sm.Session();
      DEA_Data& dea = sm.Session().DEA();
      AAA_LOG(LM_DEBUG, 
		   "[%N] Sending DEA due to authentication failure.\n");

      dea.ResultCode = AAA_AUTHENTICATION_REJECTED;
      sm.SendDEA(); 
      session.Update(AAASession::EVENT_AUTH_FAILED);
    }
  };

  class AcSendDEA_DueToAuthorizationFailure : public DiameterEapServerAction 
  {
    void operator()(DiameterEapServerStateMachine& sm)
    {
      DiameterEapServerSession& session = sm.Session();
      DEA_Data& dea = sm.Session().DEA();
      AAA_LOG(LM_DEBUG, 
		   "[%N] Sending DEA due to authorization failure.\n");

      // Erase EAP payload since it may contain EAP Success message
      // when authentication is successful but authorization fails.
      dea.EapPayload.Clear();

      dea.ResultCode = AAA_AUTHORIZATION_REJECTED;
      sm.SendDEA(); 
      session.Update(AAASession::EVENT_AUTH_FAILED);
    }
  };

  class AcSendDEA_DueToInvalidDER : public DiameterEapServerAction 
  {
    void operator()(DiameterEapServerStateMachine& sm)
    {
      DiameterEapServerSession& session = sm.Session();
      DEA_Data& dea = sm.DEA();
      AAA_LOG(LM_DEBUG, "[%N] Sending DEA due to invalid DER.\n");

      dea.ResultCode = AAA_INVALID_AVP_VALUE;
      session.Update(AAASession::EVENT_AUTH_CONTINUE);
      sm.SendDEA(); 
    }
  };

  class AcAuthorize : public DiameterEapServerAction 
  {
    void operator()(DiameterEapServerStateMachine& sm)
    {
      if (sm.AuthorizationDone() || sm.Authorize())
	sm.Event(DiameterEapServerStateMachine::EvSgAuthorizationSuccess);
      else
	sm.Event(DiameterEapServerStateMachine::EvSgAuthorizationFailure);
    }
  };

  enum state {
    StInitialize,
    StWaitDER,
    StCheckDER,
    StWaitAuthorization,
    StAccepted,
    StRejected,
    StWaitEapMsg,
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

  AcSendDEAwithContinue acSendDEAwithContinue; 
  AcSendDEAwithSuccess acSendDEAwithSuccess; 
  AcSendDEA_DueToAuthenticationFailure acSendDEA_DueToAuthenticationFailure;
  AcSendDEA_DueToAuthorizationFailure acSendDEA_DueToAuthorizationFailure;
  AcSendDEA_DueToInvalidDER acSendDEA_DueToInvalidDER;
  AcForwardEapResponse acForwardEapResponse; 
  AcCheckDER acCheckDER; 
  AcAuthorize acAuthorize;

  // Defined as a leaf class
  DiameterEapServerStateTable_S() 
  {
    AddStateTableEntry(StInitialize, 
		       DiameterEapServerStateMachine::EvRxDER,
		       StCheckDER, acCheckDER);
    AddStateTableEntry(StInitialize, 
		       DiameterEapServerStateMachine::EvSgDisconnect, 
		       StTerminated);
    AddWildcardStateTableEntry(StInitialize, StTerminated);

    AddStateTableEntry(StCheckDER, 
		       DiameterEapServerStateMachine::EvSgValidDER, 
		       StWaitEapMsg, 
		       acForwardEapResponse);
    AddStateTableEntry(StCheckDER, 
		       DiameterEapServerStateMachine::EvSgInvalidDER, 
		       StRejected, acSendDEA_DueToInvalidDER);

    AddStateTableEntry(StWaitDER, 
		       DiameterEapServerStateMachine::EvRxDER,
		       StCheckDER, acCheckDER);
    AddStateTableEntry(StWaitDER,
		       DiameterEapServerStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StWaitDER, StTerminated);

    AddStateTableEntry(StWaitEapMsg,
		       DiameterEapServerStateMachine::EvRxEapRequest,
		       StWaitDER, acSendDEAwithContinue);
    AddStateTableEntry(StWaitEapMsg,
		       DiameterEapServerStateMachine::EvRxEapSuccess,
		       StWaitAuthorization, acAuthorize);
    AddStateTableEntry(StWaitEapMsg,
		       DiameterEapServerStateMachine::EvRxEapFailure,
		       StRejected, acSendDEA_DueToAuthenticationFailure);
    AddStateTableEntry(StWaitEapMsg,
		       DiameterEapServerStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StWaitEapMsg, StTerminated);

    AddStateTableEntry(StWaitAuthorization, 
		       DiameterEapServerStateMachine::EvSgAuthorizationSuccess,
		       StAccepted, acSendDEAwithSuccess);
    AddStateTableEntry(StWaitAuthorization, 
		       DiameterEapServerStateMachine::EvSgAuthorizationFailure,
		       StRejected, acSendDEA_DueToAuthorizationFailure);
    AddStateTableEntry(StWaitAuthorization,
		       DiameterEapServerStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StWaitAuthorization, StTerminated);

    // Re-authentication
    AddStateTableEntry(StAccepted, 
		       DiameterEapServerStateMachine::EvRxDER,
		       StCheckDER, acCheckDER);
    AddStateTableEntry(StAccepted,
		       DiameterEapServerStateMachine::EvSgDisconnect,
		       StTerminated);
    AddWildcardStateTableEntry(StAccepted, StTerminated);

    AddWildcardStateTableEntry(StRejected, StRejected);

    AddWildcardStateTableEntry(StTerminated, StTerminated);

    InitialState(StInitialize);
  }
  ~DiameterEapServerStateTable_S() {}
};

typedef 
ACE_Singleton<DiameterEapServerStateTable_S, ACE_Recursive_Thread_Mutex> 
DiameterEapServerStateTable;

DiameterEapServerStateMachine::DiameterEapServerStateMachine
(DiameterEapServerSession& s, DiameterJobHandle &h)
  : AAA_StateMachine<DiameterEapServerStateMachine>
  (*this, *DiameterEapServerStateTable::instance(), "AAA_EAP_CLIENT"),
    session(s),
    handle(h),
    authorizationDone(false)
{}

void 
DiameterEapServerStateMachine::SendDEA()
{
  AAAMessage msg;

  deaData.AuthApplicationId = EapApplicationId;

  DEA_Parser parser;
  parser.setAppData(&session.deaData);
  parser.setRawData(&msg);

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
    AAA_LOG(LM_DEBUG, "Sent DEA Message.\n");
  }
}

bool
DiameterEapServerStateMachine::CheckDER()
{
  DER_Data& der = derData;
  DEA_Data& dea = deaData;

  // Auth-Request-Type check.
  if (der.AuthRequestType() != AUTH_REQUEST_TYPE_AUTHENTICATION_ONLY &&
      der.AuthRequestType() != AUTH_REQUEST_TYPE_AUTHORIZE_AUTHENTICATE)
    {
      AAA_LOG(LM_ERROR, "[%N] Invalid auth request type.\n");
      return false;
    }
  dea.AuthRequestType = der.AuthRequestType;

  AuthorizeMultiRoundTimeOut(dea.MultiRoundTimeOut);

  if (!dea.AuthRequestType.IsSet())
    {
      AAA_LOG(LM_ERROR, "[%N] Failed to set auth request type.\n");
      return false;
    }

  // Class check.
  for (unsigned i=0; i<dea.Class.size(); i++)
    {
      bool found = false;
      for (unsigned j=0; j<der.Class.size(); j++)
	{
	  if (dea.Class[i] == der.Class[j])
	    found = true;
	}
      if (!found)
	{
	  AAA_LOG(LM_ERROR, "[%N] Invalid class\n.");
	  return false;
	}
    }

  // UserName check
  if (der.UserName.IsSet())
    {
      if (!ValidateUserName(der.UserName()))
	{
	  AAA_LOG(LM_DEBUG, "[%N] Failed to validate username.\n");
	  return false;
	}
      dea.UserName = der.UserName;
    }

  // State check.
  if (dea.State.IsSet()) // Non-initial state.
    {
      if (!der.State.IsSet() || !ValidateState(der.State(), dea.State()))
	{
	  AAA_LOG(LM_DEBUG, "[%N] Invalid State AVP.\n");
	  return false;
	}
      else // Try to set initial state
	{
	  SetState(dea.State);
	}
    }
  return true;
}

void 
DiameterEapServerStateMachine::SignalContinue(std::string &eapMsg)
{
  DEA_Data& dea = deaData;
  AAA_LOG(LM_ERROR, "[%N] EAP Request received from backend.\n");
  dea.EapPayload = eapMsg;
  Notify(EvRxEapRequest);
}

void
DiameterEapServerStateMachine::SignalSuccess(std::string &eapMsg)
{
  DEA_Data& dea = deaData;
  AAA_LOG(LM_ERROR, "[%N] EAP Success received from backend.\n");
  dea.EapPayload = eapMsg;
  Notify(EvRxEapSuccess);
}

void
DiameterEapServerStateMachine::SignalFailure(std::string &eapMsg)
{
  DEA_Data& dea = deaData;
  AAA_LOG(LM_ERROR, "[%N] EAP Failure received from backend.\n");
  dea.EapPayload = eapMsg;
  Notify(EvRxEapFailure);
}

bool
DiameterEapServerStateMachine::Authorize()
{
  AAA_LOG(LM_DEBUG, "[%N] Authorizing DER.\n");
  DEA_Data& dea = deaData;
  DER_Data& der = derData;
  bool r;

  // Authorization of mandatory AVPs.

  // If AuthRequestType indicates authentication only, do nothing.
  if (dea.AuthRequestType() == AUTH_REQUEST_TYPE_AUTHENTICATION_ONLY)
    {
      AAA_LOG(LM_DEBUG, "[%N] Authorization totally success.\n");
      return true;
    }
  
  if (!AuthorizeOriginHost(der.OriginHost()))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize origin host.\n");
      return false;
    }

  if (!AuthorizeOriginRealm(der.OriginRealm()))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize origin realm.\n");
      return false;
    }

  // Authorization of optional AVPs.
  if (der.Class.IsSet())
    r = AuthorizeClass(der.Class(), dea.Class);
  else
    r = AuthorizeClass(dea.Class);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize Class.\n");
      return false;
    }

  if (!AuthorizeConfigurationToken(dea.ConfigurationToken))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize conf. token.\n");
      return false;
    }

  if (!AuthorizeAcctInterimInterval(dea.AcctInterimInterval))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize auth interim intvl.\n");
      return false;
    }

  if (der.ServiceType.IsSet())
    r = AuthorizeServiceType(der.ServiceType(), dea.ServiceType);
  else
    r = AuthorizeServiceType(dea.ServiceType);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize service type.\n");
      return false;
    }

      
  if (!AuthorizeIdleTimeout(dea.IdleTimeout))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize idle timeout.\n");
      return false;
    }

  if (!AuthorizeAuthorizationLifetime(dea.AuthorizationLifetime))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize authz lifetime.\n");
      return false;
    }

  if (!AuthorizeAuthGracePeriod(dea.AuthGracePeriod))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize auth grace period.\n");
      return false;
    }

  if (!AuthorizeAuthSessionState(dea.AuthSessionState))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize auth session state.\n");
      return false;
    }

  if (!AuthorizeReAuthRequestType(dea.ReAuthRequestType))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize reauth req. type.\n");
      return false;
    }

  if (!AuthorizeSessionTimeout(dea.SessionTimeout))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize session timeout.\n");
      return false;
    }

  if (!AuthorizeFilterId(dea.FilterId))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize filter id.\n");
      return false;
    }

  if (der.PortLimit.IsSet())
    r = AuthorizePortLimit(der.PortLimit(), dea.PortLimit);
  else
    r = AuthorizePortLimit(dea.PortLimit);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize port limit.\n");
      return false;
    }

  AuthorizeCallbackId(dea.CallbackId);

  if (der.CallbackNumber.IsSet())
    AuthorizeCallbackNumber(der.CallbackNumber(), dea.CallbackNumber);
  else
    AuthorizeCallbackNumber(dea.CallbackNumber);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize callback num.\n");
      return false;
    }

  if (!AuthorizeFramedAppletalkLink(dea.FramedAppletalkLink))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize appletalk link.\n");
      return false;
    }

  if (!AuthorizeFramedAppletalkZone(dea.FramedAppletalkZone))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize appletalk zone.\n");
      return false;
    }

  if (!AuthorizeFramedAppletalkNetwork(dea.FramedAppletalkNetwork))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize appletalk network.\n");
      return false;
    }

  if (!der.FramedCompression.IsSet())
    r = AuthorizeFramedCompression
      (der.FramedCompression, dea.FramedCompression);
  else 
    r = AuthorizeFramedCompression(dea.FramedCompression);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed compression.\n");
      return false;
    }

  if (der.FramedInterfaceId.IsSet())
    r = AuthorizeFramedInterfaceId
      (der.FramedInterfaceId(), dea.FramedInterfaceId);
  else
    r= AuthorizeFramedInterfaceId(dea.FramedInterfaceId);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed ifid.\n");
      return false;
    }

  if (der.FramedIpAddress.IsSet())
    r = AuthorizeFramedIpAddress(der.FramedIpAddress(), dea.FramedIpAddress);
  else AuthorizeFramedIpAddress(dea.FramedIpAddress);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed ipaddr.\n");
      return false;
    }

  if (der.FramedIpv6Prefix.IsSet())
    r = AuthorizeFramedIpv6Prefix
      (der.FramedIpv6Prefix(), dea.FramedIpv6Prefix);
  else
    r = AuthorizeFramedIpv6Prefix(dea.FramedIpv6Prefix);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed ipv6prx.\n");
      return false;
    }
      
  if (!AuthorizeFramedIpv6Pool(dea.FramedIpv6Pool))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed ipv6 pool.\n");
      return false;
    }

  if (!AuthorizeFramedPool(dea.FramedPool))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed pool.\n");
      return false;
    }

  if (!AuthorizeFramedIpv6Route(dea.FramedIpv6Route))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed ipv6 route.\n");
      return false;
    }

  if (!AuthorizeFramedRoute(dea.FramedRoute))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed route.\n");
      return false;
    }

  if (der.FramedIpNetmask.IsSet())
    r = AuthorizeFramedIpNetmask(der.FramedIpNetmask(), dea.FramedIpNetmask);
  else
    r = AuthorizeFramedIpNetmask(dea.FramedIpNetmask);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed ipmask.\n");
      return false;
    }

  if (!AuthorizeFramedIpxNetwork(dea.FramedIpxNetwork))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed ipx network.\n");
      return false;
    }

  if (der.FramedMtu.IsSet())
    r = AuthorizeFramedMtu(der.FramedMtu(), dea.FramedMtu);
  else
    r = AuthorizeFramedMtu(dea.FramedMtu);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed mtu.\n");
      return false;
    }

  if (der.FramedProtocol.IsSet())
    r = AuthorizeFramedProtocol
      (der.FramedProtocol(), dea.FramedProtocol);
  else
    r = AuthorizeFramedProtocol(dea.FramedProtocol);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed proto.\n");
      return false;
    }

  if (!AuthorizeFramedRouting(dea.FramedRouting))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize framed routing.\n");
      return false;
    }

  if (!AuthorizeNasFilterRule(dea.NasFilterRule))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize nas filter rule.\n");
      return false;
    }

  if (der.Tunneling.IsSet())
    r = AuthorizeTunneling(der.Tunneling(), dea.Tunneling);
  else
    r = AuthorizeTunneling(dea.Tunneling);
  if (!r)
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize tunneling.\n");
      return false;
    }

  if (!AuthorizeEapMasterSessionKey(dea.EapMasterSessionKey))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize MSK.\n");
      return false;
    }

  if (!AuthorizeAccountingEapAuthMethod(dea.AccountingEapAuthMethod))
    {
      AAA_LOG(LM_DEBUG, "[%N] Failed to authorize acct eap method.\n");
      return false;
    }

  AAA_LOG(LM_DEBUG, "[%N] Authorization totally success.\n");
  authorizationDone = true;
  return true;
}

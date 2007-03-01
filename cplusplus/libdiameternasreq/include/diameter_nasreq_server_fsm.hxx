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

/* $Id: */
/* 
   diameter_eap_server_fsm.hxx
   Server Statemachine definition for Diameter NASREQ Application 
   Written by Yoshihiro Ohba
   Created April 8, 2004.
*/

#ifndef __NASREQ_SERVER_FSM_H__
#define __NASREQ_SERVER_FSM_H__

#include "framework.h"
#include "diameter_nasreq_parser.hxx"
#include "diameter_nasreq_authinfo.hxx"

#ifdef WIN32
   #if defined(DIAMETER_NASREQ_SERVER_EXPORT)
       #define DIAMETER_NASREQ_SERVER_EXPORTS __declspec(dllexport)
   #else
       #define DIAMETER_NASREQ_SERVER_EXPORTS __declspec(dllimport)
   #endif
#else
   #define DIAMETER_NASREQ_SERVER_EXPORTS
   #define DIAMETER_NASREQ_SERVER_EXPORTS
#endif

typedef AAA_JobHandle<AAA_GroupedJob> DiameterNasreqJobHandle;

class DiameterNasreqServerSession;

/// State machine for Diameter NASREQ server.  There are two types of
/// procedures a Diameter NASREQ server does: authentication and
/// authorization.  Authentication is an act to verify a client.
/// Authorization is an act to grant a service to a client.  In many
/// cases authentication occurs before authorization, but the order
/// can be reversed in some cases.  This server implementation
/// supports both orders.
///
/// Functions related to authentication have names starting "Validate"
/// (e.g., ValidateUserName).
///
/// Authorization is performed in the form of either validation of a
/// requested attribute value, assignment of an attribute value.  The
/// assignment of an attribute value may be performed as a
/// modification to a requested attribute value or as a new assignment
/// without a requested value.  It has many member functions for
/// authorization of attributes (i.e., AuthorizeXYZ).  The
/// authorization functions return a boolean value indicating whether
/// the authorization of the particular attribute succeeded or not.
/// Authorization functions for optionally set attributes MUST return
/// true when the optional attributes are not set.  When an attribute
/// is specified as const, the application is not allowed to modify
/// the attribute and expected to just judge on whether the given
/// attribute value is accepted or not.  When an attribute is not
/// specified as const, the application is allowed to modify the
/// attribute as well as judge on whether the given attribute value is
/// accepted or not.  Authorization functions with two arguments are
/// used for attribute that can be included in both DER and DEA.  DER
/// and DEA attributes are contained in the first argument (which is
/// const) and second argument, respectivily, where the former
/// attribute contains a requested value and the latter contains an
/// enforced value.  The former attributes are not enveloped with
/// DiameterScholorAttribute or DiameterVectorAttribute since they are
/// intended to be unmodified.
class DIAMETER_NASREQ_SERVER_EXPORTS DiameterNasreqServerStateMachine 
  : public AAA_StateMachine<DiameterNasreqServerStateMachine>,
    public AAA_EventQueueJob
{
 public:
  /// Constructor.
  DiameterNasreqServerStateMachine(DiameterNasreqServerSession& s,
				   DiameterNasreqJobHandle &h);

  ~DiameterNasreqServerStateMachine() 
  {
    handle.Job().Remove(this); 
  }

  enum {
    EvSgStart,
    EvRxAuthContinue,
    EvRxAuthSuccess,
    EvRxAuthFailure,
    EvSgAuthorizationSuccess,
    EvSgAuthorizationFailure,
    EvRxAA_Request,
    EvSgSessionTimeout,
    EvSgAuthLifetimeTimeout,
    EvSgAuthGracePeriodTimeout,
    EvSgTimeout,
    EvSgDisconnect,
    EvSgValidAA_Request,
    EvSgInvalidAA_Request
  };

  /// Store an event and notify the session.
  inline void Notify(AAA_Event ev) throw (int) {
    // Enqueue the event.
    if (AAA_EventQueueJob::Enqueue(ev) <= 0)
      Abort();

    if (handle.Job().Schedule(this) < 0)
      Abort();
  }

  /// This is used for obtaining the reference to the server session
  /// object.
  inline DiameterNasreqServerSession& Session() { return session; }

  /// This is used for aborting the state machine.  Usually called
  /// when Notify() fails.
  virtual void Abort()=0;

  /// This virtual function is called when authentication information
  /// such as CHAP challenge and response is passed to the application.
  virtual void ForwardAuthenticationInfo
  (DiameterNasreqAuthenticationInfo& authenticationInfo)=0;

  /// This virtual function is called when a continuation of the
  /// authentication is signaled from the application.
  void SignalContinue(DiameterNasreqAuthenticationInfo &authInfo);

  /// This virtual function is called when an authentication success
  /// is signaled from the application.
  void SignalSuccess();

  /// This virtual function is called when an authentication failure
  /// is signaled from the application.
  void SignalFailure();

  /// This is used for constructing and sending an AA-Answer.
  void SendAA_Answer();

  /// Check received AA-Request message.  It returns true when the
  /// message is valid.  Otherwise, it returns false.  When false is
  /// returned, it results in generating a AA-Answer message with the
  /// Result-Code value set to DIAMETER_INVALID_AVP_VALUE.
  bool CheckAA_Request();

  /// Authorization function.  This function can be called from the AA
  /// application so that authorization can be performed before
  /// completing authentication.  If this function is not called from
  /// the AA application, the NASREQ server will call this function
  /// when it receives an indication of successful authentication from
  /// the AA application.
  virtual bool Authorize();

  /// Check if authorization has been completed successfully.
  bool AuthorizationDone() { return authorizationDone; }

  inline AAA_JobData& JobData() { return *handle.Job().Data(); }

  template <class T> inline T& JobData(Type2Type<T>) 
  { return (T&)*handle.Job().Data(); }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeOriginHost
  (const diameter_identity_t &originHost)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeOriginRealm
  (const diameter_identity_t &originRealm)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeNasIdentifier
  (const DiameterScholarAttribute<diameter_utf8string_t> &nasIdentifier)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeNasIpAddress
  (const DiameterScholarAttribute<diameter_octetstring_t> &nasIpaddress)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeNasIpv6Address
  (const DiameterScholarAttribute<diameter_octetstring_t> &nasIpv6address)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeNasPort
  (const DiameterScholarAttribute<diameter_unsigned32_t> &nasPort)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeNasPortId
  (const DiameterScholarAttribute<diameter_utf8string_t> &nasPortId)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeNasPortType
  (const DiameterScholarAttribute<diameter_enumerated_t> &nasPortType)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeOriginStateId
  (const DiameterScholarAttribute<diameter_unsigned32_t> &nasOriginStateId)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFilterId
  (DiameterVectorAttribute<diameter_utf8string_t> &FilterId)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizePortLimit
  (DiameterScholarAttribute<diameter_unsigned32_t> &portLimit)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizePortLimit
  (const diameter_unsigned32_t &portLimit1,
   DiameterScholarAttribute<diameter_unsigned32_t> &portLimit2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeServiceType
  (DiameterScholarAttribute<diameter_enumerated_t> &serviceType)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeServiceType
  (const diameter_enumerated_t &serviceType1,
   DiameterScholarAttribute<diameter_enumerated_t> &serviceType2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeClass
  (DiameterVectorAttribute<diameter_octetstring_t> &Class)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeConfigurationToken
  (const DiameterVectorAttribute<diameter_octetstring_t> &configurationToken)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeAcctInterimInterval
  (DiameterScholarAttribute<diameter_unsigned32_t> &acctInterimInterval)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeIdleTimeout
  (DiameterScholarAttribute<diameter_unsigned32_t> &idleTimeout)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeAuthorizationLifetime
  (DiameterScholarAttribute<diameter_unsigned32_t> &authorizationLifetime)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeAuthGracePeriod
  (DiameterScholarAttribute<diameter_unsigned32_t> &authGracePeriod)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeAuthSessionState
  (DiameterScholarAttribute<diameter_enumerated_t> &authSessionState)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeReAuthRequestType
  (DiameterScholarAttribute<diameter_enumerated_t> &authReAuthRequestType)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeSessionTimeout
  (DiameterScholarAttribute<diameter_unsigned32_t> &authSessionTimeout)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeCallbackId
  (DiameterScholarAttribute<diameter_utf8string_t> &callbackId)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeCallbackNumber
  (DiameterScholarAttribute<diameter_utf8string_t> &callbackNumber)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeCallbackNumber
  (const diameter_utf8string_t &callbackNumber1,
   DiameterScholarAttribute<diameter_utf8string_t> &callbackNumber2)
  {
    return true;
  }
  
  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeCallingStationId
  (const diameter_utf8string_t &callingStationId)
  {
    return true;
  }
  
  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeCalledStationId
  (const diameter_utf8string_t &calledStationId)
  {
    return true;
  }
  
  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeOriginatingLineInfo
  (const DiameterScholarAttribute<diameter_octetstring_t> &originatingLineInfo)
  {
    return true;
  }

  virtual bool AuthorizeConnectInfo
  (const DiameterScholarAttribute<diameter_utf8string_t> &connectInfo)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedAppletalkLink
  (DiameterScholarAttribute<diameter_unsigned32_t> &framedAppletalkLink)
  {
    return true;
  }
  
  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedAppletalkZone
  (DiameterScholarAttribute<diameter_octetstring_t> &framedAppletalkZone)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedAppletalkNetwork
  (DiameterVectorAttribute<diameter_unsigned32_t> &framedAppletalkNetwork)
  {
    return true;
  }
  
  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedCompression
  (DiameterVectorAttribute<diameter_enumerated_t> &framedCompression)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedCompression
  (const std::vector<diameter_enumerated_t> &framedCompression1,
   DiameterVectorAttribute<diameter_enumerated_t> &framedCompression2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedInterfaceId
  (DiameterScholarAttribute<diameter_unsigned64_t> &framedInterfaceId)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedInterfaceId
  (const diameter_unsigned64_t &framedInterfaceId1,
   DiameterScholarAttribute<diameter_unsigned64_t> &framedInterfaceId2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedIpAddress
  (DiameterScholarAttribute<diameter_octetstring_t> &framedIpAddress)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedIpAddress
  (const diameter_octetstring_t &framedIpAddress1,
   DiameterScholarAttribute<diameter_octetstring_t> &framedIpAddress2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedIpv6Prefix
  (DiameterVectorAttribute<diameter_octetstring_t> &framedIpv6Prefix)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedIpv6Prefix
  (const std::vector<diameter_octetstring_t> &framedIpv6Prefix1,
   DiameterVectorAttribute<diameter_octetstring_t> &framedIpv6Prefix2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedIpv6Pool
  (DiameterScholarAttribute<diameter_octetstring_t> &framedIpv6Pool)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedPool
  (DiameterScholarAttribute<diameter_octetstring_t> &framedPool)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedIpv6Route
  (DiameterVectorAttribute<diameter_utf8string_t> &framedIpv6Route)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedRoute
  (DiameterVectorAttribute<diameter_utf8string_t> &framedRoute)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedIpNetmask
  (DiameterScholarAttribute<diameter_octetstring_t> &framedIpNetmask)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedIpNetmask
  (const diameter_octetstring_t &framedIpNetmask1,
   DiameterScholarAttribute<diameter_octetstring_t> &framedIpNetmask2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedIpxNetwork
  (DiameterScholarAttribute<diameter_utf8string_t> &framedIpxNetwork)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedMtu
  (DiameterScholarAttribute<diameter_unsigned32_t> &framedMtu)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedMtu
  (const diameter_unsigned32_t &framedMtu1,
   DiameterScholarAttribute<diameter_unsigned32_t> &framedMtu2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedProtocol
  (DiameterScholarAttribute<diameter_enumerated_t> &framedProtocol)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedProtocol
  (const diameter_enumerated_t &framedProtocol1,
   DiameterScholarAttribute<diameter_enumerated_t> &framedProtoco2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeFramedRouting
  (DiameterScholarAttribute<diameter_enumerated_t> &framedRouting)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginIpHost
  (DiameterVectorAttribute<diameter_octetstring_t> &loginIpHost)
  {
    return true;
  }
  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginIpHost
  (const std::vector<diameter_octetstring_t> &loginIpHost1,
   DiameterVectorAttribute<diameter_octetstring_t> &loginIpHost2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginIpv6Host
  (DiameterVectorAttribute<diameter_octetstring_t> &loginIpv6Host)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginIpv6Host
  (const std::vector<diameter_octetstring_t> &loginIpv6Host1,
   DiameterVectorAttribute<diameter_octetstring_t> &loginIpv6Host2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginLatGroup
  (DiameterScholarAttribute<diameter_octetstring_t> &loginLatGroup)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginLatGroup
  (const DiameterScholarAttribute<diameter_octetstring_t> &loginLatGroup1,
   DiameterScholarAttribute<diameter_octetstring_t> &loginLatGroup2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginLatNode
  (DiameterScholarAttribute<diameter_octetstring_t> &loginLatNode)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginLatNode
  (const DiameterScholarAttribute<diameter_octetstring_t> &loginLatNode1,
   DiameterScholarAttribute<diameter_octetstring_t> &loginLatNode2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginLatPort
  (DiameterScholarAttribute<diameter_octetstring_t> &loginLatPort)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginLatPort
  (const DiameterScholarAttribute<diameter_octetstring_t> &loginLatPort1,
   DiameterScholarAttribute<diameter_octetstring_t> &loginLatPort2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginLatService
  (DiameterScholarAttribute<diameter_octetstring_t> &loginLatService2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginLatService
  (const DiameterScholarAttribute<diameter_octetstring_t> &loginLatService1,
   DiameterScholarAttribute<diameter_octetstring_t> &loginLatService2)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeLoginTcpPort
  (DiameterScholarAttribute<diameter_unsigned32_t> &loginTcpPort)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeNasFilterRule
  (DiameterVectorAttribute<diameter_ipfilter_rule_t> &nasFilterRule)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeTunneling
  (DiameterVectorAttribute<tunneling_t> &tunneling)
  {
    return true;
  }

  /// An authorization function called from Authorize() function.
  virtual bool AuthorizeTunneling
  (const std::vector<tunneling_t> &tunneling1, 
   DiameterVectorAttribute<tunneling_t> &tunneling2)
  {
    return true;
  }

  /// The contents of the replyMessage should be generated depending
  /// on the value of the resultCode. 
  virtual void SetReplyMessage
  (DiameterVectorAttribute<diameter_utf8string_t> &replyMessage, 
   const diameter_unsigned32_t &resultCode)
  {}

  /// Validate Auth-Request-Type AVP.
  virtual bool ValidateAuthRequestType
  (const diameter_enumerated_t &authRequestType)
  {
    if (authRequestType != AUTH_REQUEST_TYPE_AUTHENTICATION_ONLY &&
	authRequestType != AUTH_REQUEST_TYPE_AUTHORIZE_AUTHENTICATE)
      return false;
    return true;
  }

  /// Validate State AVP in DER against State AVP in DEA.  The
  /// validation method is specific to application.
  virtual bool ValidateState
  (const diameter_octetstring_t &stateInDER,
   const diameter_octetstring_t &stateInDEA)
  {
    return false;
  }

  /// Used for setting State AVP in initial answer.  Do nothing by default.
  virtual void SetState(DiameterScholarAttribute<diameter_octetstring_t> &state)
  {
  }

  /// Used for setting ARAP-Security and ARAP-Security-Data AVPs.
  virtual void SetArapMultiExchange
  (DiameterScholarAttribute<diameter_unsigned32_t> &ArapSecurity,
   DiameterVectorAttribute<diameter_octetstring_t> &ArapSecurityData)
  {
  }

  inline DiameterNasreqAuthenticationInfo& AuthenticationInfo()
  { return *authenticationInfo; }

  inline AA_RequestData& AA_Request() { return aaRequestData; }
  inline AA_AnswerData& AA_Answer() { return aaAnswerData; }

 protected:

 private:
  /// Inherited from AAA_EventQueueJob.  Not used.
  int Schedule(AAA_Job*, size_t=1) { return (-1); }

  /// Inherited from AAA_EventQueueJob.
  inline int Serve()
  {
    if (!AAA_EventQueueJob::ExistBacklog())
      {
	AAA_LOG((LM_ERROR, "%N: no backlog to serve."));
	return 0;
      }

    // Obtain the event to execute.
    AAA_Event ev = 0;
    AAA_EventQueueJob::Dequeue(ev);

    bool existBacklog = AAA_EventQueueJob::ExistBacklog();

    // Execute it.
    Event(ev);
    return existBacklog ? 1 : 0;
  }

  DiameterNasreqServerSession& session;

  // Job handle.
  DiameterNasreqJobHandle handle;

  // authentication information.
  boost::shared_ptr<DiameterNasreqAuthenticationInfo> authenticationInfo;

  bool authorizationDone;

  // Request and Answer data
  AA_RequestData aaRequestData;
  AA_AnswerData aaAnswerData;

};

#endif

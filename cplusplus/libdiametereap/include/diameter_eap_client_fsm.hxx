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
   diameter_eap_client_fsm.hxx
   Client Statemachine definition for Diameter EAP Application 
   Written by Yoshihiro Ohba
   Created December 4, 2003.
*/

#ifndef __EAP_CLIENT_FSM_H__
#define __EAP_CLIENT_FSM_H__

#include "framework.h"
#include "diameter_eap_parser.hxx"

/*!
 * Windows specific export declarations
 */
#ifdef WIN32
   #if defined(DIAMETER_EAP_CLIENT_EXPORT)
       #define DIAMETER_EAP_CLIENT_EXPORTS __declspec(dllexport)
   #else
       #define DIAMETER_EAP_CLIENT_EXPORTS __declspec(dllimport)
   #endif
#else
   #define DIAMETER_EAP_CLIENT_EXPORTS
   #define DIAMETER_EAP_CLIENT_EXPORTS
#endif

typedef AAA_JobHandle<AAA_GroupedJob> DiameterJobHandle;

class DiameterEapClientSession;

/// State machine for Diameter EAP clients.  It has many member
/// functions for enforcement of attributes (i.e., EnforceXYZ) and 
/// for setting attributes (i.e., SetXYX).  Arguments of enforcement
/// functions are not enveloped with AAA_ScholorAttribute or
/// AAA_VectorAttributes since they are intended to be unmodified.
class DIAMETER_EAP_CLIENT_EXPORTS DiameterEapClientStateMachine 
  : public AAA_StateMachine<DiameterEapClientStateMachine>,
    public AAA_EventQueueJob
{
  friend class DiameterJobMultiplexor;

 public:
  /// Constructor.
  DiameterEapClientStateMachine(DiameterEapClientSession& s,
				DiameterJobHandle &h);

  ~DiameterEapClientStateMachine() 
  {
    handle.Job().Remove(this); 
  }

  enum {
    //    EvSgStartLocalMethod,
    EvSgStart=-1,
    EvRxEapResponse=-2,
    EvRxDEA=-3,
    EvRxAA_Answer=-4,
    EvSgSessionTimeout=-5,
    EvSgAuthLifetimeTimeout=-6,
    EvSgAuthGracePeriodTimeout=-7,
    EvSgTimeout=-8,
    EvSgDisconnect=-9
  };

  /// Reimplemented from AAA_StateMachine
  //  void Start() throw (AAA_Error);

  /// Store an event and notify the session.
  inline void Notify(AAA_Event ev) {
    // Enqueue the event.
    if (AAA_EventQueueJob::Enqueue(ev) <= 0)
      Abort();

    if (handle.Job().Schedule(this) < 0)
      Abort();
  }

  inline DiameterEapClientSession& Session() { return session; }

  /// This is used for aborting the state machine.  Usually called
  /// when Notify() fails.
  virtual void Abort()=0;

  /// This is used for constructing and sending a
  /// Diameter-EAP-Request.
  void SendDER();

  /// This virtual function is called when an EAP-Response message is
  /// passed to the EAP passthrough authenticator.
  virtual void SignalContinue(std::string &eapMsg)=0;

  /// This virtual function is called when an AAA success is signaled
  /// to the EAP passthrough authenticator. An EAP-Success message is
  /// contained in the argument if any.
  virtual void SignalSuccess(std::string &eapMsg)=0;

  /// This virtual function is called when an AAA success is signaled
  /// to the EAP passthrough authenticator. An EAP-Failure message is
  /// contained in the argument if any.
  virtual void SignalFailure(std::string &eapMsg)=0;

  /// This is called by application when EAP-Response is forwarded
  /// from passthrough EAP authenticator.
  void ForwardResponse(std::string &eapMsg);

  /// This virtual function is called when a RADIUS AA-Request needs
  /// to be sent as part of authorization.
  virtual void SendAA_Request() {}

  /// This virtual function is called when the current authorization
  /// lifetime is expired.  XXX: this should also be called when an Re-Auth
  /// Request is received from the server.
  virtual void SignalReauthentication()=0;

  /// This virtual function is called when the session lifetime or the
  /// auth grace period is expired, or a disconect event is received
  /// from libdiameter.
  virtual void SignalDisconnect()=0;

  inline AAA_JobData& JobData() { return *handle.Job().Data(); }

  template <class T> inline T& JobData(Type2Type<T>) 
  { return (T&)*handle.Job().Data(); }

  // Insert-AVP member functions.

  /// This function is used for setting Destination-Realm AVP
  /// contents.  
  virtual void SetDestinationRealm
  (DiameterScholarAttribute<diameter_utf8string_t> &destinationRealm)
  {
  }

  /// This function is used for setting Destination-Host AVP
  /// contents.  
  virtual void SetDestinationHost
  (DiameterScholarAttribute<diameter_utf8string_t> &destinationHost)
  {
  }

  /// This function is used for setting Auth-Request-Type AVP
  /// contents.  
  virtual void SetAuthRequestType
  (DiameterScholarAttribute<diameter_enumerated_t> &authRequestType)
  {
    authRequestType = AUTH_REQUEST_TYPE_AUTHORIZE_AUTHENTICATE;
  }

  /// This function is used for setting Nas-Port AVP contents.  
  virtual void SetNasPort
  (DiameterScholarAttribute<diameter_unsigned32_t> &nasPort)
  {
  }

  /// This function is used for setting Nas-Port-Id AVP contents.  
  virtual void SetNasPortId
  (DiameterScholarAttribute<diameter_utf8string_t> &nasPortId)
  {
  }

  /// This function is used for setting Origin-State-Id AVP contents.  
  virtual void SetOriginStateId
  (DiameterScholarAttribute<diameter_unsigned32_t> &originStateId)
  {
  }

  /// This function is used for setting Nas-Identifier AVP contents.  
  virtual void SetNasIdentifier
  (DiameterScholarAttribute<diameter_utf8string_t> &nasIdentifier)
  {
  }

  /// This function is used for setting Nas-IP-Address AVP contents.  
  virtual void SetNasIpAddress
  (DiameterScholarAttribute<diameter_octetstring_t> &nasIpAddress)
  {
  }

  /// This function is used for setting Nas-IPv6-Address AVP contents.  
  virtual void SetNasIpv6Address
  (DiameterScholarAttribute<diameter_octetstring_t> &nasIpv6Address)
  {
  }

  /// This function is used for setting User-Name AVP contents.
  virtual void
  SetUserName(DiameterScholarAttribute<diameter_utf8string_t> &userName)
  {
  }

  /// This function is used for setting Service-Type AVP contents.
  virtual void
  SetServiceType(DiameterScholarAttribute<diameter_enumerated_t> &serviceType)
  {
  }

  /// This function is used for setting Idle-Timeout AVP contents.
  virtual void
  SetIdleTimeout(DiameterScholarAttribute<diameter_unsigned32_t> &idleTimeout)
  {
  }

  /// This function is used for setting State AVP contents.
  virtual void
  SetState(DiameterScholarAttribute<diameter_octetstring_t> &state)
  {
  }

  /// This function is used for setting Authorization-Lifetime AVP
  /// contents.
  virtual void
  SetAuthorizationLifetime
  (DiameterScholarAttribute<diameter_unsigned32_t> &authorizationLifetime)
  {
  }

  /// This function is used for setting Auth-Grace-Period AVP
  /// contents.
  virtual void
  SetAuthGracePeriod
  (DiameterScholarAttribute<diameter_unsigned32_t> &authGracePeriod)
  {
  }

  /// This function is used for setting Auth-Session-State AVP
  /// contents.
  virtual void
  SetAuthSessionState
  (DiameterScholarAttribute<diameter_enumerated_t> &authSessionState)
  {
  }

  /// This function is used for setting Session-Timeout AVP contents.
  virtual void
  SetSessionTimeout
  (DiameterScholarAttribute<diameter_unsigned32_t> &sessionTimeout)
  {
  }

  /// This function is used for setting Class AVP contents.
  virtual void SetClass
  (DiameterVectorAttribute<diameter_octetstring_t> &classInDER)
  {
  }

  /// This function is used for setting Port-Limit AVP contents.
  virtual void
  SetPortLimit(DiameterScholarAttribute<diameter_unsigned32_t> &portLimit)
  {
  }

  /// This function is used for setting Callback-Number AVP contents.
  virtual void SetCallbackNumber
  (DiameterScholarAttribute<diameter_utf8string_t> &callbackNumber)
  {
  }

  /// This function is used for setting Called-Station-Id AVP
  /// contents.
  virtual void SetCalledStationId
  (DiameterScholarAttribute<diameter_utf8string_t> &calledStationId)
  {
  }

  /// This function is used for setting Called-Station-Id AVP
  /// contents.
  virtual void SetCallingStationId
  (DiameterScholarAttribute<diameter_utf8string_t> &callingStationId)
  {
  }

  /// This function is used for setting Originating-Line-Info AVP
  /// contents.
  virtual void SetOriginatingLineInfo
  (DiameterScholarAttribute<diameter_octetstring_t> &originatingLineInfo)
  {
  }

  /// This function is used for setting Connect-Info AVP contents.
  virtual void SetConnectInfo
  (DiameterScholarAttribute<diameter_utf8string_t> &connectInfo)
  {
  }

  /// This function is used for setting Framed-Compression AVP
  /// contents.
  virtual void SetFramedCompression
  (DiameterVectorAttribute<diameter_enumerated_t> &framedCompression)
  {
  }

  /// This function is used for setting Framed-Interface-Id AVP
  /// contents.
  virtual void SetFramedInterfaceId
  (DiameterScholarAttribute<diameter_unsigned64_t> &framedInterfaceId)
  {
  }

  /// This function is used for setting Framed-IP-Address AVP
  /// contents.
  virtual void SetFramedIpAddress
  (DiameterScholarAttribute<diameter_octetstring_t> &framedIpAddress)
  {
  }

  /// This function is used for setting Framed-IPv6-Prefix AVP
  /// contents.
  virtual void SetFramedIpv6Prefix
  (DiameterVectorAttribute<diameter_octetstring_t> &framedIpv6Prefix)
  {
  }

  /// This function is used for setting Framed-IP-Netmask AVP
  /// contents.
  virtual void SetFramedIpNetmask
  (DiameterScholarAttribute<diameter_octetstring_t> &framedIpNetmask)
  {
  }

  /// This function is used for setting Framed-MTU AVP contents.
  virtual void SetFramedMtu
  (DiameterScholarAttribute<diameter_unsigned32_t> &framedMtu)
  {
  }

  /// This function is used for setting Framed-Protocol AVP contents.
  virtual void SetFramedProtocol
  (DiameterScholarAttribute<diameter_enumerated_t> &framedProtocol)
  {
  }

  /// This function is used for setting Framed-Tunneling AVP contents.
  virtual void SetTunneling
  (DiameterVectorAttribute<tunneling_t> &tunneling)
  {
  }

  // Enforcement member functions.
  
  /// This function is used for enforcing Multi-Round-Time-Out AVP
  /// contents.
  virtual void EnforceMultiRoundTimeOut
  (const diameter_unsigned32_t &multiRoundTimeout)
  {
  }

  /// This function is used for enforcing Service-Type AVP contents.
  virtual void EnforceServiceType
  (const diameter_enumerated_t &serviceTypeInDEA)
  {
  }

  /// This function is used for enforcing Class AVP contents.
  virtual void EnforceClass
  (const std::vector<diameter_octetstring_t> &classInDEA)
  {
  }

  /// This function is used for enforcing Acct-Interim-Interval AVP
  /// contents.
  virtual void EnforceAcctInterimInterval
  (const diameter_unsigned32_t &acctInterimInterval)
  {
  }

  /// This function is used for enforcing Acct-Idle-Timeout AVP
  /// contents.
  virtual void EnforceIdleTimeout
  (const diameter_unsigned32_t &idleTimeout)
  {
  }

  /// This function is used for enforcing Authorization-Lifetime AVP
  /// contents.
  virtual void EnforceAuthorizationLifetime
  (const diameter_unsigned32_t &authorizationLifetime)
  {
  }

  /// This function is used for enforcing Auth-Grace-Period AVP
  /// contents.
  virtual void EnforceAuthGracePeriod
  (const diameter_unsigned32_t &authGracePeriod)
  {
  }

  /// This function is used for enforcing Auth-Session-State AVP
  /// contents.
  virtual void EnforceAuthSessionState
  (const diameter_enumerated_t &authSessionState)
  {
  }

  /// This function is used for enforcing ReAuth-Request-Type AVP
  /// contents.
  virtual void EnforceReAuthRequestType
  (const diameter_enumerated_t &reAuthRequestType)
  {
  }

  /// This function is used for enforcing Session-Timeout AVP
  /// contents.
  virtual void EnforceSessionTimeout
  (const diameter_unsigned32_t &sessionTimeout)
  {
  }

  /// This function is used for enforcing Filter-Id AVP contents.
  virtual void EnforceFilterId
  (const std::vector<diameter_utf8string_t> &filterId)
  {
  }
  
  /// This function is used for enforcing Port-Limit AVP contents.
  virtual void EnforcePortLimit
  (const diameter_unsigned32_t &portLimit)
  {
  }

  /// This function is used for enforcing Callback-Id AVP contents.
  virtual void EnforceCallbackId
  (const diameter_utf8string_t &callbackId)
  {
  }
  
  /// This function is used for enforcing Callback-Number AVP contents.
  virtual void EnforceCallbackNumber
  (const diameter_utf8string_t &callbackNumberInDEA)
  {
  }
  
  /// This function is used for enforcing Framed-Appletalk-Link AVP
  /// contents.
  virtual void EnforceFramedAppletalkLink
  (const diameter_unsigned32_t &framedAppletalkLink)
  {
  }
  
  /// This function is used for enforcing Framed-Appletalk-Zone AVP
  /// contents.
  virtual void EnforceFramedAppletalkZone
  (const diameter_octetstring_t &framedAppletalkZone)
  {
  }

  /// This function is used for enforcing Framed-Appletalk-Network AVP
  /// contents.
  virtual void EnforceFramedAppletalkNetwork
  (const std::vector<diameter_unsigned32_t> &framedAppletalkNetwork)
  {
  }
  
  /// This function is used for enforcing Framed-Compression AVP
  /// contents.
  virtual void EnforceFramedCompression
  (const std::vector<diameter_enumerated_t> &framedCompression)
  {
  }

  /// This function is used for enforcing Framed-Interface-Id AVP
  /// contents.
  virtual void EnforceFramedInterfaceId
  (const diameter_unsigned64_t &framedInterfaceId)
  {
  }
  
  /// This function is used for enforcing Framed-IP-Address AVP
  /// contents.
  virtual void EnforceFramedIpAddress
  (const diameter_octetstring_t &framedIpAddress)
  {
  }
  
  /// This function is used for enforcing Framed-IPv6-Prefix AVP
  /// contents.
  virtual void EnforceFramedIpv6Prefix
  (const std::vector<diameter_octetstring_t> &framedIpv6Prefix)
  {
  }

  /// This function is used for enforcing Framed-IPv6-Pool AVP
  /// contents.
  virtual void EnforceFramedIpv6Pool
  (const diameter_octetstring_t &framedIpv6Pool)
  {
  }

  /// This function is used for enforcing Framed-Pool AVP contents.
  virtual void EnforceFramedPool
  (const diameter_octetstring_t &framedPool)
  {
  }

  /// This function is used for enforcing Framed-IPv6-Route AVP contents.
  virtual void EnforceFramedIpv6Route
  (const std::vector<diameter_utf8string_t> &framedIpv6Route)
  {
  }

  /// This function is used for enforcing Framed-IP-Netmask AVP
  /// contents.
  virtual void EnforceFramedIpNetmask
  (const diameter_octetstring_t &framedIpNetmaskInDEA)
  {
  }

  /// This function is used for enforcing Framed-IPX-Network AVP
  /// contents.
  virtual void EnforceFramedIpxNetwork
  (const diameter_utf8string_t &framedIpxNetwork)
  {
  }

  /// This function is used for enforcing Framed-MTU AVP contents.
  virtual void EnforceFramedMtu
  (const diameter_unsigned32_t &framedMtu)
  {
  }

  /// This function is used for enforcing Framed-Protocol AVP
  /// contents.
  virtual void EnforceFramedProtocol
  (const diameter_enumerated_t &framedProtocol)
  {
  }

  /// This function is used for enforcing Framed-Routing AVP
  /// contents.
  virtual void EnforceFramedRouting
  (const diameter_enumerated_t &framedRouting)
  {
  }

  /// This function is used for enforcing Framed-NAS-Filter-Rule AVP
  /// contents.
  virtual void EnforceNasFilterRule
  (const std::vector<diameter_ipfilter_rule_t> &nasFilterRule)
  {
  }

  /// This function is used for enforcing Framed-Tunneling AVP
  /// contents.
  virtual void EnforceTunneling
  (const std::vector<tunneling_t> &tunneling)
  {
  }

  /// This function is used for enforcing EAP-Master-Session-Key AVP
  /// contents.
  virtual void EnforceEapMasterSessionKey
  (const diameter_octetstring_t &eapMasterSessionKey)
  {
  }

  /// This function is used for enforcing Accounting-EAP-Auth-Method
  /// AVP contents.
  virtual void EnforceAccountingEapAuthMethod
  (const std::vector<diameter_unsigned64_t> &accountingEapAuthMethodInDEA)
  {
  }
  
  DER_Data& DER() { return derData; }
  DEA_Data& DEA() { return deaData; }

 protected:

 private:
  /// Inherited from AAA_EventQueueJob.
  int Schedule(AAA_Job *job, size_t=1) { return (-1); }

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

  DiameterEapClientSession &session;

  /// Job handle.
  DiameterJobHandle handle;

  /// DER and DEA packet data.
  DER_Data derData;
  DEA_Data deaData;
  // XXX: AA_AnswerData aaAnswerData;

};

#endif

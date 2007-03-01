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

/* $Id: diameter_eap_parser.hxx,v 1.14 2006/05/02 14:25:15 vfajardo Exp $ */
/* 
   diameter_eap_parser.hxx
   Parser Data Structure in Diameter EAP Application 
   Written by Yoshihiro Ohba
   Created December 5, 2003.
*/

#ifndef __DIAMETER_EAP_PARSER_H__
#define __DIAMETER_EAP_PARSER_H__

#include <vector>
#include "diameter_parser.h"
#include "diameter_nasreq_parser.hxx"

const diameter_unsigned32_t EapApplicationId = 5;
const AAACommandCode EapCommandCode = 268;

#if 0
/// Definition for Tunneling AVP internal structure.
class tunneling_t
{
 public:

  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (TunnelType.IsSet())
      {
	c = cm.acquire("Tunnel-Type");
	TunnelType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
	cl.add(c);
      }
    if (TunnelMediumType.IsSet())
      {
	c = cm.acquire("Tunnel-Medium-Type");
	TunnelMediumType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
	cl.add(c);
      }
    if (TunnelClientEndpoint.IsSet())
      {
	c = cm.acquire("Tunnel-Client-Endpoint");
	TunnelClientEndpoint.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
	cl.add(c);
      }
    if (TunnelServerEndpoint.IsSet())
      {
	c = cm.acquire("Tunnel-Server-Endpoint");
	TunnelServerEndpoint.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
	cl.add(c);
      }
    if (TunnelPreference.IsSet())
      {
	c = cm.acquire("Tunnel-Preference");
	TunnelPreference.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
    if (TunnelClientAuthId.IsSet())
      {
	c = cm.acquire("Tunnel-Client-Auth-Id");
	TunnelClientAuthId.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
    if (TunnelServerAuthId.IsSet())
      {
	c = cm.acquire("Tunnel-Server-Auth-Id");
	TunnelServerAuthId.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
	cl.add(c);
      }
    if (TunnelPassword.IsSet())
      {
	c = cm.acquire("Tunnel-Password");
	TunnelPassword.CopyTo(*c, AAA_AVP_STRING_TYPE);
	cl.add(c);
      }
    if (TunnelPrivateGroupId.IsSet())
      {
	c = cm.acquire("Tunnel-Private-Group-Id");
	TunnelPrivateGroupId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
	  
    if ((c = cl.search("Tunnel-Type")))
      {
	TunnelType.CopyFrom(*c);
      }
    if ((c = cl.search("Tunnel-Medium-Type")))
      {
	TunnelMediumType.CopyFrom(*c);
      }
    if ((c = cl.search("Tunnel-Client-Endpoint")))
      {
	TunnelClientEndpoint.CopyFrom(*c);
      }
    if ((c = cl.search("Tunnel-Server-Endpoint")))
      {
	TunnelServerEndpoint.CopyFrom(*c);
      }
    if ((c = cl.search("Tunnel-Preference")))
      {
	TunnelPreference.CopyFrom(*c);
      }
    if ((c = cl.search("Tunnel-Client-Auth-Id")))
      {
	TunnelClientAuthId.CopyFrom(*c);
      }
    if ((c = cl.search("Tunnel-Server-Auth-Id")))
      {
	TunnelServerAuthId.CopyFrom(*c);
      }
    if ((c = cl.search("Tunnel-Password")))
      {
	TunnelPassword.CopyFrom(*c);
      }
    if ((c = cl.search("Tunnel-Private-Group-Id")))
      {
	TunnelPrivateGroupId.CopyFrom(*c);
      }
  }
  // Required AVPs
  DiameterScholarAttribute<diameter_enumerated_t> TunnelType;
  DiameterScholarAttribute<diameter_enumerated_t> TunnelMediumType;
  DiameterScholarAttribute<diameter_utf8string_t> TunnelClientEndpoint;
  DiameterScholarAttribute<diameter_utf8string_t> TunnelServerEndpoint;
  // Optional AVPs
  DiameterScholarAttribute<diameter_unsigned32_t> TunnelPreference;
  DiameterScholarAttribute<diameter_unsigned32_t> TunnelClientAuthId;
  DiameterScholarAttribute<diameter_unsigned32_t> TunnelServerAuthId;
  DiameterScholarAttribute<diameter_octetstring_t> TunnelPassword;
  DiameterScholarAttribute<diameter_utf8string_t> TunnelPrivateGroupId;
};

/// Definition for Proxy-Info AVP internal structure.
class proxyinfo_t
{
 public:
  void CopyTo(AAAAvpContainerList &cl)
  {
    DiameterAvpContainerManager cm;
    AAAAvpContainer *c;
    if (ProxyHost.IsSet())
      {
	c = cm.acquire("Proxy-Host");
	ProxyHost.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
	cl.add(c);
      }
    if (ProxyState.IsSet())
      {
	c = cm.acquire("Proxy-State");
	ProxyState.CopyTo(*c, AAA_AVP_STRING_TYPE);
	cl.add(c);
      }
    if (Avp.IsSet())
      {
	c = cm.acquire("AVP");
	Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
	cl.add(c);
      }
  }
  void CopyFrom(AAAAvpContainerList &cl)
  {
    AAAAvpContainer *c;
    if ((c = cl.search("Proxy-Host")))
      {
	ProxyHost.CopyFrom(*c);
      }
    if ((c = cl.search("Proxy-State")))
      {
	ProxyState.CopyFrom(*c);
      }
    if ((c = cl.search("AVP")))
      {
	Avp.CopyFrom(*c);
      }
  }	  
  // Required AVPs
  DiameterScholarAttribute<diameter_identity_t> ProxyHost;
  DiameterScholarAttribute<diameter_octetstring_t> ProxyState;
  // Optional AVPs
  DiameterVectorAttribute<diameter_avp_t> Avp;
};
#endif

/// Definition for DER message contents internal structure.
class DER_Data
{
 public:
  DER_Data()
  {
    EapPayload.Set(std::string());
  }

  void Clear()
  {
    SessionId.Clear();
    AuthApplicationId.Clear();
    OriginHost.Clear();
    OriginRealm.Clear();
    DestinationRealm.Clear();
    AuthRequestType.Clear();
    NasPort.Clear();
    NasPortId.Clear();
    OriginStateId.Clear();
    DestinationHost.Clear();
    NasIdentifier.Clear();
    NasIpAddress.Clear();
    NasIpv6Address.Clear();
    NasPortType.Clear();
    PortLimit.Clear();
    UserName.Clear();
    EapPayload.Clear();
    ServiceType.Clear();
    IdleTimeout.Clear();
    State.Clear();
    AuthorizationLifetime.Clear();
    AuthGracePeriod.Clear();
    AuthSessionState.Clear();
    SessionTimeout.Clear();
    CallbackNumber.Clear();
    CalledStationId.Clear();
    CallingStationId.Clear();
    Class.Clear();
    OriginatingLineInfo.Clear();
    ConnectInfo.Clear();
    FramedCompression.Clear();
    FramedInterfaceId.Clear();
    FramedIpAddress.Clear();
    FramedIpv6Prefix.Clear();
    FramedIpNetmask.Clear();
    FramedMtu.Clear();
    FramedProtocol.Clear();
    Tunneling.Clear();
    ProxyInfo.Clear();
    RouteRecord.Clear();
    Avp.Clear();
  }

  /// DER AVPs
  DiameterScholarAttribute<diameter_utf8string_t> SessionId;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthApplicationId;
  DiameterScholarAttribute<diameter_identity_t>  OriginHost;
  DiameterScholarAttribute<diameter_identity_t>  OriginRealm;
  DiameterScholarAttribute<diameter_identity_t>  DestinationRealm;
  /* In RFC3588:

   "8.7.  Auth-Request-Type AVP
                                                                                
   The Auth-Request-Type AVP (AVP Code 274) is of type Enumerated and is
   included in application-specific auth requests to inform the peers
   whether a user is to be authenticated only, authorized only or both.
   Note any value other than both MAY cause RADIUS interoperability
   issues.  The following values are defined:
                                                                                
   AUTHENTICATE_ONLY          1
      The request being sent is for authentication only, and MUST
      contain the relevant application specific authentication AVPs that
      are needed by the Diameter server to authenticate the user.

   AUTHORIZE_ONLY             2
      The request being sent is for authorization only, and MUST contain
      the application specific authorization AVPs that are necessary to
      identify the service being requested/offered.

   AUTHORIZE_AUTHENTICATE     3
      The request contains a request for both authentication and
      authorization.  The request MUST include both the relevant
      application specific authentication information, and authorization
      information necessary to identify the service being
      requested/offered."

  */
  DiameterScholarAttribute<diameter_enumerated_t> AuthRequestType;
  DiameterScholarAttribute<diameter_unsigned32_t> NasPort;
  DiameterScholarAttribute<diameter_utf8string_t> NasPortId;
  DiameterScholarAttribute<diameter_unsigned32_t> OriginStateId;
  DiameterScholarAttribute<diameter_identity_t>  DestinationHost;
  DiameterScholarAttribute<diameter_utf8string_t> NasIdentifier;
  DiameterScholarAttribute<diameter_octetstring_t> NasIpAddress;
  DiameterScholarAttribute<diameter_octetstring_t> NasIpv6Address;
  DiameterScholarAttribute<diameter_enumerated_t> NasPortType;
  /* In draft-ietf-aaa-diameter-nasreq-13.txt:

   "6.5.  Port-Limit AVP
                                                                                
   The Port-Limit AVP (AVP Code 62) is of type Unsigned32 and sets the
   maximum number of ports to be provided to the user by the NAS.  It
   MAY be used in an authentication and/or authorization request as a
   hint to the server that multilink PPP [PPPMP] service is desired, but
   the server is not required to honor the hint in the corresponding
   response."

  */
  DiameterScholarAttribute<diameter_unsigned32_t> PortLimit;
  DiameterScholarAttribute<diameter_utf8string_t> UserName;
  DiameterScholarAttribute<diameter_octetstring_t> EapPayload;
  /* In RFC2865:

   "5.6.  Service-Type
                                                                                
   Description
                                                                                
      This Attribute indicates the type of service the user has
      requested, or the type of service to be provided.  It MAY be used
      in both Access-Request and Access-Accept packets.  A NAS is not
      required to implement all of these service types, and MUST treat
      unknown or unsupported Service-Types as though an Access-Reject
      had been received instead."

   Value

      The Value field is four octets.
                                                                                
       1      Login
       2      Framed
       3      Callback Login
       4      Callback Framed
       5      Outbound
       6      Administrative
       7      NAS Prompt
       8      Authenticate Only
       9      Callback NAS Prompt
      10      Call Check
      11      Callback Administrative

  */
  DiameterScholarAttribute<diameter_enumerated_t> ServiceType;
  DiameterScholarAttribute<diameter_unsigned32_t> IdleTimeout;
  /* In Section 5.24 of RFC 2865:

   "5.24.  State
                                                                                
   Description
                                                                                
      This Attribute is available to be sent by the server to the client
      in an Access-Challenge and MUST be sent unmodified from the client
      to the server in the new Access-Request reply to that challenge,
      if any.
                                                                                
      This Attribute is available to be sent by the server to the client
      in an Access-Accept that also includes a Termination-Action
      Attribute with the value of RADIUS-Request.  If the NAS performs
      the Termination-Action by sending a new Access-Request upon
      termination of the current session, it MUST include the State
      attribute unchanged in that Access-Request.
                                                                                
      In either usage, the client MUST NOT interpret the attribute
      locally.  A packet must have only zero or one State Attribute.
      Usage of the State Attribute is implementation dependent."

  */
  DiameterScholarAttribute<diameter_octetstring_t> State;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthorizationLifetime;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthGracePeriod;
  /* In Section 8 of RFC3588:

  "An access device that does not expect to send a re-authorization or
  a session termination request to the server MAY include the Auth-
  Session-State AVP with the value set to NO_STATE_MAINTAINED as a
  hint to the server.  If the server accepts the hint, it agrees that
  since no session termination message will be received once service
  to the user is terminated, it cannot maintain state for the session.
  If the answer message from the server contains a different value in
  the Auth-Session-State AVP (or the default value if the AVP is
  absent), the access device MUST follow the server's directives.
  Note that the value NO_STATE_MAINTAINED MUST NOT be set in
  subsequent re- authorization requests and answers."

  In Section 8.1 of RFC3588:

  "There are four different authorization session state machines
  supported in the Diameter base protocol.  The first two describe a
  session in which the server is maintaining session state, indicated
  by the value of the Auth-Session-State AVP (or its absence).  One
  describes the session from a client perspective, the other from a
  server perspective.  The second two state machines are used when the
  server does not maintain session state.  Here again, one describes
  the session from a client perspective, the other from a server
  perspective."

  In Section 8.11 of RFC3588:

  "8.11.  Auth-Session-State AVP
                                                                                
  The Auth-Session-State AVP (AVP Code 277) is of type Enumerated and
  specifies whether state is maintained for a particular session.  The
  client MAY include this AVP in requests as a hint to the server, but
  the value in the server's answer message is binding.  The following
  values are supported:
                                                                                
  STATE_MAINTAINED              0
      This value is used to specify that session state is being
      maintained, and the access device MUST issue a session termination
      message when service to the user is terminated.  This is the
      default value.
                                                                                
  NO_STATE_MAINTAINED           1
      This value is used to specify that no session termination messages
      will be sent by the access device upon expiration of the
      Authorization-Lifetime."

    */
  DiameterScholarAttribute<diameter_enumerated_t> AuthSessionState;
  DiameterScholarAttribute<diameter_unsigned32_t> SessionTimeout;
  /* In draft-ietf-aaa-diameter-nasreq-13.txt:

   "6.2.  Callback-Number AVP
                                                                                
   The Callback-Number AVP (AVP Code 19) is of type UTF8String, and
   contains a dialing string to be used for callback.  It MAY be used in
   an authentication and/or authorization request as a hint to the
   server that a Callback service is desired, but the server is not
   required to honor the hint in the corresponding response.
                                                                                
   The codification of the range of allowed usage of this field is
   outside the scope of this specification."
                                                                                
  */
  DiameterScholarAttribute<diameter_utf8string_t> CallbackNumber;
  DiameterScholarAttribute<diameter_utf8string_t> CalledStationId;
  DiameterScholarAttribute<diameter_utf8string_t> CallingStationId;
  /* RFC3588 

  8.20.  Class AVP
                                                                                
  The Class AVP (AVP Code 25) is of type OctetString and is used to by
  Diameter servers to return state information to the access device.
  When one or more Class AVPs are present in application-specific
  authorization answer messages, they MUST be present in subsequent
  re-authorization, session termination and accounting messages.
  Class AVPs found in a re-authorization answer message override the
  ones found in any previous authorization answer message.  Diameter
  server implementations SHOULD NOT return Class AVPs that require
  more than 4096 bytes of storage on the Diameter client.  A Diameter
  client that receives Class AVPs whose size exceeds local available
  storage MUST terminate the session.

  */
  DiameterVectorAttribute<diameter_octetstring_t> Class;
  DiameterScholarAttribute<diameter_octetstring_t> OriginatingLineInfo;
  DiameterScholarAttribute<diameter_utf8string_t> ConnectInfo;
  DiameterVectorAttribute<diameter_enumerated_t> FramedCompression;
  DiameterScholarAttribute<diameter_unsigned64_t> FramedInterfaceId;
  DiameterScholarAttribute<diameter_octetstring_t> FramedIpAddress;
  DiameterVectorAttribute<diameter_octetstring_t> FramedIpv6Prefix;
  DiameterScholarAttribute<diameter_octetstring_t> FramedIpNetmask;
  /* In draft-ietf-aaa-diameter-nasreq-13.txt:

   "6.9.3.  Framed-MTU AVP
                                                                                
   The Framed-MTU AVP (AVP Code 12) is of type Unsigned32 and contains
   the Maximum Transmission Unit to be configured for the user, when it
   is not negotiated by some other means (such as PPP). This AVP SHOULD
   only be present in authorization responses. The MTU value MUST be in
   the range of 64 and 65535."

  */
  DiameterScholarAttribute<diameter_unsigned32_t> FramedMtu;
  /* In draft-ietf-aaa-diameter-nasreq-13.txt:

   "6.9.1.  Framed-Protocol AVP
                                                                                
   The Framed-Protocol AVP (AVP Code 7) is of type Enumerated and
   contains the framing to be used for framed access. This AVP MAY be
   present in both requests and responses. The supported values are
   listed in [RADIUSTypes].  The following list is informational:

       1  PPP
       2  SLIP
       3  AppleTalk Remote Access Protocol (ARAP)
       4  Gandalf proprietary SingleLink/MultiLink protocol
       5  Xylogics proprietary IPX/SLIP
       6  X.75 Synchronous"

  */
  DiameterScholarAttribute<diameter_enumerated_t> FramedProtocol;
  DiameterGroupedVectorAttribute<tunneling_t> Tunneling;
  DiameterGroupedVectorAttribute<proxyinfo_t> ProxyInfo;
  DiameterVectorAttribute<diameter_identity_t> RouteRecord;
  DiameterVectorAttribute<diameter_avp_t> Avp;
};

/// Definition for DEA message contents internal structure.
class DEA_Data
{
 public:
  DEA_Data() {}

  void Clear()
  {
    UserName.Clear();
    EapPayload.Clear();
    EapReissuedPayload.Clear();
    EapMasterSessionKey.Clear();
    MultiRoundTimeOut.Clear();
    AccountingEapAuthMethod.Clear();
    OriginStateId.Clear();
    PortLimit.Clear();
    ServiceType.Clear();
    IdleTimeout.Clear();
    State.Clear();
    Class.Clear();
    AuthorizationLifetime.Clear();
    AuthGracePeriod.Clear();
    AuthSessionState.Clear();
    SessionTimeout.Clear();
    CallbackNumber.Clear();
    FramedAppletalkLink.Clear();
    FramedAppletalkNetwork.Clear();
    FramedAppletalkZone.Clear();
    FramedCompression.Clear();
    FramedInterfaceId.Clear();
    FramedIpAddress.Clear();
    FramedIpv6Prefix.Clear();
    FramedIpv6Pool.Clear();
    FramedIpv6Route.Clear();
    FramedIpNetmask.Clear();
    FramedRoute.Clear();
    FramedPool.Clear();
    FramedIpxNetwork.Clear();
    FramedMtu.Clear();
    FramedProtocol.Clear();
    FramedRouting.Clear();
    NasFilterRule.Clear();
    Tunneling.Clear();
    RedirectHost.Clear();
    RedirectHostUsage.Clear();
    RedirectMaxCacheTime.Clear();
    ProxyInfo.Clear();
    Avp.Clear();
  }

  /// DEA AVPs
  DiameterScholarAttribute<diameter_utf8string_t> SessionId;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthApplicationId;
  DiameterScholarAttribute<diameter_enumerated_t> AuthRequestType;
  DiameterScholarAttribute<diameter_identity_t>  OriginHost;
  DiameterScholarAttribute<diameter_identity_t>  OriginRealm;
  DiameterScholarAttribute<diameter_utf8string_t> UserName;
  DiameterScholarAttribute<diameter_octetstring_t> EapPayload;
  DiameterScholarAttribute<diameter_octetstring_t> EapReissuedPayload;
  DiameterScholarAttribute<diameter_octetstring_t> EapMasterSessionKey;
  DiameterVectorAttribute<diameter_unsigned64_t> AccountingEapAuthMethod;
  DiameterScholarAttribute<diameter_unsigned32_t> OriginStateId;
  DiameterScholarAttribute<diameter_unsigned32_t> PortLimit;
  DiameterScholarAttribute<diameter_enumerated_t> ServiceType;
  DiameterScholarAttribute<diameter_unsigned32_t> IdleTimeout;
  DiameterScholarAttribute<diameter_octetstring_t> State;
  DiameterVectorAttribute<diameter_octetstring_t> Class;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthorizationLifetime;
  DiameterScholarAttribute<diameter_unsigned32_t> AuthGracePeriod;
  DiameterScholarAttribute<diameter_enumerated_t> AuthSessionState;
  DiameterScholarAttribute<diameter_unsigned32_t> SessionTimeout;
  DiameterScholarAttribute<diameter_utf8string_t> CallbackNumber;
  DiameterVectorAttribute<diameter_enumerated_t> FramedCompression;
  DiameterScholarAttribute<diameter_unsigned64_t> FramedInterfaceId;
  DiameterScholarAttribute<diameter_octetstring_t> FramedIpAddress;
  DiameterVectorAttribute<diameter_octetstring_t> FramedIpv6Prefix;
  DiameterScholarAttribute<diameter_octetstring_t> FramedIpNetmask;
  DiameterScholarAttribute<diameter_unsigned32_t> FramedMtu;
  DiameterScholarAttribute<diameter_enumerated_t> FramedProtocol;
  DiameterVectorAttribute<diameter_ipfilter_rule_t> NasFilterRule;
  DiameterGroupedVectorAttribute<tunneling_t> Tunneling;
  DiameterGroupedVectorAttribute<proxyinfo_t> ProxyInfo;
  DiameterVectorAttribute<diameter_avp_t> Avp;
  DiameterScholarAttribute<diameter_unsigned32_t> ResultCode; // ++
  // -- std::vector<diameter_identity_t>  DestinationRealm;
  // -- std::vector<diameter_unsigned32_t> NasPort;
  // -- std::vector<diameter_utf8string_t> NasPortId;
  // -- std::vector<diameter_identity_t>  DestinationHost;
  // -- std::vector<diameter_utf8string_t> NasIdentifier;
  // -- std::vector<diameter_octetstring_t> NasIpAddress;
  // -- std::vector<diameter_octetstring_t> NasIpv6Address;
  // -- std::vector<diameter_enumerated_t> NasPortType;
  DiameterScholarAttribute<diameter_unsigned32_t> MultiRoundTimeOut; // ++ 

  /* RFC2869

   5.12.  Configuration-Token
                                                                                
   Description
                                                                                
      This attribute is for use in large distributed authentication
      networks based on proxy.  It is sent from a RADIUS Proxy Server to
      a RADIUS Proxy Client in an Access-Accept to indicate a type of
      user profile to be used.  It should not be sent to a NAS.

  */
  DiameterVectorAttribute<diameter_octetstring_t> ConfigurationToken; // ++


  DiameterScholarAttribute<diameter_unsigned32_t> AcctInterimInterval; // ++ 
  DiameterScholarAttribute<diameter_utf8string_t> ErrorMessage; // ++
  DiameterScholarAttribute<diameter_identity_t>  ErrorReportingHost; // ++
  DiameterScholarAttribute<diameter_enumerated_t> ReAuthRequestType; // ++

  /* In draft-ietf-aaa-diameter-nasreq-13.txt:

   "4.9.  Reply-Message AVP
                                                                                
   The Reply-Message AVP (AVP Code 18) is of type UTF8String, and
   contains text which MAY be displayed to the user.  When used in an
   AA-Answer message with a successful Result-Code AVP it indicates a
   success message. When found in the same message with a Result-Code
   other than DIAMETER_SUCCESS it contains a failure message.
                                                                                
   The Reply-Message AVP MAY indicate a dialog message to prompt the
   user before another AA-Request attempt. When used in an AA-Answer, it
   MAY indicate a dialog message to prompt the user for a response.
                                                                                
   Multiple Reply-Message's MAY be included and if any are displayed,
   they MUST be displayed in the same order as they appear in the
   message."

  */
                                                                                
  DiameterVectorAttribute<diameter_utf8string_t> ReplyMessage; // ++

  /* In draft-ietf-aaa-diameter-nasreq-13.txt:

   "6.7.  Filter-Id AVP
                                                                                
   The Filter-Id AVP (AVP Code 11) is of type UTF8String, and contains
   the name of the filter list for this user. Zero or more Filter-Id
   AVPs MAY be sent in an authorization answer.
                                                                                
   Identifying a filter list by name allows the filter to be used on
   different NASes without regard to filter-list implementation details.
   However, this AVP is not roaming friendly since filter naming differs
   from one service provider to another.
                                                                                
   In non-RADIUS environments, it is RECOMMENDED that the NAS-Filter-
   Rule AVP be used instead."

  */

  DiameterVectorAttribute<diameter_utf8string_t> FilterId; // ++

  /* In draft-ietf-aaa-diameter-nasreq-13.txt:

   "6.3.  Callback-Id AVP
                                                                                
   The Callback-Id AVP (AVP Code 20) is of type UTF8String, and contains
   the name of a place to be called, to be interpreted by the NAS. This
   AVP MAY be present in an authentication and/or authorization
   response.
                                                                                
   This AVP is not roaming-friendly since it assumes that the Callback-
   Id is configured on the NAS. It is therefore preferable to use the
   Callback-Number AVP instead."

  */
                                                                                
  DiameterScholarAttribute<diameter_utf8string_t> CallbackId; // ++
  DiameterScholarAttribute<diameter_unsigned32_t> FramedAppletalkLink; // ++ 
  DiameterVectorAttribute<diameter_unsigned32_t> FramedAppletalkNetwork; // ++ 
  DiameterScholarAttribute<diameter_octetstring_t> FramedAppletalkZone; // ++ 
  DiameterVectorAttribute<diameter_utf8string_t> FramedIpv6Route; // ++
  DiameterScholarAttribute<diameter_octetstring_t> FramedIpv6Pool; // ++ 
  DiameterScholarAttribute<diameter_octetstring_t> FramedPool; // ++ 
  DiameterScholarAttribute<diameter_utf8string_t> FramedIpxNetwork; // ++
  DiameterVectorAttribute<diameter_utf8string_t> FramedRoute; // ++


  /* In draft-ietf-aaa-diameter-nasreq-13.txt:

   "6.9.2.  Framed-Routing AVP
                                                                                
   The Framed-Routing AVP (AVP Code 10) is of type Enumerated and
   contains the routing method for the user, when the user is a router
   to a network.  This AVP SHOULD only be present in authorization
   responses. The supported values are listed in [RADIUSTypes].      The
   following list is informational:
                                                                                
      0  None
      1  Send routing packets
      2  Listen for routing packets
      3  Send and Listen" 

  */

  DiameterScholarAttribute<diameter_enumerated_t> FramedRouting; // ++
  DiameterVectorAttribute<diameter_identity_t>  RedirectHost; // ++
  DiameterScholarAttribute<diameter_enumerated_t>  RedirectHostUsage; // ++
  DiameterScholarAttribute<diameter_unsigned32_t>  RedirectMaxCacheTime; // ++
  //  -- std::vector<diameter_utf8string_t> CalledStationId;
  //  -- std::vector<diameter_utf8string_t> CallingStationId;
  //  -- std::vector<diameter_octetstring_t> OriginatingLineInfo;
  //  -- std::vector<diameter_utf8string_t> ConnectInfo;
  //  -- std::vector<diameter_identity_t> RouteRecord;
};

typedef AAAParser<DiameterMsg*, DER_Data*> DER_Parser;
typedef AAAParser<DiameterMsg*, DEA_Data*> DEA_Parser;

template<> void DER_Parser::parseRawToApp();
template<> void DER_Parser::parseAppToRaw();

template<> void DEA_Parser::parseRawToApp();
template<> void DEA_Parser::parseAppToRaw();

#endif //__EAP_CLIENT_DATA_H__

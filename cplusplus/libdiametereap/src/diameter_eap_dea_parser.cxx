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

/* $Id: diameter_eap_dea_parser.cxx,v 1.15 2006/03/16 17:01:32 vfajardo Exp $ */
/* 
   diameter_eap_dea_parser.cxx
   Diameter EAP Answer Parser
   Written by Yoshihiro Ohba
   Created December 8, 2003.
*/

#include "diameter_eap_parser.hxx"

template<> void 
DEA_Parser::parseAppToRaw()
{
  DEA_Data &data = *getAppData();
  DiameterMsg &aaaMessage = *getRawData();

  
  DiameterDictionaryManager dm;
  DiameterAvpContainerManager cm;
  AAAAvpContainer *c;
                          
  AAACommandCode code;
  DiameterApplicationId appId;

  // Obtain Command Code and Application Identifier.
  if (!dm.getCommandCode("Diameter-EAP-Answer", &code, &appId))
    {
      AAA_LOG((LM_ERROR, "Cannot find Diameter message in dictionary\n."));
      throw (DIAMETER_DICTIONARY_ERROR);
      return;
    }

  // Specify the header.
  diameter_hdr_flag flag = {0,0,0};  // Answer
  DiameterMsgHeader hdr(1, 0, flag, code, appId, 0, 0);
  aaaMessage.hdr = hdr;

  if (data.AuthApplicationId.IsSet())
    {
      c = cm.acquire("Auth-Application-Id");
      data.AuthApplicationId.CopyTo(*c, AAA_AVP_INTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.AuthRequestType.IsSet())
    {
      c = cm.acquire("Auth-Request-Type");
      data.AuthRequestType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.OriginStateId.IsSet())
    { 
      c = cm.acquire("Origin-State-Id");
      data.OriginStateId.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.PortLimit.IsSet())
    { 
      c = cm.acquire("Port-Limit");
      data.PortLimit.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.UserName.IsSet())
    { 
      c = cm.acquire("User-Name");
      data.UserName.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.EapPayload.IsSet())
    { 
      c = cm.acquire("EAP-Payload");
      data.EapPayload.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.EapReissuedPayload.IsSet())
    { 
      c = cm.acquire("EAP-Reissued-Payload");
      data.EapReissuedPayload.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.EapMasterSessionKey.IsSet())
    { 
      c = cm.acquire("EAP-Master-Session-Key");
      data.EapMasterSessionKey.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.AccountingEapAuthMethod.IsSet())
    { 
      c = cm.acquire("Accounting-Eap-Auth-Method");
      data.AccountingEapAuthMethod.CopyTo(*c, AAA_AVP_UINTEGER64_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ServiceType.IsSet())
    { 
      c = cm.acquire("Service-Type");
      data.ServiceType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ResultCode.IsSet())
    { 
      c = cm.acquire("Result-Code");
      data.ResultCode.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.MultiRoundTimeOut.IsSet())
    { 
      c = cm.acquire("Multi-Round-Time-Out");
      data.MultiRoundTimeOut.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ConfigurationToken.IsSet())
    { 
      c = cm.acquire("Configuration-Token");
      data.ConfigurationToken.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.AcctInterimInterval.IsSet())
    { 
      c = cm.acquire("Acct-Interim-Interval");
      data.AcctInterimInterval.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ErrorMessage.IsSet())
    { 
      c = cm.acquire("Error-Message");
      data.ErrorMessage.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ErrorReportingHost.IsSet())
    {
      c = cm.acquire("Error-Reporting-Host");
      data.ErrorReportingHost.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ReAuthRequestType.IsSet())
    { 
      c = cm.acquire("Re-Auth-Request-Type");
      data.ReAuthRequestType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ReplyMessage.IsSet())
    { 
      c = cm.acquire("Reply-Message");
      data.ReplyMessage.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FilterId.IsSet())
    { 
      c = cm.acquire("Filter-Id");
      data.FilterId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.CallbackId.IsSet())
    { 
      c = cm.acquire("Callback-Id");
      data.CallbackId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedAppletalkLink.IsSet())
    { 
      c = cm.acquire("Framed-Appletalk-Link");
      data.FramedAppletalkLink.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedAppletalkNetwork.IsSet())
    { 
      c = cm.acquire("Framed-Appletalk-Network");
      data.FramedAppletalkNetwork.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedAppletalkZone.IsSet())
    { 
      c = cm.acquire("Framed-Appletalk-Zone");
      data.FramedAppletalkZone.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedIpv6Route.IsSet())
    { 
      c = cm.acquire("Framed-IPv6-Route");
      data.FramedIpv6Route.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedIpv6Pool.IsSet())
    { 
      c = cm.acquire("Framed-IPv6-Pool");
      data.FramedIpv6Pool.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedPool.IsSet())
    { 
      c = cm.acquire("Framed-Pool");
      data.FramedPool.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedIpxNetwork.IsSet())
    { 
      c = cm.acquire("Framed-IPX-Network");
      data.FramedIpxNetwork.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedRoute.IsSet())
    { 
      c = cm.acquire("Framed-Route");
      data.FramedRoute.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedRouting.IsSet())
    { 
      c = cm.acquire("Framed-Routing");
      data.FramedRouting.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.RedirectHost.IsSet())
    {
      c = cm.acquire("Redirect-Host");
      data.RedirectHost.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.RedirectHostUsage.IsSet())
    { 
      c = cm.acquire("Redirect-Host-Usage");
      data.RedirectHostUsage.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }

  if (data.RedirectMaxCacheTime.IsSet())
    { 
      c = cm.acquire("Redirect-Max-Cache-Time");
      data.RedirectMaxCacheTime.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }

  if (data.IdleTimeout.IsSet())
    { 
      c = cm.acquire("Idle-Timeout");
      data.IdleTimeout.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.State.IsSet())
    { 
      c = cm.acquire("State");
      data.State.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.AuthorizationLifetime.IsSet())
    { 
      c = cm.acquire("Authorization-Lifetime");
      data.AuthorizationLifetime.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.AuthGracePeriod.IsSet())
    { 
      c = cm.acquire("Auth-Grace-Period");
      data.AuthGracePeriod.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.AuthSessionState.IsSet())
    { 
      c = cm.acquire("Auth-Session-State");
      data.AuthSessionState.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.SessionTimeout.IsSet())
    { 
      c = cm.acquire("Session-Timeout");
      data.SessionTimeout.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.CallbackNumber.IsSet())
    { 
      c = cm.acquire("Callback-Number");
      data.CallbackNumber.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.Class.IsSet())
    {
      c = cm.acquire("Class");
      data.Class.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedCompression.IsSet())
    { 
      c = cm.acquire("Framed-Compression");
      data.FramedCompression.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedInterfaceId.IsSet())
    { 
      c = cm.acquire("Framed-Interface-Id");
      data.FramedInterfaceId.CopyTo(*c, AAA_AVP_UINTEGER64_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedIpAddress.IsSet())
    { 
      c = cm.acquire("Framed-IP-Address");
      data.FramedIpAddress.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedIpv6Prefix.IsSet())
    {
      c = cm.acquire("Framed-IPv6-Prefix");
      data.FramedIpv6Prefix.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedIpNetmask.IsSet())
    { 
      c = cm.acquire("Framed-IP-Netmask");
      data.FramedIpNetmask.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedMtu.IsSet())
    { 
      c = cm.acquire("Framed-MTU");
      data.FramedMtu.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FramedProtocol.IsSet())
    { 
      c = cm.acquire("Framed-Protocol");
      data.FramedProtocol.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.NasFilterRule.IsSet())
    {
      c = cm.acquire("NAS-Filter-Rule");
      data.NasFilterRule.CopyTo(*c, AAA_AVP_IPFILTER_RULE_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.Tunneling.IsSet())
    {
      c = cm.acquire("Tunneling");
      data.Tunneling.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.ProxyInfo.IsSet())
    {
      c = cm.acquire("Proxy-Info");
      data.ProxyInfo.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.Avp.IsSet())
    {
      c = cm.acquire("AVP");
      data.Avp.CopyTo(*c, AAA_AVP_CUSTOM_TYPE);
      aaaMessage.acl.add(c);
    }
}

template<> void 
DEA_Parser::parseRawToApp()
{
  DEA_Data &data = *getAppData();
  DiameterMsg &aaaMessage = *getRawData();

  data.Clear();

  //  AAAAvpContainerEntry *e;
  AAAAvpContainer *c;
  if ((c = aaaMessage.acl.search("Session-Id")))
    {
      data.SessionId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Auth-Application-Id")))
    {
      data.AuthApplicationId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Origin-Host")))
    {
      data.OriginHost.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Origin-Realm")))
    {
      data.OriginRealm.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Origin-State-Id")))
    {
      data.OriginStateId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Port-Limit")))
    {
      data.PortLimit.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("User-Name")))
    {
      data.UserName.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("EAP-Payload")))
    {
      data.EapPayload.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("EAP-Reissued-Payload")))
    {
      data.EapReissuedPayload.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("EAP-Master-Session-Key")))
    {
      data.EapMasterSessionKey.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Accounting-Eap-Auth-Method")))
    {
      data.AccountingEapAuthMethod.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Service-Type")))
    {
      data.ServiceType.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Result-Code")))
    {
      data.ResultCode.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Multi-Round-Time-Out")))
    {
      data.MultiRoundTimeOut.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Configuration-Token")))
    {
      data.ConfigurationToken.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Acct-Interim-Interval")))
    {
      data.AcctInterimInterval.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Error-Message")))
    {
      data.ErrorMessage.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Error-Reporting-Host")))
    {
      data.ErrorReportingHost.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Re-Auth-Request-Type")))
    {
      data.ReAuthRequestType.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Filter-Id")))
    {
      data.FilterId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Callback-Id")))
    {
      data.CallbackId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-Appletalk-Link")))
    {
      data.FramedAppletalkLink.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-Appletalk-Network")))
    {
      data.FramedAppletalkNetwork.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-Appletalk-Zone")))
    {
      data.FramedAppletalkZone.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-IPv6-Route")))
    {
      data.FramedIpv6Route.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-IPv6-Pool")))
    {
      data.FramedIpv6Pool.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-Pool")))
    {
      data.FramedPool.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-IPX-Network")))
    {
      data.FramedIpxNetwork.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-Route")))
    {
      data.FramedRoute.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-Routing")))
    {
      data.FramedRouting.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Redirect-Host")))
    {
      data.RedirectHost.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Redirect-Host-Usage")))
    {
      data.RedirectHostUsage.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Redirect-Max-Cache-Time")))
    {
      data.RedirectMaxCacheTime.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Idle-Timeout")))
    {
      data.IdleTimeout.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("State")))
    {
      data.State.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Authorization-Lifetime")))
    {
      data.AuthorizationLifetime.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Auth-Grace-Period")))
    {
      data.AuthGracePeriod.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Auth-Session-State")))
    {
      data.AuthSessionState.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Session-Timeout")))
    {
      data.SessionTimeout.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Callback-Number")))
    {
      data.CallbackNumber.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Class")))
    {
      data.Class.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-Compression")))
    {
      data.FramedCompression.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-Interface-Id")))
    {
      data.FramedInterfaceId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-IP-Address")))
    {
      data.FramedIpAddress.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-IPv6-Prefix")))
    {
      data.FramedIpv6Prefix.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-MTU")))
    {
      data.FramedMtu.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-Protocol")))
    {
      data.FramedProtocol.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("NAS-Filter-Rule")))
    {
      data.NasFilterRule.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Tunneling")))
    {
      data.Tunneling.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Proxy-Info")))
    {
      data.ProxyInfo.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("AVP")))
    {
      data.Avp.CopyFrom(*c);
    }
}


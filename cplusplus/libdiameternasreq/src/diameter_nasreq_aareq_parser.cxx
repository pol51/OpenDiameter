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

/* $Id: diameter_nasreq_aareq_parser.cxx,v 1.7 2006/03/16 17:01:47 vfajardo Exp $ */
/* 
   diameter_nasrq_aareq_parser.cxx
   Diameter AA-Request Parser
   Written by Yoshihiro Ohba
   Created May 1, 2004
*/

#include "diameter_nasreq_parser.hxx"

template<> void 
AA_RequestParser::parseAppToRaw()
{
  AA_RequestData &data = *getAppData();
  DiameterMsg &aaaMessage = *getRawData();

  DiameterDictionaryManager dm;
  DiameterAvpContainerManager cm;
  AAAAvpContainer *c;
                          
  AAACommandCode code;
  DiameterApplicationId appId;

  // Obtain Command Code and Application Identifier.
  if (!dm.getCommandCode("AA-Request", &code, &appId))
    {
      AAA_LOG((LM_ERROR, "[%N] Cannot find message in dictionary\n."));
      throw (DIAMETER_DICTIONARY_ERROR);
    }

  // Specify the header.
  diameter_hdr_flag flag = {1,1,0};
  DiameterMsgHeader hdr(1, 0, flag, code, appId, 0, 0);
  aaaMessage.hdr = hdr;

  if (data.AuthApplicationId.IsSet())
    {
      c = cm.acquire("Auth-Application-Id");
      data.AuthApplicationId.CopyTo(*c, AAA_AVP_INTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.OriginHost.IsSet())
    {
      c = cm.acquire("Origin-Host");
      data.OriginHost.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
      aaaMessage.acl.add(c);
    }

  if (data.OriginRealm.IsSet())
    {
      c = cm.acquire("Origin-Realm");
      data.OriginRealm.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
      aaaMessage.acl.add(c);
    }

  if (data.DestinationRealm.IsSet())
    {
      c = cm.acquire("Destination-Realm");
      data.DestinationRealm.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
      aaaMessage.acl.add(c);
    }

  if (data.AuthRequestType.IsSet())
    {
      c = cm.acquire("Auth-Request-Type");
      data.AuthRequestType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.DestinationHost.IsSet())
    { 
      c = cm.acquire("Destination-Host");
      data.DestinationHost.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.NasIdentifier.IsSet())
    { 
      c = cm.acquire("Nas-Identifier");
      data.NasIdentifier.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.NasIpAddress.IsSet())
    { 
      c = cm.acquire("Nas-IP-Address");
      data.NasIpAddress.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.NasIpv6Address.IsSet())
    { 
      c = cm.acquire("Nas-IPv6-Address");
      data.NasIpv6Address.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.NasPort.IsSet())
    { 
      c = cm.acquire("Nas-Port");
      data.NasPort.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.NasPortId.IsSet())
    { 
      c = cm.acquire("Nas-Port-Id");
      data.NasPortId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.NasPortType.IsSet())
    { 
      c = cm.acquire("Nas-Port-Type");
      data.NasPortType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
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
  if (data.UserPassword.IsSet())
    { 
      c = cm.acquire("User-Password");
      data.UserPassword.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ServiceType.IsSet())
    { 
      c = cm.acquire("Service-Type");
      data.ServiceType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
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
  if (data.CallbackNumber.IsSet())
    { 
      c = cm.acquire("Callback-Number");
      data.CallbackNumber.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.CalledStationId.IsSet())
    { 
      c = cm.acquire("Called-Station-Id");
      data.CalledStationId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.CallingStationId.IsSet())
    { 
      c = cm.acquire("Calling-Station-Id");
      data.CallingStationId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.OriginatingLineInfo.IsSet())
    { 
      c = cm.acquire("Originating-Line-Info");
      data.OriginatingLineInfo.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ConnectInfo.IsSet())
    { 
      c = cm.acquire("Connect-Info");
      data.ConnectInfo.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ChapAuth.IsSet())
    {
      c = cm.acquire("CHAP-Auth");
      data.ChapAuth.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.ChapChallenge.IsSet())
    { 
      c = cm.acquire("CHAP-Challenge");
      data.ChapChallenge.CopyTo(*c, AAA_AVP_STRING_TYPE);
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
      data.FramedInterfaceId.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
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
  if (data.ArapPassword.IsSet())
    { 
      c = cm.acquire("ARAP-Password");
      data.ArapPassword.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ArapChallengeResponse.IsSet())
    { 
      c = cm.acquire("ARAP-Challenge-Response");
      data.ArapChallengeResponse.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ArapSecurity.IsSet())
    { 
      c = cm.acquire("ARAP-Security");
      data.ArapSecurity.CopyTo(*c, AAA_AVP_INTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ArapSecurityData.IsSet())
    { 
      c = cm.acquire("ARAP-Security-Data");
      data.ArapSecurityData.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.LoginIpHost.IsSet())
    { 
      c = cm.acquire("Login-IP-Host");
      data.LoginIpHost.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.LoginIpv6Host.IsSet())
    { 
      c = cm.acquire("Login-IPv6-Host");
      data.LoginIpv6Host.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.LoginLatGroup.IsSet())
    { 
      c = cm.acquire("Login-LAT-Group");
      data.LoginLatGroup.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.LoginLatNode.IsSet())
    { 
      c = cm.acquire("Login-LAT-Node");
      data.LoginLatNode.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.LoginLatPort.IsSet())
    { 
      c = cm.acquire("Login-LAT-Port");
      data.LoginLatPort.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.LoginLatService.IsSet())
    { 
      c = cm.acquire("Login-LAT-Service");
      data.LoginLatService.CopyTo(*c, AAA_AVP_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.Tunneling.IsSet())
    {
      c = cm.acquire("Tunneling");
      data.Tunneling.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.ProxyInfo.size()>0)
    {
      c = cm.acquire("Proxy-Info");
      data.ProxyInfo.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.RouteRecord.IsSet())
    {
      c = cm.acquire("Route-Record");
      data.RouteRecord.CopyTo(*c, AAA_AVP_STRING_TYPE);
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
AA_RequestParser::parseRawToApp()
{
  AA_RequestData &data = *getAppData();
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
      if (NasreqApplicationId
	  != (*c)[0]->dataRef(Type2Type<diameter_unsigned32_t>()))
	{
	  AAA_LOG((LM_ERROR, "[%N] Unexpected application id.\n"));
	  throw (DIAMETER_PAYLOAD_ERROR);
	}
    }
  if ((c = aaaMessage.acl.search("Origin-Host")))
    {
      data.OriginHost.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Origin-Realm")))
    {
      data.OriginRealm.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Destination-Realm")))
    {
      data.DestinationRealm.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Auth-Request-Type")))
    {
      data.AuthRequestType.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Destination-Host")))
    {
      data.DestinationHost.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Nas-Identifier")))
    {
      data.NasIdentifier.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Nas-IP-Address")))
    {
      data.NasIpAddress.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Nas-IPv6-Address")))
    {
      data.NasIpv6Address.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Nas-Port")))
    {
      data.NasPort.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Nas-Port-Id")))
    {
      data.NasPortId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Nas-Port-Type")))
    {
      data.NasPortType.CopyFrom(*c);
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
  if ((c = aaaMessage.acl.search("User-Password")))
    {
      data.UserPassword.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Service-Type")))
    {
      data.ServiceType.CopyFrom(*c);
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
  if ((c = aaaMessage.acl.search("Callback-Number")))
    {
      data.CallbackNumber.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Called-Station-Id")))
    {
      data.CalledStationId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Calling-Station-Id")))
    {
      data.CallingStationId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Originating-Line-Info")))
    {
      data.OriginatingLineInfo.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Connect-Info")))
    {
      data.ConnectInfo.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("CHAP-Auth")))
    {
      data.ChapAuth.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("CHAP-Challenge")))
    {
      data.ChapChallenge.CopyFrom(*c);
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
  if ((c = aaaMessage.acl.search("Framed-IP-Netmask")))
    {
      data.FramedIpAddress.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Framed-MTU")))
    {
      data.FramedMtu = (*c)[0]->dataRef(Type2Type<diameter_unsigned32_t>());
    }
  if ((c = aaaMessage.acl.search("Framed-Protocol")))
    {
      data.FramedProtocol.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("ARAP-Password")))
    {
      data.ArapPassword.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("ARAP-Challenge-Response")))
    {
      data.ArapChallengeResponse.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("ARAP-Security")))
    {
      data.ArapSecurity.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("ARAP-Security-Data")))
    {
      data.ArapSecurityData.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Login-IP-Host")))
    {
      data.LoginIpHost.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Login-IPv6-Host")))
    {
      data.LoginIpv6Host.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Login-LAT-Group")))
    {
      data.LoginLatGroup.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Login-LAT-Node")))
    {
      data.LoginLatNode.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Login-LAT-Port")))
    {
      data.LoginLatPort.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Login-LAT-Service")))
    {
      data.LoginLatService.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Tunneling")))
    {
      data.Tunneling.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Proxy-Info")))
    {
      data.ProxyInfo.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Route-Record")))
    {
      data.RouteRecord.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("AVP")))
    {
      data.Avp.CopyFrom(*c);
    }
}


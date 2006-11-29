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

/* $Id: diameter_cc_req_parser.cxx $ */

#include "diameter_cc_parser.h"

template<> void 
CCR_Parser::parseAppToRaw()
{
  CCR_Data &data = *getAppData();
  DiameterMsg &aaaMessage = *getRawData();

  DiameterDictionaryManager dm;
  DiameterAvpContainerManager cm;
  AAAAvpContainer *c;
                          
  AAACommandCode code;
  DiameterApplicationId appId;

  // Obtain Command Code and Application Identifier.
  if (!dm.getCommandCode("CC-Request", &code, &appId))
    {
      AAA_LOG((LM_ERROR, "[%N] Cannot find message in dictionary\n."));
      throw (DIAMETER_DICTIONARY_ERROR);
    }

  // Specify the header.
  diameter_hdr_flag flag = {1,1,0};
  aaaMessage.hdr = DiameterMsgHeader(1, 0, flag, code, appId, 0, 0);

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
  if (data.AuthApplicationId.IsSet())
    {
      c = cm.acquire("Auth-Application-Id");
      data.AuthApplicationId.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ServiceContextId.IsSet())
    {
      c = cm.acquire("Service-Context-Id");
      data.ServiceContextId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.CCRequestType.IsSet())
    { 
      c = cm.acquire("CC-Request-Type");
      data.CCRequestType.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.CCRequestNumber.IsSet())
    { 
      c = cm.acquire("CC-Request-Number");
      data.CCRequestNumber.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.DestinationHost.IsSet())
    { 
      c = cm.acquire("Destination-Host");
      data.DestinationHost.CopyTo(*c, AAA_AVP_DIAMID_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.UserName.IsSet())
    { 
      c = cm.acquire("User-Name");
      data.UserName.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.CCSubSessionId.IsSet())
    { 
      c = cm.acquire("CC-Sub-Session-Id");
      data.CCSubSessionId.CopyTo(*c, AAA_AVP_UINTEGER64_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.AcctMultiSessionId.IsSet())
    { 
      c = cm.acquire("Acct-Multi-Session-Id");
      data.AcctMultiSessionId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.OriginStateId.IsSet())
    { 
      c = cm.acquire("Origin-State-Id");
      data.OriginStateId.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.EventTimestamp.IsSet())
    { 
      c = cm.acquire("Event-Timestamp");
      data.EventTimestamp.CopyTo(*c, AAA_AVP_TIME_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.SubscriptionId.IsSet())
    { 
      c = cm.acquire("Subscription-Id");
      data.SubscriptionId.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.ServiceIdentifier.IsSet())
    { 
      c = cm.acquire("Service-Identifier");
      data.ServiceIdentifier.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.TerminationCause.IsSet())
    { 
      c = cm.acquire("Termination-Cause");
      data.TerminationCause.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.RequestedServiceUnit.IsSet())
    { 
      c = cm.acquire("Requested-Service-Unit");
      data.RequestedServiceUnit.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.RequestedAction.IsSet())
    { 
      c = cm.acquire("Requested-Action");
      data.RequestedAction.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.UsedServiceUnit.IsSet())
    { 
      c = cm.acquire("Used-Service-Unit");
      data.UsedServiceUnit.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.MultipleServicesIndicator.IsSet())
    { 
      c = cm.acquire("Multiple-Services-Indicator");
      data.MultipleServicesIndicator.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.MultipleServicesCreditControl.IsSet())
    { 
      c = cm.acquire("Multiple-Services-Credit-Control");
      data.MultipleServicesCreditControl.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.ServiceParameterInfo.IsSet())
    { 
      c = cm.acquire("Service-Parameter-Info");
      data.ServiceParameterInfo.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.CCCorrelationId.IsSet())
    { 
      c = cm.acquire("CC-Correlation-Id");
      data.CCCorrelationId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.UserEquipmentInfo.IsSet())
    { 
      c = cm.acquire("User-Equipment-Info");
      data.UserEquipmentInfo.CopyTo(*c);
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
CCR_Parser::parseRawToApp()
{
  CCR_Data &data = *getAppData();
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
      if (CCApplicationId
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
  if ((c = aaaMessage.acl.search("Service-Context-Id")))
    {
      data.ServiceContextId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("CC-Request-Type")))
    {
      data.CCRequestType.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("CC-Request-Number")))
    {
      data.CCRequestNumber.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Destination-Host")))
    {
      data.DestinationHost.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("User-Name")))
    {
      data.UserName.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("CC-Sub-Session-Id")))
    {
      data.CCSubSessionId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Acct-Multi-Session-Id")))
    {
      data.AcctMultiSessionId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Origin-State-Id")))
    {
      data.OriginStateId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Event-Timestamp")))
    {
      data.EventTimestamp.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Subscription-Id")))
    {
      data.SubscriptionId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Termination-Cause")))
    {
      data.TerminationCause.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Requested-Service-Unit")))
    {
      data.RequestedServiceUnit.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Requested-Action")))
    {
      data.RequestedAction.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Used-Service-Unit")))
    {
      data.UsedServiceUnit.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Multiple-Services-Indicator")))
    {
      data.MultipleServicesIndicator.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Multiple-Services-Credit-Control")))
    {
      data.MultipleServicesCreditControl.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Service-Parameter-Info")))
    {
      data.ServiceParameterInfo.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("CC-Correlation-Id")))
    {
      data.CCCorrelationId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("User-Equipment-Info")))
    {
      data.UserEquipmentInfo.CopyFrom(*c);
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


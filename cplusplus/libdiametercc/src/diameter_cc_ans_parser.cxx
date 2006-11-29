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

/* $Id: diameter_cc_ans_parser.cxx $ */

#include "diameter_cc_parser.h"

template<> void 
CCA_Parser::parseAppToRaw()
{
  CCA_Data &data = *getAppData();
  DiameterMsg &aaaMessage = *getRawData();

  
  DiameterDictionaryManager dm;
  DiameterAvpContainerManager cm;
  AAAAvpContainer *c;
                          
  AAACommandCode code;
  DiameterApplicationId appId;

  // Obtain Command Code and Application Identifier.
  if (!dm.getCommandCode("CC-Answer", &code, &appId))
    {
      AAA_LOG((LM_ERROR, "Cannot find Diameter message in dictionary\n."));
      throw (DIAMETER_DICTIONARY_ERROR);
      return;
    }

  // Specify the header.
  diameter_hdr_flag flag = {0,0,0};  // Answer
  aaaMessage.hdr = DiameterMsgHeader(1, 0, flag, code, appId, 0, 0);

  if (data.ResultCode.IsSet())
    { 
      c = cm.acquire("Result-Code");
      data.ResultCode.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
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
  if (data.AuthApplicationId.IsSet())
    {
      c = cm.acquire("Auth-Application-Id");
      data.AuthApplicationId.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
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
  if (data.UserName.IsSet())
    { 
      c = cm.acquire("User-Name");
      data.UserName.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.CCSessionFailover.IsSet())
    { 
      c = cm.acquire("CC-Session-Failover");
      data.CCSessionFailover.CopyTo(*c, AAA_AVP_ENUM_TYPE);
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
  if (data.AuthRequestType.IsSet())
    { 
      c = cm.acquire("Auth-Request-Type");
      data.AcctMultiSessionId.CopyTo(*c, AAA_AVP_ENUM_TYPE);
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
  if (data.GrantedServiceUnit.IsSet())
    { 
      c = cm.acquire("Granted-Service-Unit");
      data.GrantedServiceUnit.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.MultipleServicesCreditControl.IsSet())
    { 
      c = cm.acquire("Multiple-Services-Credit-Control");
      data.MultipleServicesCreditControl.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.CostInformation.IsSet())
    { 
      c = cm.acquire("Cost-Information");
      data.CostInformation.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.FinalUnitIndication.IsSet())
    { 
      c = cm.acquire("Final-Unit-Indication");
      data.FinalUnitIndication.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.CheckBalanceResult.IsSet())
    { 
      c = cm.acquire("Check-Balance-Result");
      data.CheckBalanceResult.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.CreditControlFailureHandling.IsSet())
    { 
      c = cm.acquire("Credit-Control-Failure-Handling");
      data.CreditControlFailureHandling.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.DirectDebitingFailureHandling.IsSet())
    { 
      c = cm.acquire("Direct-Debiting-Failure-Handling");
      data.DirectDebitingFailureHandling.CopyTo(*c, AAA_AVP_ENUM_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.ValidityTime.IsSet())
    { 
      c = cm.acquire("Validity-Time");
      data.ValidityTime.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
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
  if (data.ProxyInfo.size()>0)
    {
      c = cm.acquire("Proxy-Info");
      data.ProxyInfo.CopyTo(*c);
      aaaMessage.acl.add(c);
    }
  if (data.RouteRecord.IsSet())
    { 
      c = cm.acquire("Route-Record");
      data.RouteRecord.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.FailedAvp.IsSet())
    { 
      c = cm.acquire("Failed-AVP");
      data.FailedAvp.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
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
CCA_Parser::parseRawToApp()
{
  CCA_Data &data = *getAppData();
  DiameterMsg &aaaMessage = *getRawData();

  data.Clear();

  AAAAvpContainer *c;

  if ((c = aaaMessage.acl.search("Session-Id")))
    {
      data.SessionId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Result-Code")))
    {
      data.ResultCode.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Origin-Host")))
    {
      data.OriginHost.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Origin-Realm")))
    {
      data.OriginRealm.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Auth-Application-Id")))
    {
      data.AuthApplicationId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("CC-Request-Type")))
    {
      data.CCRequestType.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("CC-Request-Number")))
    {
      data.CCRequestNumber.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("User-Name")))
    {
      data.UserName.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("CC-Session-Failover")))
    {
      data.CCSessionFailover.CopyFrom(*c);
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
  if ((c = aaaMessage.acl.search("Granted-Service-Unit")))
    {
      data.GrantedServiceUnit.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Multiple-Services-Credit-Control")))
    {
      data.MultipleServicesCreditControl.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Cost-Information")))
    {
      data.CostInformation.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Final-Unit-Indication")))
    {
      data.FinalUnitIndication.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Check-Balance-Result")))
    {
      data.CheckBalanceResult.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Credit-Control-Failure-Handling")))
    {
      data.CreditControlFailureHandling.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Direct-Debiting-Failure-Handling")))
    {
      data.DirectDebitingFailureHandling.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Validity-Time")))
    {
      data.ValidityTime.CopyFrom(*c);
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
  if ((c = aaaMessage.acl.search("Proxy-Info")))
    {
      data.ProxyInfo.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Route-Record")))
    {
      data.RouteRecord.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Failed-AVP")))
    {
      data.FailedAvp.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("AVP")))
    {
      data.Avp.CopyFrom(*c);
    }
}


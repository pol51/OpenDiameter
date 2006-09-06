/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* OpenDiameter: Open-source software for the Diameter protocol           */
/*                                                                        */
/* Copyright (C) 2002-2004 Open Diameter Project	                  */
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
/* 
   diameter_mip4_ama_parser.cxx
   Diameter MIP AMA Answer Parser
   Written by Miriam Tauil
   Created May 27, 2004.
*/

#include "diameter_mip4_parser.hxx"

template<> void 
HAA_Parser::parseAppToRaw()
{
  HAA_Data &data = *getAppData();
  DiameterMsg &aaaMessage = *getRawData();

  
  DiameterDictionaryManager dm;
  DiameterAvpContainerManager cm;
  AAAAvpContainer *c;
                          
  AAACommandCode code;
  DiameterApplicationId appId;

  // Obtain Command Code and Application Identifier.
  if (!dm.getCommandCode("Home-Agent-MIP-Answer", &code, &appId))
    {
      AAA_LOG((LM_ERROR, "Cannot find Diameter message in dictionary\n."));
      throw (DIAMETER_DICTIONARY_ERROR);
      return;
    }

  // Specify the header.
  diameter_hdr_flag flag = {0,0,0};  // Answer
  aaaMessage.hdr = DiameterMsgHeader(1, 0, flag, code, appId, 0, 0);

  if (data.AuthApplicationId.IsSet())
    {
      c = cm.acquire("Auth-Application-Id");
      data.AuthApplicationId.CopyTo(*c, AAA_AVP_INTEGER32_TYPE);
      aaaMessage.acl.add(c);
    } 
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
 if (data.AcctMultiSessionId.IsSet())
    { 
      c = cm.acquire("Acct-Multi-Session-Id");
      data.AcctMultiSessionId.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
      aaaMessage.acl.add(c);
    }
  if (data.UserName.IsSet())
    { 
      c = cm.acquire("User-Name");
      data.UserName.CopyTo(*c, AAA_AVP_UTF8_STRING_TYPE);
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
  if (data.MipRegReply.IsSet())
    { 
      c = cm.acquire("MIP-Reg-Reply");
      data.MipRegReply.CopyTo(*c, AAA_AVP_STRING_TYPE); 
      aaaMessage.acl.add(c);
    }


 if (data.MipHomeAgentAddress.IsSet())				
    {
      c = cm.acquire("MIP-Home-Agent-Address");
      data.MipHomeAgentAddress.CopyTo(*c, AAA_AVP_ADDRESS_TYPE);
      aaaMessage.acl.add(c);
    } 
 if (data.MipMobileNodeAddress.IsSet())				
    {
      c = cm.acquire("MIP-Mobile-Node-Address");
      data.MipMobileNodeAddress.CopyTo(*c, AAA_AVP_ADDRESS_TYPE);
      aaaMessage.acl.add(c);
    }
 if (data.MipFaToHaSpi.IsSet())				
    {
      c = cm.acquire("MIP-FA-to-HA-SPI");
      data.MipFaToHaSpi.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }
 if (data.MipFaToMnSpi.IsSet())				
    {
      c = cm.acquire("MIP-FA-to-MN-SPI");
      data.MipFaToMnSpi.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
      aaaMessage.acl.add(c);
    }	
 if (data.OriginStateId.IsSet())
    { 
      c = cm.acquire("Origin-State-Id");
      data.OriginStateId.CopyTo(*c, AAA_AVP_UINTEGER32_TYPE);
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
HAA_Parser::parseRawToApp()
{
  HAA_Data &data = *getAppData();
  DiameterMsg &aaaMessage = *getRawData();

  data.Clear();

  AAAAvpContainer *c;
  if ((c = aaaMessage.acl.search("Session-Id")))
    {
      data.SessionId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Auth-Application-Id")))
    {
      data.AuthApplicationId.CopyFrom(*c);
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
  if ((c = aaaMessage.acl.search("Acct-Multi-Session-Id")))
    {
      data.AcctMultiSessionId.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("User-Name")))
    {
      data.UserName.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Error-Message")))
    {
      data.ErrorMessage.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("Error-Reporting-Host")))
    {
      data.ErrorReportingHost.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("MIP-Reg-Reply")))
    {
      data.MipRegReply.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("MIP-Home-Agent-Address")))
    {
      data.MipHomeAgentAddress.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("MIP-Mobile-Node-Address")))
    {
      data.MipMobileNodeAddress.CopyFrom(*c);
    }
 if ((c = aaaMessage.acl.search("MIP-FA-to-HA-SPI")))
    {
      data.MipFaToHaSpi.CopyFrom(*c);
    }
  if ((c = aaaMessage.acl.search("MIP-FA-to-MN-SPI")))
    {
      data.MipFaToMnSpi.CopyFrom(*c);
    }
 if ((c = aaaMessage.acl.search("Origin-State-Id")))
    {
      data.OriginStateId.CopyFrom(*c);
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

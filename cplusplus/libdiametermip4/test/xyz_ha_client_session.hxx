/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* OpenDiameter: Open-source software for the Diameter protocol           */
/*                                                                        */
/* Copyright (C) 2007 Open Diameter Project 				  */
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

#include "../include/diameter_mip4_ha_client_interface.hxx"


class XyzHaClientSession : public HaClientSession 
{
public:
  XyzHaClientSession(  Mip4DiameterHaClientInterface &s)
     :  HaClientSession(s) {}

  virtual ~XyzHaClientSession(){}
  
  /** The following functions will set the passed variable to contain 
       the value from the MIP reg Request, if the value cannot be set 
	       return 0,
	       otherwise return 1
  **/

 // diameter lib will provide the information from AMA reply to 
  // the MIP HA client implementation. MIP Ha will use this info to 
  // create a Mip replay, will do the address binding(if applicable, 
  //AMA response was positive), and send it to the MN.
  // defined as virtual in fsm. implemented in session by calling this fn
  void SendMipRegReply(diameter_unsigned32_t &amaResultCode)
  {
    
  }

  
  int SetUserName ( diameter_utf8string_t &UserName)
  {
    UserName = "sunil@homedomain.com";
    return 1;
  }

  int SetDestinationRealm ( diameter_identity_t &DestinationRealm )
  {
    DestinationRealm = "homedomain.com";
    return 1;
  }

  int SetMipMnAaaSpi( diameter_unsigned32_t *mnAaaSpi)
  {
    (*mnAaaSpi) = 0x1;
    return 1;
  }
  int SetMipAuthInputDataLength( diameter_unsigned32_t *authInputDataLength)
  {
    (*authInputDataLength) = 0x2;
    return 1;
  }
  int SetMipAuthenticatorLength( diameter_unsigned32_t *authenticatorLength)
  {
    (*authenticatorLength) = 0x3;
    return 1;
  }
  int SetMipAuthenticatorOffset( diameter_unsigned32_t *authenticatorOffset)
  {
    (*authenticatorOffset) = 0x4;
    return 1;
  }
  int SetDestinationHost (diameter_identity_t &DestinationHost )
  {
    DestinationHost = "aaaserver";
   return 1;
  }
  int SetMipMobileNodeAddress( diameter_address_t &MipMobileNodeAddress)
  {
    MipMobileNodeAddress.type =  AAA_ADDR_FAMILY_IPV4;
    MipMobileNodeAddress.value = "123.4.5.067";
   return 1;
  }
  int SetMipHomeAgentAddress( diameter_address_t &MipHomeAgentAddress)
  {
    MipHomeAgentAddress.type =  AAA_ADDR_FAMILY_IPV4;
    MipHomeAgentAddress.value = "123.4.5.011";
   return 1;
  }
  // return 1 if MN_HA keye was requested in MIP reg req
  // return 0 - otherwise
  int IsMnHaKeyRequested()
  {
    return 1;
  }

  //"Lifetime" in MIP reg header
  // set by configuration file - 
  // can be override by the application - library prints out a WARNING 
  int SetAuthorizationLifetime( diameter_unsigned32_t *AuthorizationLifetime)
  {
    (*AuthorizationLifetime) = 29;
    return 0;
  }

  int SetMipHomeAgentHost(diameter_identity_t &HomeAgentHost)
  {
    HomeAgentHost = "hac";
    return 1;
  }

  //Dynamic HA later.. 
  //int SetMipCandidateHomeAgentHost(diameter_identity_t CandidateHomeAgentHost)

  void EnforceMipMnToFaMsa(const mip_mn_to_fa_msa_info_t &mipMnToFaMsa)
  {
    //    int i = mipMnToFaMsa.MipMnAaaSpi();
    
  }

  void EnforceMipMnToHaMsa
  ( const mip_mn_to_ha_msa_info_t &mipMnToHaMsa){}

  void EnforceMipFaToMnMsa
  ( const mip_fa_to_mn_msa_info_t &mipFaToMnMsa){}

  void EnforceMipFaToHaMsa
  ( const mip_fa_to_ha_msa_info_t &mipFaToHaMsa){}

  void EnforceMipMsaLifetime 
  ( const diameter_unsigned32_t &mipMsaLifetime){}

};


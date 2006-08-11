/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* OpenDiameter: Open-source software for the Diameter protocol           */
/*                                                                        */
/* Copyright (C) 2004 Open Diameter Project 				  */
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
   diameter_mip_fa_client_interface.hxx
   Diameter MIP FA client interfacece
   Written by Miriam Tauil
   Created January 19, 2005.
*/

#ifndef __DIAMETER_MIP_FA_CLIENT_INTERFACE_H__
#define __DIAMETER_MIP_FA_CLIENT_INTERFACE_H__
#include "diameter_parser.h"
#include "diameter_mip4_fa_client_session.hxx"
#include "mip4_diameter_fa_client_interface.hxx"

/*************************************************************************
  The FA Client class specifies an interface of a MIP Foreign Agent 
  implementation.
  Once the FA implementation is implemented with the following interface, 
  the Diameter MIP FA sample application, will provide a FA implementation
  supporting the Diameter MIP Application.

 **************************************************************************/


class FaClientSession {
   private:

  Mip4DiameterFaClientInterface &_session;

    public:

      FaClientSession( Mip4DiameterFaClientInterface &s):_session(s){}
        
      virtual ~FaClientSession(){}

      // SendMipRegReplay will be called by the diameter-mip4 lib and MUST be 
      // implemented by the MIP fa implementation.
      // diameter lib will provide the information from AMA reply to 
      // the MIP FA client implementation. MIP FA will use this info to 
      // send the Mip replay and act upon the AMA response ( positive - will 
      // setup the relevant filters for the user, and process his packets 
      // appropriely
      virtual void SendMipRegReply( diameter_unsigned32_t &amaResultCode)=0;
		
      virtual void SendMipRegReply(diameter_unsigned32_t &amaResultCode,
				   diameter_octetstring_t &mipRegReply )=0;

      
      // the user implementation MUST
      // call LocalRxMipRegReq() when it accepts the MIP registration request
      // with the content of the request.
      // This is a callback function that will notify the diameter library
      // about the event and will propagate the content of the received message
      //void LocalRxMipRegReq( diameter_octetstring_t &mipRegReq) 
      void RxMipRegReq( diameter_octetstring_t &mipRegReq) 
      {
         _session.RxMipRegReq( mipRegReq);
      }

      // The derrived class will use this interface function to reset any local
      // variables, so the specificMipFaClient object can be reuse, 
      //for a different future mip session.
      // Note: the reference to the Mip4DiameterFaClientInterface does not
      // need to be reset, since it will be reuse as well.
      virtual AAAReturnCode Reset(){ return AAA_ERR_SUCCESS; }
  
  /** The following functions will set the passed variable to contain 
       the value from the MIP reg Request, if the value cannot be set 
	       return 0,
	       otherwise return 1
  **/

  /*** mandatory AMR attributes  ***/

  // UserName: if "Home Address" from MIP Registration is not 0.0.0.0 -> 
  // Home address
  //	- otherwise NAI extension should be in MIP REgistration and ...
  virtual int SetUserName ( diameter_utf8string_t &UserName) = 0;
		
  // SetDestinationRealm based on Home AAA server NAI 
  virtual int SetDestinationRealm ( diameter_utf8string_t &DestinationRealm ) = 0;

  
  //  the following are used to set MipMnAaaAuth                      
  virtual int SetMipMnAaaSpi( diameter_unsigned32_t *mnAaaSpi)=0;
  virtual int SetMipAuthInputDataLength( 
			     diameter_unsigned32_t *authInputDataLength)=0;
  virtual int SetMipAuthenticatorLength(
			     diameter_unsigned32_t *authenticatorLength)=0;
  virtual int SetMipAuthenticatorOffset( 
			     diameter_unsigned32_t *authenticatorOffset)=0;

  /*** optional AMR attributes - start here ***/
		
  // From Home AAA server NAI 
  virtual int SetDestinationHost (diameter_identity_t &DestinationHost )=0;
  //"Home Address" mobile node's home IP address = type address
  virtual int SetMipMobileNodeAddress( diameter_address_t &MipMobileNodeAddress)=0; 
  // "Home Agent" mobile node's home agent IP address = type address
  virtual int SetMipHomeAgentAddress( diameter_address_t &MipHomeAgentAddress)=0;

  // IsMnHaKeyRequested() needed to setup flags for  MipFeatureVector 
  // return 1 if MN_HA keye was requested in MIP reg req
  // return 0 - otherwise
  virtual int IsMnHaKeyRequested()=0;

  virtual int IsMnFaKeyRequested() =0;
  virtual int IsFaHaKeyRequested() =0;
  virtual int IsMnHomeAddrRequested() =0;
  virtual int IsMnHomeAgentRequested() =0;

  // dynamic host AVP                    

  //"Lifetime" in MIP reg header
  virtual int SetAuthorizationLifetime(
	       diameter_unsigned32_t *AuthorizationLifetime)
  { 
    return 0;
  }

  virtual int SetMipFaChallenge( diameter_octetstring_t &mipFaChallenge)=0;
  
  // dynamic host AVP 
  // virtual int SetMipCandidateHomeAgentHost( diameter_identity_t CandidateHomeAgentHost)=0;

  // Home Agent NAI Or 
  // Home Agent field provides IP address+DNS host name lookup ??
  virtual int SetMipHomeAgentHost(diameter_identity_t &HomeAgentHost)=0;

  virtual int SetMipHaToFaSpi(diameter_unsigned32_t *mipHaToFaSpi)=0;

  // Enforce/Propagate parameters that are coming back in the AMA message   
  // to the MIP Agent

  virtual void EnforceAuthorizationLifetime
  (const diameter_unsigned32_t &authorizationLifetime)=0;

  void EnforceAuthSessionState( const diameter_enumerated_t &authSessionState)
  {}

  void EnforceReAuthRequestType( const diameter_enumerated_t &reAuthReqType){}


  //If the MH supports security features, it should implement the following
  // virtual function, in order to make use of the security parameters 
  // generated for this session.
  virtual void EnforceMipMnToFaMsa
  (const mip_mn_to_fa_msa_info_t &mipMnToFaMsa){}

  virtual void EnforceMipMnToHaMsa
  ( const mip_mn_to_ha_msa_info_t &mipMnToHaMsa){}

  virtual void EnforceMipFaToMnMsa
  ( const mip_fa_to_mn_msa_info_t &mipFaToMnMsa){}

  virtual void EnforceMipFaToHaMsa
  ( const mip_fa_to_ha_msa_info_t &mipFaToHaMsa){}

  virtual void EnforceMipHaToMnMsa 
  ( const mip_ha_to_mn_msa_info_t &mipHaToMnMsa){}

  virtual void EnforceMipMsaLifetime 
  ( const diameter_unsigned32_t &mipMsaLifetime){}

  // FA to get the AMA error message
  virtual void EnforceErrorMessage(const diameter_utf8string_t &errorMessage){}

  virtual void EnforceMipFilterRule ( 
      const  DiameterVectorAttribute<diameter_ipfilter_rule_t> &mipFilterRule)=0;

  
};
 
#endif  // __DIAMETER_MIP_FA_CLIENT_INTERFACE_H__
 
 

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
/* 
   diameter_mip4_aaas_server_interface.hxx
   Diameter MIP AAA server, server session interfacece
   Written by Miriam Tauil
   Created October 1, 2004.
*/

#ifndef __DIAMETER_MIP_AAAS_SERVER_INTERFACE_H__
#define __DIAMETER_MIP_AAAS_SERVER_INTERFACE_H__
#include "diameter_parser.h"
#include "diameter_mip4_aaas_server_session.hxx"




/*************************************************************************
  The Mip4AaaSServer class specifies an interface for a MIP AAA Server
  (server session portion)  implementation.
  Once the AAA server implementation is implemented with the following 
  interface, the Diameter MIP AAA server sample application, will provide a 
  AAA server implementation supporting the Diameter MIP Application.

 **************************************************************************/


class Mip4AaaSServer {
   private:


  AMR_Data _amrData;
   public:

      // amrData content is necessary for this object to operate
      // it is not passed on the constructor, since it will be created
      // by the AAAServerSessionFactoryClass, and the factory class
      // will create new sessions when an AMR arrives, which will cause
      // the call of the AMR_Handler in DiameterMip4AaaSServerSession,
      // which will call SetAmrData (AMR_Data &amrData)

      Mip4AaaSServer(){}
        
      virtual ~Mip4AaaSServer(){}

      void SetAmrData (AMR_Data &amrData)
      {
	_amrData = amrData;
      }

      /** The following functions will set the passed variable to contain 
         the value set by the AAA server, if the value cannot be set 
	       return 0,
	       otherwise return 1
      **/

      /** MN-HA Nonce is created by the AAAS and submited to the HA 
	  using AMA : MN in CoA  or
	  using HAR : MN in Foreign Agent CoA 

	  The following 2 will be called if in MIP-Feature-Vector in AMR
	  MN-FA key request was set.
      **/
      // mnHaNonce => MIP-Mn-to-Ha-Msa
      virtual int SetMnHaNonce(diameter_octetstring_t &mnHaNonce)=0;  
 
      /* sent to the Ha in AMA or HAR in Mip-Ha-To-Mn-Msa */
      virtual int SetHaMnKey(diameter_octetstring_t &mipSessionKey)=0;

      virtual int SetMipMsaLifetime(diameter_unsigned32_t *mipMsaLifetime)=0;

      //virtual int SetResultCode( diameter_unsigned32_t &resultcode)=0;
      virtual int SetErrorMessage(diameter_utf8string_t &errorMessage){ return 0;}

      // The following fn will be called only in case MN is in a foreign 
      // network and SA with FA is requested (in MIP-Feature-Vector in AMR
      // MN-FA key request was set)
      // mnFaNonce => MIP-Mn-to-Fa-Msa
      virtual int SetMnFaNonce(diameter_octetstring_t &mnFaNonce)=0;  
      // sent to the Fa in AMA in Mip-Fa-To-Mn-Msa 
      virtual int SetMnFaKey(diameter_octetstring_t &mipSessionKey)=0;
      // sent to the Fa in AMA in Mip-Fa-To-Ha-Msa & to Ha in HAR
      virtual int SetFaHaKey(diameter_octetstring_t &mipSessionKey)=0;
  
  virtual void SetMipFilterRule(){} // need to be chaged ...

 // will return the Algorithm type associated with MIPv4 authorization 
  // extention. Currently defined (2) HMAC-SHA-1
  virtual void  SetAlgorithmType( diameter_unsigned32_t *mipAlgorithmType)
  {
    (*mipAlgorithmType) = 2;
  }

  // will set Replay-Mode according one of the values allowed in the I-Draft.
  // 1 None
  // 2 Timestamp
  // 3 Nonces
  virtual void SetReplayMode( diameter_enumerated_t *mipReplayMode)
  {
    (*mipReplayMode) = 1;
  }

  // returns 1, when the AAAS sets authLifetime
  // otherwisw returns 0 and this AVP will be set per the authLifetime
  // requested in AMR
  virtual int SetAuthorizationLifetime(diameter_unsigned32_t *authLifetime)=0;

  #define STATE_MAINTAINED 1
  #define NO_STATE_MAINTAINED 0
  virtual void  SetAuthState(diameter_enumerated_t *authState)
  {
    (*authState)  = STATE_MAINTAINED;
  }
  /* 
     AuthenticateUser() fn will use the AMR information (MIP-Mn-AAA-Auth) to 
     authorize the user will return 1 for authorize user or 0 otherwise.
  */
  virtual bool AuthenticateUser(std::string UserName, 
			     diameter_address_t MipMobileNodeAddress,
			     diameter_unsigned32_t MipMnAaaSpi,
			     diameter_unsigned32_t MipAuthInputDataLength,
			     diameter_unsigned32_t MipAuthenticatorLength,
			     diameter_unsigned32_t MipAuthenticatorOffset,
			     std::string MipRegRequest )=0;


  virtual int SetAaaSAllocatedHomeAgentHost(
			    DiameterScholarAttribute<diameter_identity_t> &hostname)
  {
#ifdef OPTION_1
    char _hostname[100];
    if ( ! gethostname( _hostname , 100) )
      {
	// assume HA is running on the AAAS host
	hostname.Set( _hostname); 
	return 1;
      }
    else
      return 0;
#endif
#ifdef OPTION_2
    // assume HA is running on the AAAS host - 
    // taken from config file
    hostname.Set( DIAMETER_CFG_TRANSPORT()->host);

#endif
    hostname.Set( "has.homedomain.com");
    return 1;


  }


  virtual AAAReturnCode Reset(){ return AAA_ERR_SUCCESS;}
 
  /* this fn will be call to release local resource allocated to a session
     will be called in the session fn: HandleAbort(), 
     HandleAuthGracePeriodTimeout(), HandleDisconnect, HandleSessiontimeout()
     STR received (where this is captured??)
  */
  virtual void ReleaseSessionResources(){}

  
};
 
#endif  // __DIAMETER_MIP_AAAS_SERVER_INTERFACE_H__
 
 

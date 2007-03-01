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
   diameter_specific_aaas_server_interface.hxx
   Diameter MIP Specific AAA server, for the AAA server sample application.
   Written by Miriam Tauil
   Created October 6, 2004.
*/

#ifndef __DIAMETER_MIP_SPECIFIC_AAAS_SERVER_INTERFACE_H__
#define __DIAMETER_MIP_SPECIFIC_AAAS_SERVER_INTERFACE_H__

#include "diameter_mip4_aaas_server_interface.hxx"

/*************************************************************************
  The SpecificMip4AaaSServer class implements the interface specified for 
  a MIP AAA Server.
 **************************************************************************/


class SpecificMip4AaaSServer:public Mip4AaaSServer {
   private:

   public:

      SpecificMip4AaaSServer():Mip4AaaSServer(){}
      virtual ~SpecificMip4AaaSServer(){}


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
      int SetMnHaNonce(diameter_octetstring_t &mnHaNonce) {
	mnHaNonce = "LLLLL";
	return 1;
      }
      /* sent to the Ha in AMA or HAR in Mip-Ha-To-Mn-Msa */
      int SetHaMnKey(diameter_octetstring_t &mipSessionKey) {
	mipSessionKey = "12345";
	return 1;
      }

      int SetMipMsaLifetime(diameter_unsigned32_t *mipMsaLifetime) {
	(*mipMsaLifetime) = 360; //in sec., 1 hour
  	return 1;
      }

      // The following fn will be called only in case MN is in a foreign 
      // network and SA with FA is requested (in MIP-Feature-Vector in AMR
      // MN-FA key request was set)
      // mnFaNonce => MIP-Mn-to-Fa-Msa
      int SetMnFaNonce( diameter_octetstring_t &mnFaNonce) {
	mnFaNonce = "5678";
	return 1;
      }
      // sent to the Fa in AMA in Mip-Fa-To-Mn-Msa 
      int SetMnFaKey(diameter_octetstring_t &mipSessionKey) {
	mipSessionKey = "12345";  
	return 1;
      }
      // sent to the Fa in AMA in Mip-Fa-To-Ha-Msa & to Ha in HAR
      int SetFaHaKey(diameter_octetstring_t &mipSessionKey) {
  	mipSessionKey = "12345";
	return 1;
      }

      int SetAuthorizationLifetime(diameter_unsigned32_t *authLifetime)
      {
  	(*authLifetime) = 360;
	return 1;
      }
 
  /* 
     AuthenticateUser() fn will use the AMR information (MIP-Mn-AAA-Auth) to 
     authorize the user will return 1 for authorize user or 0 otherwise.
  */
     bool AuthenticateUser(std::string UserName, 
			     diameter_address_t MipMobileNodeAddress,
			     diameter_unsigned32_t MipMnAaaSpi,
			     diameter_unsigned32_t MipAuthInputDataLength,
			     diameter_unsigned32_t MipAuthenticatorLength,
			     diameter_unsigned32_t MipAuthenticatorOffset,
			     std::string MipRegRequest)
   {
       return 1;
   }
 
 };
 
#endif  // __DIAMETER_MIP_AAAS_SERVER_INTERFACE_H__
 
 

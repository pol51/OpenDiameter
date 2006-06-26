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
/* Lesser General License for more details.                               */
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
   diameter_mip4_aaas_server_session.hxx
   Server Session definition for Diameter MIP4 AAA Server Application 
   Written by Miriam Tauil
   Created October 8th, 2004.
*/

#ifndef __MIP4_AAAS_SERVER_SESSION_H__
#define __MIP4_AAAS_SERVER_SESSION_H__

#include <string>
#include <list>
#include "ace/Synch.h"
#include "diameter_api.h"
#include "diameter_mip4_parser.hxx"
#include "diameter_mip4_aaas_server_fsm.hxx"
#include "diameter_mip4_aaas_server_sint_interface.hxx"
#include "diameter_mip4_aaas_client_session.hxx"

#include "aaa_data_defs.h"  //retrieve local realm from configuration file

/// Diameter MIP4 AAA server Server session.  This class is defined as multiple
/// inheritance, one from AAAServerSession (defined in Diameter API)
/// and the other from DiameterMip4AaaSServerStateMachine.
template<class SpecificAaaSServerSession > 
class DiameterMip4AaaSServerSession : 
  public AAAServerSession, public DiameterMip4AaaSServerStateMachine,
  public DiameterMip4AaaSServerSessionInterface
{
 public:

  /// AMR Message Handler
  //class DIAMETER_MIP4_AAAS_SERVER_EXPORTS AMR_Handler:public AAASessionMessageHandler
class AMR_Handler:public AAASessionMessageHandler
  {
  public:
    AMR_Handler(AAAApplicationCore &appCore, 
		DiameterMip4AaaSServerSession<SpecificAaaSServerSession >  &s) 
      : AAASessionMessageHandler(appCore, MipAmrCommandCode),
	session(s)
    {}
  private:
    DiameterMip4AaaSServerSession<SpecificAaaSServerSession >  &session;
    AAAReturnCode HandleMessage (AAAMessage &msg)
    {
      // Header flag check.
      if (!msg.hdr.flags.r)
	{
	  AAA_LOG(LM_ERROR, "[%N] Received AMA instead of AMR.\n");
	  return AAA_ERR_UNKNOWN_CMD;
	}

      // Parse the received message.
      AMR_Parser parser;
      parser.setAppData(&session.amrData);
      parser.setRawData(&msg);

      try {
	parser.parseRawToApp();
      }
      catch ( DiameterParserError ) 
	{
	  AAA_LOG(LM_ERROR, "[%N] Payload error.\n");
	  return AAA_ERR_PARSING_ERROR;
	}

      session.Notify(DiameterMip4AaaSServerStateMachine::EvRxAMR);
      return AAA_ERR_SUCCESS;
   
    }
  };

  /// Constuctor.
  DiameterMip4AaaSServerSession( AAAApplicationCore &appCore, 
			 diameter_unsigned32_t appId=Mip4ApplicationId):
    AAAServerSession(appCore, Mip4ApplicationId),  
    DiameterMip4AaaSServerStateMachine(*this, appCore.GetTask().JobHandle()),
    specificAaaSServerSession(*new SpecificAaaSServerSession()),
    clientSession( *new DiameterMip4AaaSClientSession( appCore, *this))
  {

    requestHandler = (new AMR_Handler(appCore, *this));
    
    // Register the AMR message handler
    if (RegisterMessageHandler( requestHandler) != AAA_ERR_SUCCESS)
    {
      AAA_LOG(LM_ERROR, "[%N] AMR_Handler registration failed.\n");
      throw -1; 
    }
  }

  /// Destructor.
  ~DiameterMip4AaaSServerSession() 
  {
    delete (&specificAaaSServerSession);
    delete (&clientSession);
    delete requestHandler;
  }
  

  // this is for aaas client session to pass information back to this session  
  // part of implementation of the DiameterMip4AaaSServerSessionInterface
  void ServerSessionNotify (AAA_Event ev)
  {
    Notify(ev);
  }

  //this is for the aaas server fsm to notify aaas client session to send HAR
  void NotifyClientSession( int event)
  {
    //clientSession.Notify( event);
  }
  

  /// Returns the pointer to myself.
  DiameterMip4AaaSServerSession* Self() { return this; }

  /// Reimplemented from AAAServerSession. 
  AAAReturnCode HandleMessage(AAAMessage &msg)
  {
    AAA_LOG(LM_ERROR, "[%N] Unknown command.\n");
    return AAA_ERR_UNKNOWN_CMD;
  }

  /// Reimplemented from AAAServerSession. 
  AAAReturnCode HandleDisconnect()
  {
    AAA_LOG(LM_ERROR, "[%N] Session termination event received.\n");
    Notify(DiameterMip4AaaSServerStateMachine::EvSgDisconnect);
    return AAA_ERR_SUCCESS; 
  }

  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleSessionTimeout()
  { 
    AAA_LOG(LM_ERROR, "[%N] Session timeout received.\n");
    Notify(DiameterMip4AaaSServerStateMachine::EvSgSessionTimeout);
    return AAA_ERR_SUCCESS; 
  }
          
  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleAuthLifetimeTimeout()
  { 
    AAA_LOG(LM_ERROR, "[%N] Timeout received.\n");
    Notify(DiameterMip4AaaSServerStateMachine::EvSgAuthLifetimeTimeout);
    return AAA_ERR_SUCCESS; 
  }
                                                                               
  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleAuthGracePeriodTimeout()
  { 
    AAA_LOG(LM_ERROR, "[%N] Timeout received.\n");
    Notify(DiameterMip4AaaSServerStateMachine::EvSgAuthGracePeriodTimeout);
    return AAA_ERR_SUCCESS; 
  }

  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleAbort() { return AAA_ERR_SUCCESS; }

  /// Reimplemented from AAAServerSession. This is a general timeout
  /// event handler.
  AAAReturnCode HandleTimeout()
  { 
    AAA_LOG(LM_ERROR, "[%N] General timeout received.\n");
    Notify(DiameterMip4AaaSServerStateMachine::EvSgTimeout);
    return AAA_ERR_SUCCESS; 
  }

  void Start() throw (AAA_Error)
  {
    DiameterMip4AaaSServerStateMachine::Start();

  }

  AAAReturnCode Reset() throw (AAA_Error)
  {    
    AAAServerSession::Reset();  //Start();

    //sets the session state to StInitialize
    DiameterMip4AaaSServerStateMachine::Restart();
   
    return( specificAaaSServerSession.Reset() );
  }

  int SetMnHaNonce( diameter_octetstring_t &mnHaNonce)     //MnHaNonce for Mn
  {
    return specificAaaSServerSession.SetMnHaNonce( mnHaNonce);
    //return 1;
  }
  int SetHaMnKey(  AAA_ScholarAttribute<diameter_octetstring_t> &mipSessionKey)  //MnHaKey for HA
  {
    diameter_octetstring_t _mipSessionKey;
    int rc = specificAaaSServerSession.SetHaMnKey( _mipSessionKey);
    if (rc)
      mipSessionKey.Set( _mipSessionKey);
    return rc;
    
  }
  int SetAlgorithmType( AAA_ScholarAttribute<diameter_unsigned32_t> &mipAlgorithmType)
  {
    diameter_unsigned32_t _mipAlgorithmType;
    specificAaaSServerSession.SetAlgorithmType( &_mipAlgorithmType);
    mipAlgorithmType.Set( _mipAlgorithmType);
    return 1;
  }

  int SetReplayMode( AAA_ScholarAttribute<diameter_unsigned32_t> &mipReplayMode)
  {
    diameter_unsigned32_t _mipReplayMode;
    specificAaaSServerSession.SetReplayMode( &_mipReplayMode);
    mipReplayMode.Set( _mipReplayMode);
    return 1;
  }

  int SetAuthorizationLifetime(
	       AAA_ScholarAttribute<diameter_unsigned32_t>&authLifetime)
  {
    diameter_unsigned32_t _authorizationLifetime;
    if ( specificAaaSServerSession.SetAuthorizationLifetime(
					 &_authorizationLifetime) == 0 )
	authLifetime.Set(0);
    else
        authLifetime.Set(_authorizationLifetime);
    return 1;
  }

  void SetAuthState( AAA_ScholarAttribute<diameter_enumerated_t> &state)
  {
    diameter_enumerated_t _state;
    specificAaaSServerSession.SetAuthState( &_state);
    state.Set( _state);
  }
 bool AuthenticateUser(std::string UserName, 
			     diameter_address_t MipMobileNodeAddress,
			     diameter_unsigned32_t MipMnAaaSpi,
			     diameter_unsigned32_t MipAuthInputDataLength,
			     diameter_unsigned32_t MipAuthenticatorLength,
			     diameter_unsigned32_t MipAuthenticatorOffset,
			     std::string MipRegRequest )
  {
    return ( specificAaaSServerSession.AuthenticateUser( 
	 UserName,  MipMobileNodeAddress,
	 MipMnAaaSpi, MipAuthInputDataLength, MipAuthenticatorLength,
	 MipAuthenticatorOffset, MipRegRequest ) );
  }

  int SetMipMsaLifetime( 
	       AAA_ScholarAttribute<diameter_unsigned32_t>&mipMsaLifetime)
  {
      diameter_unsigned32_t _mipMsaLifetime;
      specificAaaSServerSession.SetMipMsaLifetime( &_mipMsaLifetime);
      mipMsaLifetime.Set(_mipMsaLifetime);
      return 1;
  }

  int SetErrorMessage(diameter_utf8string_t &errorMessage)
  {
    return (specificAaaSServerSession.SetErrorMessage( errorMessage));
  }

  bool IsItLocalRealm(diameter_identity_t realm)
  {

    // get local realm from config file
     if (realm == AAA_CFG_TRANSPORT()->realm)
       return 1;
     else 
       return 0;
  }

  // mnFaNonce => MIP-Mn-to-Fa-Msa
  int SetMnFaNonce(diameter_octetstring_t &mnFaNonce)
  {
    return ( specificAaaSServerSession.SetMnFaNonce( mnFaNonce) );
  }
  
  // sent to the Fa in AMA in Mip-Fa-To-Mn-Msa 
  int SetMnFaKey(diameter_octetstring_t &mipSessionKey)
  {
    return ( specificAaaSServerSession.SetMnFaKey( mipSessionKey) );
  }
  
  // sent to the Fa in AMA in Mip-Fa-To-Ha-Msa & to Ha in HAR
  int SetFaHaKey(diameter_octetstring_t &mipSessionKey)
  {
    return ( specificAaaSServerSession.SetFaHaKey( mipSessionKey) );
  }
  
  
  int SetAaaSAllocatedHomeAgentHost(
			 AAA_ScholarAttribute<diameter_identity_t> &hostname)
  {
    //return 
      (specificAaaSServerSession.SetAaaSAllocatedHomeAgentHost(hostname));
    printf ("hostname %s\n", hostname().data());
    return 1;
  }
  

  AMR_Data amrData;
  AMA_Data amaData;
  HAR_Data harData;
  HAA_Data haaData;

  AMR_Data& AMR() { return amrData; }
  AMA_Data& AMA() { return amaData; }

  HAR_Data& HAR() { return harData; }
  HAA_Data& HAA() { return haaData; }

 protected:
 private:

  SpecificAaaSServerSession &specificAaaSServerSession;
  DiameterMip4AaaSClientSession &clientSession;

  AMR_Handler *requestHandler;

};

#endif

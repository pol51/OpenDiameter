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
   diameter_mip4_ha_server_session.hxx
   HA Server Session definition for Diameter MIP4 Application 
   Written by Miriam Tauil
   Created December 20th, 2004.
*/

#ifndef __MIP4_HA_SERVER_SESSION_H__
#define __MIP4_HA_SERVER_SESSION_H__

#include <string>
#include <list>
#include "ace/Synch.h"
#include "diameter_api.h"
#include "diameter_mip4_parser.hxx"
#include "diameter_mip4_ha_server_fsm.hxx"
//#include "diameter_mip4_ha_server_interface.hxx"

/// Diameter MIP4 HA server Server session.  This class is defined as multiple
/// inheritance, one from AAAServerSession (defined in Diameter API)
/// and the other from DiameterMip4HaServerStateMachine.

template<class SpecificHaServerSession > 
class DiameterMip4HaServerSession : 
  public AAAServerSession, public DiameterMip4HaServerStateMachine
{
 public:

  /// HAR Message Handler
  //class DIAMETER_MIP4_HA_SERVER_EXPORTS HAR_Handler:public AAASessionMessageHandler
class HAR_Handler:public AAASessionMessageHandler
  {
  public:
    HAR_Handler(AAAApplicationCore &appCore, 
		DiameterMip4HaServerSession<SpecificHaServerSession >  &s) 
      : AAASessionMessageHandler(appCore, MipHarCommandCode),
	session(s)
    {}
  private:
    DiameterMip4HaServerSession<SpecificHaServerSession >  &session;
    AAAReturnCode HandleMessage (DiameterMsg &msg)
    {

      // Header flag check.
      if (!msg.hdr.flags.r)
	{
	  AAA_LOG((LM_ERROR, "[%N] Received HAA instead of HAR.\n"));
	  return AAA_ERR_UNKNOWN_CMD;
	}

      // Parse the received message.
      HAR_Parser parser;
      parser.setAppData(&session.harData);
      parser.setRawData(&msg);

      try {
	parser.parseRawToApp();
      }
      catch ( DiameterParserError ) 
	{
	  AAA_LOG((LM_ERROR, "[%N] Payload error.\n"));
	  return AAA_ERR_PARSING_ERROR;
	}
#ifdef PRINT_MSG_CONTENT
  printf("HAR:\nAuthorization Lifetime: %d\nMip-Reg-Request: %s\nUser Name: %s\nDestination Realm: %s\nDestination Host: %s\n\n", 
	 session.harData.AuthorizationLifetime(), 
	 session.harData.MipRegRequest().data(), 
	 session.harData.UserName().data(),
	 session.harData.DestinationRealm().data(),
	 session.harData.DestinationHost().data() );
#endif
       session.Notify(DiameterMip4HaServerStateMachine::EvRxHAR);
      
      return AAA_ERR_SUCCESS;
   
    }
  };

  /// Constuctor.
  DiameterMip4HaServerSession( AAAApplicationCore &appCore, 
			 diameter_unsigned32_t appId=Mip4ApplicationId):
    AAAServerSession(appCore, Mip4ApplicationId),  
    DiameterMip4HaServerStateMachine(*this, appCore.GetTask().JobHandle()),
    specificHaServerSession(*new SpecificHaServerSession())
  {

    requestHandler = (new HAR_Handler(appCore, *this));
    
    // Register the HAR message handler
    if (RegisterMessageHandler( requestHandler) != AAA_ERR_SUCCESS)
    {
      AAA_LOG((LM_ERROR, "[%N] HAR_Handler registration failed.\n"));
      throw -1; 
    }

  }

  /// Destructor.
  virtual ~DiameterMip4HaServerSession() 
  {
    delete (&specificHaServerSession);
    delete requestHandler;
  }
  
  /// Returns the pointer to myself.
  DiameterMip4HaServerSession* Self() { return this; }

  /// Reimplemented from AAAServerSession. 
  AAAReturnCode HandleMessage(DiameterMsg &msg)
  {
    AAA_LOG((LM_ERROR, "[%N] Unknown command.\n"));
    return AAA_ERR_UNKNOWN_CMD;
  }

  /// Reimplemented from AAAServerSession. 
  AAAReturnCode HandleDisconnect()
  {
    AAA_LOG((LM_ERROR, "[%N] Session termination event received.\n"));
    Notify(DiameterMip4HaServerStateMachine::EvSgDisconnect);
    return AAA_ERR_SUCCESS; 
  }

  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleSessionTimeout()
  { 
    AAA_LOG((LM_ERROR, "[%N] Session timeout received.\n"));
    Notify(DiameterMip4HaServerStateMachine::EvSgSessionTimeout);
    return AAA_ERR_SUCCESS; 
  }
          
  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleAuthLifetimeTimeout()
  { 
    AAA_LOG((LM_ERROR, "[%N] Timeout received.\n"));
    Notify(DiameterMip4HaServerStateMachine::EvSgAuthLifetimeTimeout);
    return AAA_ERR_SUCCESS; 
  }
                                                                               
  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleAuthGracePeriodTimeout()
  { 
    AAA_LOG((LM_ERROR, "[%N] Timeout received.\n"));
    Notify(DiameterMip4HaServerStateMachine::EvSgAuthGracePeriodTimeout);
    return AAA_ERR_SUCCESS; 
  }

  /// Reimplemented from AAAServerSession.
  AAAReturnCode HandleAbort() { return AAA_ERR_SUCCESS; }

  /// Reimplemented from AAAServerSession. This is a general timeout
  /// event handler.
  AAAReturnCode HandleTimeout()
  { 
    AAA_LOG((LM_ERROR, "[%N] General timeout received.\n"));
    Notify(DiameterMip4HaServerStateMachine::EvSgTimeout);
    return AAA_ERR_SUCCESS; 
  }

  void Start() throw (AAA_Error)
  {
    DiameterMip4HaServerStateMachine::Start();
  }

  AAAReturnCode Reset() throw (AAA_Error)
  {    
    AAAServerSession::Reset();  //Start();

    //sets the session state to StInitialize
    DiameterMip4HaServerStateMachine::Restart();
   
    return ( specificHaServerSession.Reset() );
    
  }

  // implementation of the virtual functions defined in ha_session_fsm
  // by calling the functions of the SpecificHaServer

  int ProcessMipRegRequest( diameter_octetstring_t &MipRegReq )
  {
    return specificHaServerSession.ProcessMipRegRequest( MipRegReq );
  }

  int SetMipFaToHaSpi(diameter_unsigned32_t &faHaSpi)
  {
    return specificHaServerSession.SetMipFaToHaSpi( faHaSpi);
  }

  int SetMipFaToMnSpi(diameter_unsigned32_t &faMnSpi)
  {
    return specificHaServerSession.SetMipFaToMnSpi( faMnSpi);
  }

  /****
  int SetHaMnKey(  DiameterScholarAttribute<diameter_octetstring_t> &mipSessionKey)
  {
    diameter_octetstring_t _mipSessionKey;
    int rc = specificAaaSServerSession.SetHaMnKey( _mipSessionKey);
    if (rc)
      mipSessionKey.Set( _mipSessionKey);
    return rc;
  }
  ********/
  int SetErrorMessage(DiameterScholarAttribute<diameter_utf8string_t> &errorMessage)
  {
    diameter_utf8string_t _errorMessage;
    int rc;
    if ((rc = specificHaServerSession.SetErrorMessage( _errorMessage)))
      errorMessage.Set( _errorMessage);
    return rc;
  }

  int SetMipRegReply(DiameterScholarAttribute<diameter_octetstring_t> &reply)
  {
    diameter_octetstring_t _reply="";
    int rc=0;
    if ((rc = specificHaServerSession.SetMipRegReply( _reply )))
      reply.Set( _reply);
    return rc;
  }

  // is called if MN address does not appear in HAR
  int SetMipMnAddress(DiameterScholarAttribute<diameter_address_t> &address)
  {
    diameter_address_t _address;
    int rc;
    if ((rc = specificHaServerSession.SetMipMnAddress( _address)))
      address.Set(_address);
    return rc;
  }

  // Must be populated by HA
  int SetAcctMultiSessionId( DiameterScholarAttribute<diameter_utf8string_t> &acctMultiSessionId)
  {
    diameter_utf8string_t _acctMultiSessionId;
    int rc;
    if ((rc =
	 specificHaServerSession.SetAcctMultiSessionId( _acctMultiSessionId)))
      acctMultiSessionId.Set( _acctMultiSessionId);
    return rc;
  }

  HAR_Data harData;
  HAA_Data haaData;

  HAR_Data& HAR() { return harData; }
  HAA_Data& HAA() { return haaData; }

 protected:
 private:

  SpecificHaServerSession &specificHaServerSession;

  HAR_Handler *requestHandler;

};

#endif

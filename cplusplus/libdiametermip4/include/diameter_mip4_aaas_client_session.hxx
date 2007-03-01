/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* OpenDiameter: Open-source software for the Diameter protocol           */
/*                                                                        */
/* Copyright (C) 2007  Open Diameter Project.                             */
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
   diameter_mip4_aaas_client_session.hxx
   "AAA Server" Client Session definition for Diameter MIP 4 Application 
   Written by Miriam Tauil
   Created June 30, 2004.
*/

#ifndef __MIP4_AAAS_CLIENT_SESSION_H__
#define __MIP4_AAAS_CLIENT_SESSION_H__

#include <string>
#include <list>
#include "ace/Synch.h"
#include "diameter_api.h"

#include "diameter_mip4_parser.hxx"
#include "diameter_mip4_aaas_client_fsm.hxx"
#include "diameter_mip4_aaas_server_fsm.hxx"
#include "diameter_mip4_aaas_server_sint_interface.hxx"


/// Diameter MIP  Client session.  This class is defined as multiple
/// inheritance,  from AAAClientSession (defined in Diameter API),
/// DiameterMip4AaaSClientStateMachine 

//class DIAMETER_MIP4_AAAS_CLIENT_EXPORTS DiameterMip4AaaSClientSession:public AAAClientSession, public DiameterMip4AaaSClientStateMachine

class  DIAMETER_MIP4_AAAS_CLIENT_EXPORTS DiameterMip4AaaSClientSession:
  public AAAClientSession, public DiameterMip4AaaSClientStateMachine
{
 public:

  //class DIAMETER_MIP4_HAA_HANDLER_EXPORTS HAA_Handler : 
  class HAA_Handler : 
    public AAASessionMessageHandler
  {
    friend class  DiameterMip4AaaSClientSession;
  public:
    HAA_Handler(AAAApplicationCore &appCore,
		DiameterMip4AaaSClientSession  &s) 
      : AAASessionMessageHandler(appCore, MipHarCommandCode),
	session(s)
    {}

  private:  
    DiameterMip4AaaSClientSession &session;

    AAAReturnCode HandleMessage (DiameterMsg &msg)
    {

    // Header flag check.
      if (msg.hdr.flags.r)
	{
	  AAA_LOG((LM_ERROR, "[%N] Received HAR instead of HAA.\n"));
	  return AAA_ERR_UNKNOWN_CMD;
	}

      // Parse the received message.
      HAA_Parser parser;
      parser.setAppData( &(session.serverSession.HAA()));
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

    printf("HAA:\nResult-Code: %d\nMip-Reg-Reply:  %s\nUser-Name:%s\n \n", 
	 session.serverSession.HAA().ResultCode(),
	 session.serverSession.HAA().MipRegReply().data(), 
	 session.serverSession.HAA().UserName().data());
#endif
      session.Notify(DiameterMip4AaaSClientStateMachine::EvRxHAA);
      //terminates session
      return AAA_ERR_SUCCESS;
    }
  };

  HAA_Handler *answerHandler; 

  /// Constuctor.
  DiameterMip4AaaSClientSession(AAAApplicationCore &appCore, DiameterMip4AaaSServerSessionInterface &s): AAAClientSession(appCore, Mip4ApplicationId),  
    DiameterMip4AaaSClientStateMachine(*this, appCore.GetTask().JobHandle()),
    serverSession(s) 
  {
    answerHandler = (new HAA_Handler(appCore, *this));
    
    // Register the HAA message handler
    if (RegisterMessageHandler( answerHandler) != AAA_ERR_SUCCESS)
    {
      AAA_LOG((LM_ERROR, "[%N] HAA_Handler registration failed.\n"));
      throw -1; 
    }

    //initiate HAR data structures 
    serverSession.HAA().Clear();
    Start();
}

  /// Destructor.
  ~DiameterMip4AaaSClientSession() 
  {
    delete answerHandler;
  }


  DiameterMip4AaaSClientSession* Self() { return this; }

  /// Reimplemented from AAAClientSession. This is invoked during
  /// incomming message events. The msg argument is pre-allocated by
  /// the library and is valid only within the context of this
  /// method. It is the responsibility of the derived class to
  /// override this function and capture the events if it is
  /// interested in it.
  AAAReturnCode HandleMessage(DiameterMsg &msg)
  {
    AAA_LOG((LM_ERROR, "[%N] Unknown command.\n"));
    return AAA_ERR_UNKNOWN_CMD;
  }

  /// Reimplemented from AAAClientSession. This is invoked during
  /// session disconnect event. Disconnection occurs when a session is
  /// terminated or the peer connection is disconnection and unable to
  /// recover. It is the responsibility of the derived class to
  /// override this function and capture this events if it is
  /// interested in it.
  AAAReturnCode HandleDisconnect()
  { 
    AAA_LOG((LM_ERROR, "[%N] Session termination event received.\n"));
    Notify(DiameterMip4AaaSClientStateMachine::EvSgDisconnect);
    return AAA_ERR_SUCCESS; 
  }
  /// Reimplemented from AAAClientSession.
  AAAReturnCode HandleSessionTimeout()
  { 
    AAA_LOG((LM_ERROR, "[%N] Session timeout received.\n"));
    Notify(DiameterMip4AaaSClientStateMachine::EvSgSessionTimeout);
    return AAA_ERR_SUCCESS; 
  }
                                     
  /// Reimplemented from AAAClientSession.
  AAAReturnCode HandleAuthLifetimeTimeout()
  { 
    AAA_LOG((LM_ERROR, "[%N] Timeout received.\n"));
    Notify(DiameterMip4AaaSClientStateMachine::EvSgAuthLifetimeTimeout);
    return AAA_ERR_SUCCESS; 
  }

  /// Reimplemented from AAAClientSession.
  AAAReturnCode HandleAuthGracePeriodTimeout()
  { 
    AAA_LOG((LM_ERROR, "[%N] Timeout received.\n"));
    Notify(DiameterMip4AaaSClientStateMachine::EvSgAuthGracePeriodTimeout);
    return AAA_ERR_SUCCESS; 
  }

  /// Reimplemented from AAAClientSession. This is invoked during
  /// session abort event. Abort occurs when the server decides to
  /// terminate the client session by sending an ASR. It is the
  /// responsibility of the derived class to override this function
  /// and capture this events if it is interested in it.
  AAAReturnCode HandleAbort() { return AAA_ERR_SUCCESS; }

  void Start() throw (AAA_Error)
  {
    DiameterMip4AaaSClientStateMachine::Start();
    AAAClientSession::Start();
  }
  
  AAAReturnCode Reset() throw (AAA_Error)
  {    
    AAAClientSession::Start();

    //sets the session state to StInitialize
    // DiameterMip4AaaSClientStateMachine::Restart();  ??? Commented out 1/3/05
    return AAA_ERR_SUCCESS;
  }
  
  // The implementation of this function in a child class is optional.
  // The function's purpose is to provide an interface for memory deallocation
  // of the DiameterMip4AaaSClientSession, when it is allocated using the "new"
  // operator. 
  //  virtual void Abort(){} commented out 1/3/05

  // these vars are initiated in the constructor
  DiameterMip4AaaSServerSessionInterface &serverSession;

  HAR_Data& HAR() { return serverSession.HAR(); }
  HAA_Data& HAA() { return serverSession.HAA(); }

};


#endif  // __MIP4_HA_CLIENT_SESSION_H__

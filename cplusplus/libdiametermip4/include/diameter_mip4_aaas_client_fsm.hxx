/* BEGIN_COPYRIGHT                                                        */
/*                                                                        */
/* OpenDiameter: Open-source software for the Diameter protocol           */
/*                                                                        */
/* Copyright (C) 2002 -2007  Open Diameter Project 		          */   
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
   diameter_mip4_aaas_client_fsm.hxx
   Client Statemachine definition for Diameter MIP AAA Server Application 
   Written by Miriam Tauil
   Created October 21, 2004.
*/

#ifndef __MIP4_AAAS_CLIENT_FSM_H__
#define __MIP4_AAAS_CLIENT_FSM_H__

#include "framework.h"
#include "diameter_api.h"
#include "diameter_mip4_parser.hxx"

/*!
 * Windows specific export declarations
 */
#ifdef WIN32
   #if defined(DIAMETER_MIP4_AAAS_CLIENT_EXPORT)
       #define DIAMETER_MIP4_AAAS_CLIENT_EXPORTS __declspec(dllexport)
   #else
       #define DIAMETER_MIP4_AAAS_CLIENT_EXPORTS __declspec(dllimport)
   #endif
#else
   #define DIAMETER_MIP4_AAAS_CLIENT_EXPORTS
   #define DIAMETER_MIP4_AAAS_CLIENT_EXPORTS
#endif
typedef AAA_JobHandle<AAA_GroupedJob> DiameterJobHandle;


/// State machine for Diameter MIP "AAA Server" client. It is used
/// to send the HAR message and accept its response.
/// This class is internal to the library since it is used by the "AAA Server"
/// session class. 

// forward declaration is needed instead of including the header file to avoid 
// a circular dependency
class DiameterMip4AaaSClientSession;

//class DIAMETER_MIP4_AAAS_CLIENT_EXPORTS DiameterMip4AaaSClientStateMachine 
class DiameterMip4AaaSClientStateMachine 
  : public AAA_StateMachine<DiameterMip4AaaSClientStateMachine>,
    public AAA_EventQueueJob
{
  friend class DiameterJobMultiplexor;

public:
   
  /// Constructor.
  DiameterMip4AaaSClientStateMachine( DiameterMip4AaaSClientSession &s,
	 DiameterJobHandle &h);

  ~DiameterMip4AaaSClientStateMachine() 
  {
    handle.Job().Remove(this); 
  }
  
  //expose value of protected variable from DiameterMip4AaaSClientStateMachine 
  diameter_enumerated_t State()
  {
    return AAA_StateMachine<DiameterMip4AaaSClientStateMachine>::state; 
  }
  
  // static diameter_enumerated_t InitialState()
  //{
  //  return state::StInitialize;
  //}
 

  // This is a copy from diameter_mip4_aaas_client_fsm.cxx
  // DiameterMip4AaaSClientStateTable_S, since I couldn't expose StInitialize
  // to be retuned to the application as a sign for a free session object.

  enum state {
    StInitialize //,
    //StWaitHAA,
    //StCheckResultCode,
    //StAccepted,
    //StRejected,
    //StTerminated
  };

  enum {
    EvSendHAR,
    EvRxHAA,
    EvSgSuccess,        // HAA result
    EvSgLimitedSuccess, //  
    EvSgFailure,        // HAA result 
    EvSgSessionTimeout, 
    EvSgAuthLifetimeTimeout,
    EvSgAuthGracePeriodTimeout,
    EvSgDisconnect,
    EvSgReset           //Reset session to reuse object 
  };

  /// Store an event and notify the session.
  //inline 
  void Notify(AAA_Event ev) {
    // Enqueue the event.
    AAA_EventQueueJob::Enqueue(ev);

    // Notify the session.
    handle.Job().Schedule(this);
  }

  inline DiameterMip4AaaSClientSession& Session() { return session; }
  

  /// This is used for sending a Diameter-HAR-Request.
  void SendHAR();

  inline AAA_JobData& JobData() { return *handle.Job().Data(); }

  template <class T> inline T& JobData(Type2Type<T>) 
  { return (T&)*handle.Job().Data(); }
  

 protected:
 private:
  DiameterMip4AaaSClientSession &session;

  /// Job handle.
  DiameterJobHandle handle;


  /// Inherited from AAA_EventQueueJob.
  int Schedule(AAA_Job *job, size_t=1) { return (-1); }

  /// Inherited from AAA_EventQueueJob.
  inline int Serve()
  {
    if (!AAA_EventQueueJob::ExistBacklog())
      {
	AAA_LOG((LM_ERROR, "%N: no backlog to serve."));
	return 0;
      }

    // Obtain the event to execute.
    AAA_Event ev = 0;
    AAA_EventQueueJob::Dequeue(ev);

    bool existBacklog = AAA_EventQueueJob::ExistBacklog();

    // Execute it.
    Event(ev);
    return existBacklog ? 1 : 0;
  }

};

#endif //  __MIP4_AAAS_CLIENT_FSM_H__

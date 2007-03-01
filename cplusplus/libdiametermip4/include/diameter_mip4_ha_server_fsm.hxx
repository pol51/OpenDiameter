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

/* $Id: */
/* 
   diameter_mip4_ha_server_fsm.hxx
   Home Agent Server session (Accepts HAR) Statemachine definition 
   for Diameter MIP4 App. 
   Written by Miriam Tauil
   Created December 20, 2004.
*/

#ifndef __HA_SERVER_FSM_H__
#define __HA_SERVER_FSM_H__

#include "framework.h"
#include "diameter_api.h"
#include "diameter_mip4_parser.hxx"

#ifdef WIN32
   #if defined(DIAMETER_MIP4_HA_SERVER_EXPORT)
       #define DIAMETER_MIP4_HA_SERVER_EXPORTS __declspec(dllexport)
   #else
       #define DIAMETER_MIP4_HA_SERVER_EXPORTS __declspec(dllimport)
   #endif
#else
   #define DIAMETER_MIP4_HA_SERVER_EXPORTS
#endif

typedef AAA_JobHandle<AAA_GroupedJob> DiameterJobHandle;



/// State machine for Diameter MIP4 HA server session (Accepts HAR).  

//class DIAMETER_MIP4_HA_SERVER_EXPORTS DiameterMip4HaServerStateMachine 
class DiameterMip4HaServerStateMachine : 
  public AAA_StateMachine<DiameterMip4HaServerStateMachine>,
  public AAA_EventQueueJob
{
  friend class DiameterJobMultiplexor;

 public:
  /// Constructor.
  DiameterMip4HaServerStateMachine(AAAServerSession& s,
				DiameterJobHandle &h);

  virtual  ~DiameterMip4HaServerStateMachine();
  //  {
  //  handle.Job().Remove(this); 
  //}

  enum {
    EvRxHAR,
    EvSgRegRequestAccepted,
    EvSgRegRequestRejected,
    EvSgInvalidRegRequest,
    EvSgHAASent,  //is it needed?
    EvSgSessionTimeout,
    EvSgAuthLifetimeTimeout,
    EvSgAuthGracePeriodTimeout,
    EvSgTimeout,
    EvSgDisconnect
  };

  /// Store an event and notify the session.
  inline void Notify(AAA_Event ev) throw (int) {
    // Enqueue the event.
    if (AAA_EventQueueJob::Enqueue(ev) <= 0)
      Abort();

    if (handle.Job().Schedule(this) < 0)
      Abort();
  }

  /// This is used for obtaining the reference to the server session
  /// object.
  inline AAAServerSession& Session() { return session; }

  /// This is used for aborting the state machine.  Usually called
  /// when Notify() fails.
  virtual void Abort()
  {
    AAA_LOG((LM_DEBUG, "Diameter HA server session aborted.\n"));
    //Abort();  WHAT SHOULD BE DONE HERE ??
  }

  /// This is used for constructing and sending response to HAR
  void SendHAA();

  /* The following virtual functions provide the state machine with information
     needed in the state machine that is provided by the specific
     implementation of the Home Agent.
     This are implemented in the HA Server Session class by calling the 
     specificHaServer functions.
  */

  virtual int ProcessMipRegRequest( diameter_octetstring_t &diameterMipRegRequest )=0;
  // called if FA-Ha-Key-Requested: generated by HA
  virtual int SetMipFaToHaSpi(diameter_unsigned32_t &faHaSpi)=0;  
 
  // called if MN-FA-Key-Requested: HA extract value from MipRegRequest
  virtual int SetMipFaToMnSpi(diameter_unsigned32_t &faMnSpi)=0;

  virtual int SetErrorMessage(DiameterScholarAttribute<diameter_utf8string_t> &errorMessage)=0;

  virtual int SetMipRegReply(DiameterScholarAttribute<diameter_octetstring_t> &reply)=0;

 // is called if MN address does not appear in HAR
 virtual int SetMipMnAddress(DiameterScholarAttribute<diameter_address_t> &address)=0;

  // Must be populated by HA
  virtual int SetAcctMultiSessionId( DiameterScholarAttribute<diameter_utf8string_t> &acctMultiSessionId)=0;


  inline AAA_JobData& JobData() { return *handle.Job().Data(); }

  template <class T> inline T& JobData(Type2Type<T>) 
  { return (T&)*handle.Job().Data(); }


  // implemented in ha_server_session.hxx
  virtual HAR_Data& HAR()=0;
  virtual HAA_Data& HAA()=0;

 protected:

 private:
  /// Inherited from AAA_EventQueueJob.  Not used.
  int Schedule(AAA_Job*, size_t=1) { return (-1); }

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

  AAAServerSession& session;  
  /// Job handle.
  DiameterJobHandle handle;

};

#endif

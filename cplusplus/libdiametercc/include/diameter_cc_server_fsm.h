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
   diameter_cc_server_fsm.h
   Server Statemachine definition for Diameter Credit Control Application 
*/

#ifndef __CC_SERVER_FSM_H__
#define __CC_SERVER_FSM_H__

#include "framework.h"
#include "diameter_cc_parser.h"
#include "diameter_cc_account.h"

#ifdef WIN32
   #if defined(DIAMETER_CC_SERVER_EXPORT)
       #define DIAMETER_CC_SERVER_EXPORTS __declspec(dllexport)
   #else
       #define DIAMETER_CC_SERVER_EXPORTS __declspec(dllimport)
   #endif
#else
   #define DIAMETER_CC_SERVER_EXPORTS
   #define DIAMETER_CC_SERVER_EXPORTS
#endif

typedef AAA_JobHandle<AAA_GroupedJob> DiameterCCJobHandle;

class DiameterCCServerSession;

class DIAMETER_CC_SERVER_EXPORTS DiameterCCServerStateMachine 
  : public AAA_StateMachine<DiameterCCServerStateMachine>,
    public AAA_EventQueueJob
{
 public:
  /// Constructor.
  DiameterCCServerStateMachine(DiameterCCServerSession& s,
                               DiameterCCJobHandle &h);

  ~DiameterCCServerStateMachine() 
  {
    handle.Job().Remove(this); 
  }

  enum {
    EvInitialRequest,
    EvEventRequest,
    EvValidInitialRequest,
    EvInvalidInitialRequest,
    EvInitialRequestSuccessful,
    EvInitialRequestUnsuccessful,
    EvValidEventRequest,
    EvInvalidEventRequest,
    EvEventRequestSuccessful,
    EvEventRequestUnsuccessful,
    EvUpdateRequest,
    EvTerminationRequest,
    EvTccExpired,
    EvValidUpdateRequest,
    EvInvalidUpdateRequest,
    EvValidTerminationRequest,
    EvInvalidTerminationRequest,
    EvUpdateRequestSuccessful,
    EvUpdateRequestUnsuccessful,
    EvTerminationRequestSuccessful,
    EvTerminationRequestUnsuccessful,
    EvSgDisconnect,
    EvSgSessionTimeout,
    EvSgTimeout

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
  inline DiameterCCServerSession& Session() { return session; }

  /// This is used for aborting the state machine.  Usually called
  /// when Notify() fails.
  virtual void Abort()=0;

  /// This is used for constructing and sending an CCA.
  void SendCCA();

  /// Validate Initial Request.  
  virtual bool ValidateInitialRequest() {return true;};

  /// Validate Update Request.  
  virtual bool ValidateUpdateRequest() {return true;};

  /// Validate Event Request.  
  virtual bool ValidateEventRequest() {return true;};

  /// Validate Termination Request.  
  virtual bool ValidateTerminationRequest() {return true;};

  /// Initial Request action.  
  virtual bool InitialRequest() {return true;};

  virtual bool InitialAnswer() {return true;};

  /// Update Request action.  
  virtual bool UpdateRequest() {return true;};

  /// Termination Request action.  
  virtual bool TerminationRequest() {return true;};

  /// Event Request action.  
  virtual bool EventRequest() {return true;};

  inline AAA_JobData& JobData() { return *handle.Job().Data(); }

  template <class T> inline T& JobData(Type2Type<T>) 
  { return (T&)*handle.Job().Data(); }


  /// The contents of the replyMessage should be generated depending
  /// on the value of the resultCode. 
  virtual void SetReplyMessage
  (DiameterVectorAttribute<diameter_utf8string_t> &replyMessage, 
   const diameter_unsigned32_t &resultCode)
  {}


  inline DiameterCCAccount& Account()
  { return *accountInfo; }

  inline void Account(const DiameterCCAccount& acc)
  { accountInfo = 
      std::auto_ptr<DiameterCCAccount>(new DiameterCCAccount(acc));
  }


  inline CCR_Data& CCR_DATA() { return ccrData; }
  inline CCA_Data& CCA_DATA() { return ccaData; }

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

  DiameterCCServerSession& session;

  // Job handle.
  DiameterCCJobHandle handle;

protected:
  // authentication information.
  std::auto_ptr<DiameterCCAccount> accountInfo;
  
  // Request and Answer data
  CCR_Data ccrData;
  CCA_Data ccaData;

};

#endif

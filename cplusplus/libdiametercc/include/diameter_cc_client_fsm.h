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

/* 
   File: diameter_cc_client_fsm.h
   Author: Amrit Kaur (kaur_amrit@hotmail.com)
*/

#ifndef __CC_CLIENT_FSM_H__
#define __CC_CLIENT_FSM_H__

#include "framework.h"
#include "diameter_cc_parser.h"
#include "diameter_cc_account.h"

/*!
 * Windows specific export declarations
 */
#ifdef WIN32
   #if defined(DIAMETER_CC_CLIENT_EXPORT)
       #define DIAMETER_CC_CLIENT_EXPORTS __declspec(dllexport)
   #else
       #define DIAMETER_CC_CLIENT_EXPORTS __declspec(dllimport)
   #endif
#else
   #define DIAMETER_CC_CLIENT_EXPORTS
   #define DIAMETER_CC_CLIENT_EXPORTS
#endif

typedef AAA_JobHandle<AAA_GroupedJob> DiameterJobHandle;

class DiameterCCClientSession;

class DIAMETER_CC_CLIENT_EXPORTS DiameterCCClientStateMachine 
  : public AAA_StateMachineWithTimer<DiameterCCClientStateMachine>,
    public AAA_EventQueueJob
{
  friend class DiameterJobMultiplexor;

 public:
  /// Constructor.
  DiameterCCClientStateMachine(DiameterCCClientSession& s,                               
                               DiameterJobHandle &h,
                               ACE_Reactor &reactor);
  
  ~DiameterCCClientStateMachine() 
  {
    handle.Job().Remove(this);
    AAA_StateMachineWithTimer<DiameterCCClientStateMachine>::Stop(); 
  }

  enum {
    EvInitialRequest,
    EvInitialAnswer,
    EvSuccessfulAnswer,

    EvTerminationRequest,
    EvTerminationAnswer,
    EvSuccessfulTerminationAnswer,

    EvGrantWOSession,
    EvTerminateService,
    EvTxExpired,
    EvTxContinue,
    EvServiceTerminated,
    EvChangeInRating,
    EvGrantedUnitsElapsed,
    EvSentUpdateRequest,

    EvRARReceived,
    EvUpdateAnswer,
    EvTccExpired,

    EvFailure,

    EvDirectDebitingRequest,
    EvDirectDebitingAnswer,

    EvRefundAccountRequest,
    EvRefundAccountAnswer,

    EvCheckBalanceRequest,
    EvCheckBalanceAnswer,

    EvPriceEnquiryRequest,
    EvPriceEnquiryAnswer,

    EvStoredEventSent,
    EvGrantService,
    EvStoreRequestWithTFlag,
    EvStoreRequest,
    EvFailedAnswer,
    EvFailureToSend,
    EvSgDisconnect,
    EvSgSessionTimeout,
    EvSgAuthLifetimeTimeout,
    EvSgTimeout
  };

  /// Store an event and notify the session.
  inline void Notify(AAA_Event ev) {
    // Enqueue the event.
    if (AAA_EventQueueJob::Enqueue(ev) <= 0)
      Abort();

    if (handle.Job().Schedule(this) < 0)
      Abort();
  }
  
  virtual void Timeout(AAA_Event ev) {
    Notify(ev);
  }
  
  inline DiameterCCClientSession& Session() { return session; }

  /// This is used for aborting the state machine.  Usually called
  /// when Notify() fails.
  virtual void Abort()=0;

  /// This is used for constructing and sending a AA-Request.
  void SendCCR();
  
  virtual bool InitialRequest();

  virtual bool InitialAnswer();

  virtual bool TerminationRequest();

  virtual bool TerminationAnswer(){return true;}

  virtual bool DirectDebitingRequest();

  virtual bool DirectDebitingAnswer(){return true;}

  virtual bool RefundAccountRequest();

  virtual bool RefundAccountAnswer(){return true;}

  virtual bool CheckBalanceRequest();

  virtual bool CheckBalanceAnswer(){return true;}

  virtual bool PriceEnquiryRequest();

  virtual bool PriceEnquiryAnswer(){return true;}

  virtual bool TxExpired();
  
  virtual bool CreditControlFailureHandling(){return true;}

  /// This virtual function is called when a continuation of the
  /// authentication is signaled to the application.
  virtual void SignalContinue(DiameterCCAccount & account)=0;

  /// This virtual function is called when an authentication success
  /// is signaled to the application.
  virtual void SignalSuccess()=0;

  /// This virtual function is called when an authentication failure
  /// is signaled to the application.
  virtual void SignalFailure()=0;

  /// This is called by the application when authentication info is
  /// passed to the NAS.
  //void ForwardAuthenticationInfo
  //(DiameterCCAccount &account);

  /// This virtual function is called when the current authorization
  /// lifetime is expired.  XXX: this should also be called when an Re-Auth
  /// Request is received from the server.
  virtual void SignalReauthentication()=0;

  /// This virtual function is called when the session lifetime or the
  /// auth grace period is expired, or a disconect event is received
  /// from libdiameter.
  virtual void SignalDisconnect()=0;

  inline AAA_JobData& JobData() { return *handle.Job().Data(); }

  template <class T> inline T& JobData(Type2Type<T>) 
  { return (T&)*handle.Job().Data(); }

  // Insert-AVP member functions.

  /// This function is used for setting Destination-Realm AVP
  /// contents.  
  virtual void SetDestinationRealm
  (DiameterScholarAttribute<diameter_utf8string_t> &destinationRealm)
  {
  }

  /// This function is used for setting Destination-Host AVP
  /// contents.  
  virtual void SetDestinationHost
  (DiameterScholarAttribute<diameter_utf8string_t> &destinationHost)
  {
  }

  inline DiameterCCAccount& Account()
  { return *account; }

  inline CCR_Data& CCR_DATA() { return ccrData; }
  inline CCA_Data& CCA_DATA() { return ccaData; }

 protected:

 private:
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

  DiameterCCClientSession &session;

  // Job handle.
  DiameterJobHandle handle;

  // authentication information.
  boost::shared_ptr<DiameterCCAccount> account;
  
protected:
  // CC-Request and CC-Answer data.
  CCR_Data ccrData;
  CCA_Data ccaData;
};

#endif

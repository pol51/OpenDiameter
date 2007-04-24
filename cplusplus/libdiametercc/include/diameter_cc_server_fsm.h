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
   File: diameter_cc_server_fsm.h
   Author: Amrit Kaur (kaur_amrit@hotmail.com)
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
  : public AAA_StateMachineWithTimer<DiameterCCServerStateMachine>,
    public AAA_EventQueueJob
{
 public:
  /// Constructor.
  DiameterCCServerStateMachine(DiameterCCServerSession& s,
                               DiameterCCJobHandle &h,
                               ACE_Reactor &reactor);

  ~DiameterCCServerStateMachine() 
  {
    handle.Job().Remove(this); 
    AAA_StateMachineWithTimer<DiameterCCServerStateMachine>::Stop(); 
  }

  enum {
    EvInitialRequest,
    EvValidInitialRequest,
    EvInvalidInitialRequest,
    EvInitialRequestSuccessful,
    EvInitialRequestUnsuccessful,

    EvUpdateRequest,
    EvValidUpdateRequest,
    EvInvalidUpdateRequest,
    EvUpdateRequestSuccessful,
    EvUpdateRequestUnsuccessful,

    EvTerminationRequest,
    EvValidTerminationRequest,
    EvInvalidTerminationRequest,
    EvTerminationRequestSuccessful,
    EvTerminationRequestUnsuccessful,

    EvDirectDebitingRequest,
    EvValidDirectDebitingRequest,
    EvInvalidDirectDebitingRequest,
    EvDirectDebitingRequestSuccessful,
    EvDirectDebitingRequestUnsuccessful,

    EvRefundAccountRequest,
    EvValidRefundAccountRequest,
    EvInvalidRefundAccountRequest,
    EvRefundAccountRequestSuccessful,
    EvRefundAccountRequestUnsuccessful,

    EvCheckBalanceRequest,
    EvValidCheckBalanceRequest,
    EvInvalidCheckBalanceRequest,
    EvCheckBalanceRequestSuccessful,
    EvCheckBalanceRequestUnsuccessful,

    EvPriceEnquiryRequest,
    EvValidPriceEnquiryRequest,
    EvInvalidPriceEnquiryRequest,
    EvPriceEnquiryRequestSuccessful,
    EvPriceEnquiryRequestUnsuccessful,

    EvTccExpired,

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

  virtual void Timeout(AAA_Event ev) {
    Notify(ev);
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

  /// Initial Request action.  
  virtual bool InitialRequest() {return true;};

  virtual bool InitialAnswer() {return true;};

  /// Validate Update Request.  
  virtual bool ValidateUpdateRequest() {return true;};

  /// Update Request action.  
  virtual bool UpdateRequest() {return true;};

  /// Validate Termination Request.  
  virtual bool ValidateTerminationRequest() {return true;};

  /// Termination Request action.  
  virtual bool TerminationRequest() {return true;};

  /// Validate Direct Debiting Request.  
  virtual bool ValidateDirectDebitingRequest() {return true;};

  /// Direct Debiting Request action.  
  virtual bool DirectDebitingRequest() {return true;};

  /// Validate Refund Account Request.  
  virtual bool ValidateRefundAccountRequest() {return true;};

  /// Refund Account Request action.  
  virtual bool RefundAccountRequest() {return true;};

  /// Validate Check Balance Request.    
  virtual bool ValidateCheckBalanceRequest() {return true;};

  /// Check Balance Request action.  
  virtual bool CheckBalanceRequest() {return true;};

  /// Validate Price Enquiry Request.  
  virtual bool ValidatePriceEnquiryRequest() {return true;};

  /// Price Enquiry Request action.  
  virtual bool PriceEnquiryRequest() {return true;};

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

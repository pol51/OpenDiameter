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
// $Id: eap_peerfsm.hxx,v 1.30 2004/06/17 21:13:36 yohba Exp $

// eap_peerfsm.hxx:  Peer state machine
// Written by Yoshihiro Ohba

#ifndef __EAP_PEERFSM_HXX__
#define __EAP_PEERFSM_HXX__

#include <memory>
#include <ace/Basic_Types.h>
#include <ace/Task.h>
#include <ace/Activation_Queue.h>
#include <ace/Method_Request.h>
#include <ace/Future.h>
#include "eap.hxx"
#include "eap_fsm.hxx"

typedef ACE_Future<std::string> EapFutureStringResult;

class EapPeerSwitchStateMachine;
class EapSession;

/// An ACE method request used for non-blocking function call to input
/// peer identity.
class EAP_EXPORTS InputIdentityMethodRequest : public ACE_Method_Request
{
public:
  InputIdentityMethodRequest(EapPeerSwitchStateMachine *sm,
			     EapFutureStringResult &futureResult)
    : stateMachine(sm), futureResult(futureResult) {}
  int call();

private:
  EapPeerSwitchStateMachine *stateMachine;
  EapFutureStringResult futureResult;
};

/// This class is used for asynchronous execution of a method to input
/// an identity.
class EAP_EXPORTS EapInputIdentityTask : public ACE_Task<ACE_MT_SYNCH>
{
public:
  EapInputIdentityTask() {}

  void Set(EapPeerSwitchStateMachine *sm) { stateMachine = sm; }

  ~EapInputIdentityTask() { ACE_Thread::kill(thread, 2); }
  virtual int open()
  {
    return activate(THR_NEW_LWP);
  }
  virtual int close(unsigned long flags = 0)
  {
    return 0;
  }
  virtual int svc()
  {
    thread = ACE_Thread::self();
    do {
      std::auto_ptr<ACE_Method_Request> methodRequest(activationQueue.dequeue());
      if (methodRequest->call() == -1)
	break;
    } while(0);
    return 0;
  }
  
  EapFutureStringResult InputIdentity()
  {
    EapFutureStringResult futureResult;
    activationQueue.enqueue
      (new InputIdentityMethodRequest(stateMachine, futureResult));
    return futureResult;
  }
  
private:
  EapPeerSwitchStateMachine *stateMachine;
  ACE_Activation_Queue activationQueue;
  ACE_thread_t thread;
};


/// A leaf class for peer state machine.  Class creation can be made
/// only by EapSession class instances.
class EAP_EXPORTS EapPeerSwitchStateMachine : 
  public EapSwitchStateMachine,
  public EapStateMachine<EapPeerSwitchStateMachine>
{
  friend class EapPeerSwitchStateTable_S;
public:

  void Start() throw(AAA_Error);

  inline void Notify(AAA_Event ev)
  {
    try {
      EapStateMachine<EapPeerSwitchStateMachine>::Notify(ev);
    }
    catch (int i) {
	  ACE_UNUSED_ARG(i);
      EAP_LOG(LM_DEBUG, "Nofify() failed.\n");
      Abort();
    }
  }

  /// This function is called when an application receives an EAP
  /// message that needs to be processed by the state machine.
  void Receive(AAAMessageBlock*);

  /// A callback function called when the EAP session completes with
  /// success.
  virtual void Success()=0;

  /// A callback function called when the EAP session completes with
  /// failure.  
  virtual void Failure()=0;

  /// A callback function called when Notification message is received.
  virtual void Notification(std::string &str)=0;

  /// This callback function is called when peer identity needs to be
  /// input by the application.
  virtual std::string& InputIdentity()=0;

  /// This function is returns a reference to notificationAllowed.
  inline bool& NotificationAllowed() { return notificationAllowed; }

  /// This function is returns a reference to notificationStr.
  inline std::string& NotificationString() { return notificationStr; }

  /// Use this function to obtain the reference to client timeout value.
  inline ACE_UINT16& AuthPeriod() { return authPeriod; }

  /// Use this function to obtain the reference to lastIdentifier.
  inline ACE_Byte& LastIdentifier() { return lastIdentifier; }

  /// Use this function to obtain the reference to lastIdValidity.
  inline bool& LastIdValidity() { return lastIdValidity; }

  /// Use this function to obtain the reference to reqMethod.
  inline EapType& ReqMethod() { return reqMethod; }

  /// Use this function to obtain the reference to receivedFirstRequest.
  inline bool& ReceivedFirstRequest() { return receivedFirstRequest; }

  /// Use this function to obtain the reference to inputIdentityTask.
  inline EapInputIdentityTask& InputIdentityTask() { return inputIdentityTask; }

  /// Use this function to obtain the reference to futureIdentity.
  inline EapFutureStringResult& FutureIdentity() { return futureIdentity; }

  /// Use this function to obtain identityInputTimerType.
  inline int InputIdentityTimerType() { return inputIdentityTimerType; }

  /// Decision variable definition.
  enum EapPeerDecision {
    FAIL,
    COND_SUCC,
    UNCOND_SUCC
  };

  inline EapPeerDecision& Decision() { return decision; }

  /// External method states visibile to peer switch state machine.  Method
  /// state machines can use finer grained states inside themselves
  /// machines but MethodState() must be defined by each method
  /// state machine to provide mapping from an internal method state
  /// and to an external method state.
  enum EapPeerMethodState {
    NONE,
    INIT,
    CONT,
    MAY_CONT,
    DONE,
  };

  inline EapPeerMethodState& MethodState() { return methodState; }

  /// External events passed from applications and methods.
  enum event {
    EvRxMsg=-1,          // Message reception event passed from
			 // application via EapSession::Receive().
    EvSgPortEnabled=-2,  // Port enabled event passed from application.
    EvSgValidReq=-3,     // Integrity check sucess event passed from
			 // methods.
    EvSgInvalidReq=-4    // Integrity check failure event passed from
			 // methods.
  };


protected:
  EapPeerSwitchStateMachine(ACE_Reactor &r, EapJobHandle &h);

  ~EapPeerSwitchStateMachine() {}

  /// This variable indicates the Identifier of the last valid Request
  /// that was responsed.
  ACE_Byte lastIdentifier;

  /// This boolean variable indicates whether the lastIdentifier is
  /// valid (e.g., indating at least one valid Response was received
  /// in the session.  This variable will be re-initialized at the time of
  /// re-authentication.
  bool lastIdValidity;

  /// The default value for client timeout.
  static const ACE_UINT16 defaultAuthPeriod;

  /// Authorization period.
  ACE_UINT16 authPeriod;

  /// This boolean variable indicates whether the peer received the
  /// first Request from the authenticator.
  bool receivedFirstRequest;

  /// This is used to retain the status of Notification message handling.
  bool notificationAllowed;

  /// Notification payload is stored here.
  std::string notificationStr;

  /// reqMethod or the method contained in the received request.
  EapType reqMethod;

  /// A handle to the thread that is spawned for identity input.
  EapInputIdentityTask inputIdentityTask;

  /// The place where an identity string will be set as a result of
  /// asynchrounous function call.
  EapFutureStringResult futureIdentity;

  /// Timer type used for input identity timer;
  int inputIdentityTimerType;

  /// This variable stores peer decision.
  EapPeerDecision decision;

  /// This variable stores external method state.
  EapPeerMethodState methodState;
};

#endif // __EAP_PEERFSM_HXX__

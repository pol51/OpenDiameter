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
// $Id: eap_authfsm.hxx,v 1.28 2004/09/29 23:58:50 yohba Exp $

// eap_authfsm.hxx:  Authenticator state machine
// Written by Yoshihiro Ohba

#ifndef __EAP_AUTHFSM_HXX__
#define __EAP_AUTHFSM_HXX__

#include <ace/Basic_Types.h>
#include <string>
#include "eap.hxx"
#include "eap_fsm.hxx"
#include "eap_method_registrar.hxx"


/// A class for authetnicator state machine.  Class creation can be made
/// only by EapSession.
class EAP_EXPORTS EapAuthSwitchStateMachine : public EapSwitchStateMachine
{
public:

  /// This function is called when an application receives an EAP
  /// message that needs to be processed by the state machine.
  void Receive(AAAMessageBlock*);

  /// A callback function called when the EAP session completes with
  /// success.  Success message is contained in the argument of
  /// Success().  It is up to the implementation of the derived class
  /// whether Success message is actually sent or not.  If further
  /// processing for the message is performed in other threads, the
  /// application must make a duplication of the message via
  /// AAAMessageBlock::Acquire(AAAMessageBlock*) so that the API will
  /// not release the message allocation until reference count becomes
  /// 0.
  virtual void Success(AAAMessageBlock *b)=0;

  /// This Success callback is called when EAP authentication succeeds
  /// without generating an EAP-Success message.
  virtual void Success()=0;

  /// A callback function called when the EAP session completes with
  /// failure.  Failure message is contained in the argument of
  /// Failure().  It is up to the implementation of the derived class
  /// whether Failure message is actually sent or not.  If further
  /// processing for the message is performed in other threads, the
  /// application must make a duplication of the message via
  /// AAAMessageBlock::Acquire(AAAMessageBlock*) so that the API will
  /// not release the message allocation until reference count becomes
  /// 0.
  virtual void Failure(AAAMessageBlock *b)=0;

  /// This Success callback is called when EAP authentication fails
  /// without generating an EAP-Failure message.
  virtual void Failure()=0;

  /// Call this function to get the reference to the maximum
  /// retransmission count.
  inline ACE_UINT16& MaxRetransmissionCount() 
  { return maxRetransmissionCount; }

  /// Call this function to get the reference to the retransmission
  /// interval value.
  inline ACE_UINT16& RetransmissionInterval() 
  { return retransmissionInterval; }

  /// Call this function to get the currentIdentifier.
  //  inline ACE_Byte CurrentIdentifier() { return currentIdentifier; }

  /// Call this function to set the isEapBackend variable.
  virtual inline bool IsEapBackend(void) { return false; }

  /// Call this function to check whether retransmission is enabled.
  inline bool RetransmissionEnabled() { return (retransmissionInterval != 0); }

  /// Call this function to set whether the backend authenticator
  /// sends the initial Request.
  inline void NeedInitialRequestToSend(bool b) { needInitialRequestToSend=b; }

  /// Call this function to check whether the backend authenticator
  /// sends the initial Request.
  inline bool NeedInitialRequestToSend() { return needInitialRequestToSend; }

  /// Call this function to get the reference to the notificationString.
  inline std::string& NotificationString() { return notificationString; }

  /// Call this function to get the reference to the retransmissionCount.
  inline ACE_UINT16& RetransmissionCount() { return retransmissionCount; }

  enum EapAuthDecision {
      DecisionSuccess,
      DecisionFailure,
      DecisionContinue,
      DecisionPassthrough
  };

  /// Call this function to obtain the authenticator decision.
  virtual EapAuthDecision Decision();

  /// External method state visibile to authenticator switch state machine.  
  /// The state is updated by the authenticator state machine.

  enum EapAuthMethodState {
    PROPOSED=0,
    CONT,
    END,     
  };


  EapAuthMethodState& MethodState();

  /// External events passed from applications and methods.
  enum event {
    EvRxMsg=-1,                 // Message reception event passed from
				// application via
				// EapSession::Receive().
    EvSgPortEnabled=-2,         // Port enabled event passed from
				// application.
    EvSgValidResp=-3,           // Event for integrity check success
				// with continueing method (passed
				// from methods).
    EvSgInvalidResp=-4,         // Integrity check failure event
				// passed from methods.
    EvSgEndMethod=-5,           // Event for integrity check sucess
				// with end method (passed from
				// methods).
    EvSgRestart=-6,             // Event generated to start the state
				// machine as a non-backend server.
    EvSgAaaContinue=-7,         // Event generated when AAA continues.

    EvSgAaaSuccess=-8,          // Event generated when AAA succeeds.

    EvSgAaaFailure=-9,          // Event generated when AAA fails.
  };

protected:


  EapAuthSwitchStateMachine(ACE_Reactor &r, EapJobHandle &h) 
    : EapSwitchStateMachine(r, h),
      discardCount(0),
      retransmissionCount(0),
      maxRetransmissionCount(defaultMaxRetransmissionCount),
      retransmissionInterval(defaultRetransmissionInterval),
      needInitialRequestToSend(true)
  {}

  virtual ~EapAuthSwitchStateMachine() {}

  static const ACE_UINT16 defaultMaxRetransmissionCount;
  static const ACE_UINT16 defaultRetransmissionInterval;

  ACE_UINT16 discardCount;
  ACE_UINT16 retransmissionCount;
  ACE_UINT16 maxRetransmissionCount;
  ACE_UINT16 retransmissionInterval;  // Set this value to zero when
				      // retransmission is disabled.
  std::string notificationString;     // Used for sending notification
				      // to the per.
  bool needInitialRequestToSend;

  /// This variable stores method state.
  EapAuthMethodState methodState;
};

/// A leaf class for standalone authenticator state machine.  Class
/// creation can be made only by EapSession.
class EAP_EXPORTS EapStandAloneAuthSwitchStateMachine : 
    public EapAuthSwitchStateMachine,
    public EapStateMachine<EapStandAloneAuthSwitchStateMachine>
{
 public:

  void Start() throw(AAA_Error)
  {
    // Set the current policy element to point to the initial policy
    // element.
    policy.CurrentPolicyElement(policy.InitialPolicyElement());

    // Delete the last executed method if any.
    DeleteMethodStateMachine();

    // Initialize the state machine.
    EapStateMachine<EapStandAloneAuthSwitchStateMachine>::Start();

    // Initialize key stuff.
    keyData.resize(0);
    keyAvailable=false;

    // Generate the initial event.
    Notify(EapAuthSwitchStateMachine::EvSgRestart);
  }

  inline void Notify(AAA_Event ev)
  {
    try {
      EapStateMachine<EapStandAloneAuthSwitchStateMachine>::Notify(ev);
    }
    catch (int i) {
	  ACE_UNUSED_ARG(i);
      EAP_LOG(LM_DEBUG, "Nofify() failed.\n");
      Abort();
    }
  }

protected:
  EapStandAloneAuthSwitchStateMachine(ACE_Reactor &r, EapJobHandle &h);

  virtual ~EapStandAloneAuthSwitchStateMachine();
};

/// A leaf class for backend authenticator state machine.  Class
/// creation can be made only by EapSession.
class EAP_EXPORTS EapBackendAuthSwitchStateMachine :
    public EapAuthSwitchStateMachine,
    public EapStateMachine<EapBackendAuthSwitchStateMachine>
{
public:

  void Start() throw(AAA_Error)
  {
    // Set the current policy element to point to the initial policy
    // element.
    policy.CurrentPolicyElement(policy.InitialPolicyElement());

    // Initialize the state machine.
    EapStateMachine<EapBackendAuthSwitchStateMachine>::Start();

    // Initialize key stuff.
    keyData.resize(0);
    keyAvailable=false;

    // Generate the initial event.
    Notify(EapAuthSwitchStateMachine::EvSgRestart);
  }

  inline void Notify(AAA_Event ev)
  {
    try {
      EapStateMachine<EapBackendAuthSwitchStateMachine>::Notify(ev);
    }
    catch (int i) {
	  ACE_UNUSED_ARG(i);
      EAP_LOG(LM_DEBUG, "Nofify() failed.\n");
      Abort();
    }
  }

  /// Call this function to start the session.
  void Start(AAAMessageBlock *msg);

  inline bool IsEapBackend(void) { return true; }

protected:
  EapBackendAuthSwitchStateMachine(ACE_Reactor &r, EapJobHandle &h);

  virtual ~EapBackendAuthSwitchStateMachine();
};

/// A leaf class for passthrough authenticator state machine.  Class
/// creation can be made only by EapSession.
class EAP_EXPORTS EapPassThroughAuthSwitchStateMachine : 
    public EapAuthSwitchStateMachine,
    public EapStateMachine<EapPassThroughAuthSwitchStateMachine>
{
public:

  void Start() throw(AAA_Error)
  {
    // Set the current policy element to point to the initial policy
    // element.
    policy.CurrentPolicyElement(policy.InitialPolicyElement());

    // Initialize the state machine.
    EapStateMachine<EapPassThroughAuthSwitchStateMachine>::Start();

    // Initialize key stuff.
    keyData.resize(0);
    keyAvailable=false;

    // Generate the initial event.
    Notify(EapAuthSwitchStateMachine::EvSgRestart);
  }

  inline void Notify(AAA_Event ev)
  {
    try {
      EapStateMachine<EapPassThroughAuthSwitchStateMachine>::Notify(ev);
    }
    catch (int i) {
      ACE_UNUSED_ARG(i);
      EAP_LOG(LM_DEBUG, "Nofify() failed.\n");
      Abort();
    }
  }

  /// A callback function callled when a Response is forwarded to an
  /// EAP server.  // virtual void ForwardResponse(AAAMessageBlock
  /// *msg)=0;
  virtual void ForwardResponse(AAAMessageBlock *msg)=0;

  /// Call this function for getting a reference to aaaRxQueue.
  inline EapMessageQueue& AAARxQueue() throw() { return aaaRxQueue; }

  /// The specified identifier is used as the next identifier.
  ACE_Byte GetNextIdentifier(ACE_Byte id) { return id; }

  /// Reimplemented from EapAuthSwitchStateMachine::Decision().
  EapAuthDecision Decision();

  /// Called by application when AAA succeeds.  The argument may be null.
  void AAA_Success(AAAMessageBlock *msg) throw(int)
  {
    // Check if this is operating in passthorugh mode.
    if (Decision() != DecisionPassthrough)
      {
	EAP_LOG(LM_ERROR, "Not operating in pass-through mode");
	throw -1;
      }

    if (msg)
      AAARxQueue().enqueue_tail(AAAMessageBlock::Acquire(msg));

    Notify(EvSgAaaSuccess);
  }

  /// Called by application when AAA succeeds with passing a AAA-Key.
  /// The first argument may be null.
  void AAA_Success(AAAMessageBlock *msg, std::string& aaaKey) throw(int)
  {
    // Check if this is operating in passthorugh mode.
    if (Decision() != DecisionPassthrough)
      {
	EAP_LOG(LM_ERROR, "Not operating in pass-through mode");
	throw -1;
      }

    KeyData() = aaaKey;

    if (msg)
      AAARxQueue().enqueue_tail(AAAMessageBlock::Acquire(msg));

    Notify(EvSgAaaSuccess);
  }

  /// Called by application when AAA fails.  The argument may be null.
  void AAA_Failure(AAAMessageBlock *msg) throw(int)
  {
    // Check if this is operating in passthorugh mode.
    if (Decision() != DecisionPassthrough)
      {
	EAP_LOG(LM_ERROR, "Not operating in pass-through mode.\n");
	throw -1;
      }

    if (msg)
      AAARxQueue().enqueue_tail(AAAMessageBlock::Acquire(msg));

    Notify(EvSgAaaFailure);
  }

  /// Called by application when AAA continues.  The argument must not
  /// be null.
  void AAA_Continue(AAAMessageBlock *msg) throw(int)
  {
    // Check if this is operating in passthorugh mode.
    if (Decision() != DecisionPassthrough)
      {
	EAP_LOG(LM_ERROR, "Not operating in pass-through mode.\n");
	throw -1;
      }

    if (!msg)
      {
	EAP_LOG(LM_ERROR, "AAAContinue must contain non-null msg.\n");
	throw -1;
      }
    AAARxQueue().enqueue_tail(AAAMessageBlock::Acquire(msg));
    Notify(EvSgAaaContinue);
  }

protected:
  EapPassThroughAuthSwitchStateMachine(ACE_Reactor &r, EapJobHandle &h);
  virtual ~EapPassThroughAuthSwitchStateMachine();

  /// this queue is used for receiving EAP message from EAP server.
  EapMessageQueue aaaRxQueue;
};

#endif // __EAP_AUTHFSM_HXX__

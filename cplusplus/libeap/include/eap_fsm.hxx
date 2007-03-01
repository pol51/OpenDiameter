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
// $Id: eap_fsm.hxx,v 1.42 2006/03/16 17:01:51 vfajardo Exp $

// eap_fsm.hxx:  header file for EAP finite state machine
// Written by Yoshihiro Ohba

#ifndef __EAP_FSM_HXX__
#define __EAP_FSM_HXX__

#include <ace/Basic_Types.h>
#include <ace/Singleton.h>
#include <ace/Synch.h>
#include <ace/Event_Handler.h>
#include <list>
#include <utility>
#include "framework.h"
#include "eap.hxx"
#include "eap_log.hxx"
#include "eap_policy.hxx"

typedef AAA_JobHandle< AAA_GroupedJob > EapJobHandle;

/// This class defines a job that are used by both switch state
/// machine and method state machine to schedule an event.  The
/// scheduling is based on storing a pointer to state machines which
/// themself are jobs to be multiplexed by this class object.
class EapJobMultiplexor : public AAA_JobQueueJob
{
 public:
  EapJobMultiplexor(EapJobHandle& h) : handle(h)
  {}
  ~EapJobMultiplexor() { handle.Job().Remove(this); }

  void Flush()
  {
    AAA_JobQueueJob::Flush();
    handle.Job().Remove(this);
  }

  int Serve() 
  {
    if (!ExistBacklog())
      {
	EAP_LOG(LM_ERROR, "%N: no backlog to serve.");
	return 0;
      }

    AAA_Job *job = 0;
    Dequeue(job);
    bool existBacklog = ExistBacklog();
    job->Serve();
    return existBacklog;
  }

  // Reimplementedf from AAA_JobQueueJob
  int Schedule(AAA_Job* job, size_t=1)
  {
    Enqueue(job);
    return handle.Job().Schedule(this);
  }

  inline AAA_JobData& JobData() { return *handle.Job().Data(); }

  template <class T> inline T& JobData(Type2Type<T>) 
  { return (T&)*handle.Job().Data(); }

 private:

  /// Job handle.
  EapJobHandle handle;
};

/// The functional model of EAP devides the protocol task into a part
/// that is commonly performed independent of any EAP authentication
/// method and a part that is specific to eap EAP authentication
/// method.  The former and the latter parts are refered to as
/// "switch" and "method", respectively.  State machines defined to
/// run switches and methods are refered to as <b>switch state
/// machines</b> (see \ref switchStateMachine) and <b>method state
/// machines</b> (see \ref methodStateMachine), respectively.  Both
/// switch and method state machines have the same EapStateMachine
/// structure in common.
template <class ARG>
class EapStateMachine : 
  public AAA_StateMachineWithTimer<ARG>, 
  private AAA_EventQueueJob
{
 public:
  /// Constructor.
  EapStateMachine(ARG& arg, AAA_StateTable<ARG> &table, ACE_Reactor &r, 
		  EapJobMultiplexor &mux, char *name=0)
    : AAA_StateMachineWithTimer<ARG>(arg, table, r, name),
      mux(mux)
  {}
  
  void Stop()
  {
    AAA_StateMachineWithTimer<ARG>::Stop();
    mux.Flush();
  }

  /// Inherited from AAA_EventQueueJob.
  inline int Serve()
  {
    // Obtain the event to execute.
    AAA_Event ev = 0;
    AAA_EventQueueJob::Dequeue(ev);

    bool existBacklog = AAA_EventQueueJob::ExistBacklog();

    // Execute it.
    AAA_StateMachineWithTimer<ARG>::Event(ev);
    return existBacklog ? 1 : 0;
  }

  /// Reimplemented from parent class.
  inline void Timeout(AAA_Event ev) { Notify(ev); }

  // Store an event and notify the session.  An integer -1 is thrown
  // when job scheduling fails.
  inline void Notify(AAA_Event ev) throw (int) {
    // Enqueue the event.
    if (AAA_EventQueueJob::Enqueue(ev) <= 0)
      throw -1;

    // Schedule me to the mux.
    if (mux.Schedule(this) < 0)
      throw -1;
  }

 private:

  int Schedule(AAA_Job*, size_t=1) { return (-1); }  // Not used.

  EapJobMultiplexor& mux;
};

typedef ACE_Message_Queue<ACE_MT_SYNCH> EapMessageQueue;

/// Forward declarations
class EapMethodStateMachine;

/*! \page switchStateMachine Switch State Machine

There are two types of switch state machines, <b>peer switch state
machine</b> and <b>authenticator switch state machine</b>.  Both peer
and authenticator state machines have the following attributes in common.

- A <b>method state machine</b> (see \ref methodStateMachine).

- An authentication policy that is used for creating a method state
machine of a particular EAP (authentication) method and describes
rules on how and by which order each (authentication) method is
performed to complete an EAP conversation.  Authentication policy is
explained in \ref authpolicy.

- A set of callback functions that are defined by applications and
called when the switch state machine needs to pass an asynchrounous
event and optional information associated with the event to the
applications.  The callback functionality is described in \ref
callback.

In the libeap library, a method state machine is defined as an
attribute of a switch state machine.

 */
/// The base class for switch state machines.
class EAP_EXPORTS EapSwitchStateMachine : public EapJobMultiplexor
{
public:

  /// Called by any switch or method state machine.
  virtual void Notify(AAA_Event ev)=0;

  /// This callback function is called when a message other than
  /// Success/Failure is sent from the session.  If further processing
  /// for the message is performed in other threads, the application
  /// must make a duplication of the message via
  /// AAAMessageBlock::Acquire(AAAMessageBlock*) so that the API will
  /// not release the message allocation until reference count becomes 0.
  virtual void Send(AAAMessageBlock *b)=0;

  /// This callback function is called when there is an internal error
  /// in a state machine.  The default action is to exit the program.
  /// Applications can replace the default behavior with more
  /// appropriate behavior such as deleting the errornous session.
  virtual void Abort()=0;

  /// Call this function to get the current method.
  inline EapType& CurrentMethod() { return currentMethod; }

  /// This function is called by EAP carrier protocol to input EAP message.
  virtual void Receive(AAAMessageBlock*)=0;

  /// This function is called by AAA protocol on passthrough
  /// authenticator to input EAP message.
  virtual void ReceiveFromAAA(AAAMessageBlock *msg) throw(int)
  {
    EAP_LOG(LM_ERROR, "ReceiveFromAAA Operation not allowed.");
    throw -1;
  }

  /// Call this function to get the currentIdentifier.
  inline ACE_Byte& CurrentIdentifier() { return currentIdentifier; }

  /// Call this function to get the pointer of rxMessage.  This
  /// function is typically called by switches.

  inline AAAMessageBlock* GetRxMessage() {return rxMessage; }

  /// Call this function to set the pointer to rxMessage.  This
  /// function is typically called by methods.
  inline void SetRxMessage(AAAMessageBlock *p) { 
    if (rxMessage) rxMessage->release();
    rxMessage=p; 
  }

  /// Call this function to delete rxMessage.  This function is
  /// typically called by methods.
  inline void DeleteRxMessage() { SetRxMessage(0); }

  /// Call this function to get the pointer to txMessage.  This
  /// function is typically called by switches.
  inline AAAMessageBlock* GetTxMessage() { return txMessage; }

  /// Call this function to set the pointer to txMessage.  This
  /// function is typically called by methods.
  inline void SetTxMessage(AAAMessageBlock *p) { 
    if (txMessage) txMessage->release(); 
    txMessage=p; 
  }

  /// Call this function to delete txMessage.  This function is
  /// typically called by methods.
  inline void DeleteTxMessage() { SetTxMessage(0); }

  /// Call this function to get the pointer to the identity of the EAP peer.
  inline std::string& PeerIdentity() { return peerIdentity; } 

  /// Call this function to get the pointer to the identity of the EAP
  /// authenticator.
  inline std::string& AuthenticatorIdentity() 
  { return authenticatorIdentity; } 

  /// Call this function to check the availability of the key.
  inline bool& KeyAvailable() { return keyAvailable; }
  
  /// Call this function to retrieve the key.
  inline std::string& KeyData() { return keyData; } 
  
  /// Call this function to check whether this is a tunneled session.
  bool IsEapTunneled() { return eapTunneled; }

  /// Call this function to get the pointer to policy.
  inline EapPolicy& Policy(void) { return policy; } 

  /// Call this function to create a method state mechine in the
  /// session.
  void CreateMethodStateMachine(EapType t, EapRole role);

  /// Call this function to delete the method state mechine in the
  /// session.
  void DeleteMethodStateMachine();

  /// Call this function to get the pointer to method state machine.
  inline EapMethodStateMachine& MethodStateMachine() 
  { return *methodStateMachine; }

  /// Call this function to get the reference to receiving message queue.
  EapMessageQueue& RxQueue() { return rxQueue; }

  /// Use this function to obtain the reference to discardCount.
  int& DiscardCount() { return discardCount; }

  ACE_Reactor& Reactor() { return reactor; }

protected:
  EapSwitchStateMachine(ACE_Reactor &r, EapJobHandle &h) : 
    EapJobMultiplexor(h),
    currentMethod(EapType(0)),
    txMessage(0), rxMessage(0), eapTunneled(false), 
    methodStateMachine(0), discardCount(0), keyAvailable(false),
    reactor(r)
  {
    keyData.resize(0);
  }

  virtual ~EapSwitchStateMachine() 
  {
    DeleteMethodStateMachine();
    DeleteRxMessage();
    DeleteTxMessage();
  }

  /// This indicates the currently processed EAP Type.
  EapType currentMethod;

  /// This is used for passing an outgoing EAP message from methods to
  /// the switch.  For peers, txMessage is equivalent to eapRespData
  /// and lastRespData.  For authenticators, txMessage is equivalent
  /// to eapReqData and lastReqData.
  AAAMessageBlock *txMessage;

  /// This is used for passing an incoming EAP message from the switch
  /// to methods.
  AAAMessageBlock *rxMessage;

  /// This is used for queueing incoming EAP messages to process.
  EapMessageQueue rxQueue;

  /// This variable indicates the Identifier of the last valid Request
  /// (for Peer) or the outstanding Request (for Authenticator).
  ACE_Byte currentIdentifier;

  /// This stores the Identity payload (e.g., username) of the EAP
  /// peer.  The string may or may not be null-terminated.  When the
  /// string is null-terminated, the terminating null charactor must
  /// not be included in the Identity payload.
  std::string peerIdentity;

  /// This stores the Identity payload of the EAP authenticator.  The
  /// string may or may not be null-terminated.  When the string is
  /// null-terminated, the terminating null charactor must not be
  /// included in the Identity payload.
  std::string authenticatorIdentity;

  /// This variable is set true if this session is created within
  /// another session.
  bool eapTunneled;

  /// This is the policy used for the session.
  EapPolicy policy;

  /// This stores the pointer to a method state machine.
  EapMethodStateMachine* methodStateMachine;

  /// This variable is incremented when a received message is discarded.
  int discardCount;

  /// This variable indicates whether the key is available or not.
  bool keyAvailable;

  /// This variable stores the key.
  std::string keyData;

  ACE_Reactor &reactor;
};

class EapPeerSwitchStateMachine;
class EapAuthSwitchStateMachine;

/*! \page methodStateMachine Method State Machine

A method state machine is an implementation of a particular EAP
method.  A method state machine is created by a switch state machine
when the authentication policy indicates that there is an EAP
authentication method that needs to be performed before an EAP
authentication result is obtained.  Method state machines are
implemented in class EapMethodStateMachine from which a distinct class 
is derived for handling each EAP method.

When an EAP method is capable of deriving a key, the key is set to
keyData variable.  A zero-sized keyData means that the key is not
available, otherwise the key is available.  The keyData size must be
set to zero when (re)starting the EAP method.  The key data must not
be available until the method completes successfully.

The following methods are defined in the library by default.

- Identity (implemented in EapPeerIdentityStateMachine and
EapAuthIdentityStateMachine classes)
- Notification (implemented in EapPeerNotificationStateMachine and
EapAuthNotificationStateMachine classes)
- MD5-Challenge (implemented in EapPeerMD5ChallengeStateMachine and
EapAuthMD5ChallengeStateMachine classes)

Note that Nak is implemented in switch state machines.

*/
/// The base class for method state machines.
class EAP_EXPORTS EapMethodStateMachine
{
public:

  virtual ~EapMethodStateMachine() {}

  /// This pure virtual function is called when starting any method
  /// state machine.
  virtual void Start() throw(AAA_Error)=0;

  /// This pure virtual function is called when scheduling a method
  /// event.
  virtual void Notify(AAA_Event)=0;

  /// Call this function to check the availability of the key.
  inline bool KeyAvailable() { return (keyData.size()>0); }
  
  /// Call this function to retrieve the key.
  inline std::string& KeyData() { return keyData; } 
  
  /// Call this function to obtain the job data.
  inline AAA_JobData& JobData() { return switchStateMachine.JobData(); }

  template <class T> inline T& JobData(Type2Type<T> t) 
  { return switchStateMachine.JobData(t); }

  /// Call this function to check whether the method is done.
  bool& IsDone() { return isDone; }

  /// Call this function to get the pointer to method state machine.
  inline EapSwitchStateMachine& SwitchStateMachine() 
  { return switchStateMachine; }

  /// Override this function to implement differnet next identifier
  /// allocation policies.
  virtual ACE_Byte GetNextIdentifier(ACE_Byte id) { return id + 1; }

  /// External event passed from switch.
  enum event {
    EvSgIntegrityCheck=-1, // Integrity check
  };

  /// Call this function to get the reference to peer switch state
  /// machine.
  inline EapPeerSwitchStateMachine& PeerSwitchStateMachine() 
  { 
    return (EapPeerSwitchStateMachine&)switchStateMachine;
  }

  /// Call this function to get the reference to authenticator switch
  /// state machine.
  inline EapAuthSwitchStateMachine& AuthSwitchStateMachine() 
  { 
    return (EapAuthSwitchStateMachine&)switchStateMachine;
  }

  template <class T> 
  inline T& SwitchStateMachine(Type2Type<T>) 
  { return (T&)switchStateMachine; }

protected:
  EapMethodStateMachine(EapSwitchStateMachine &s)
    : switchStateMachine(s), isDone(false)
  {
    keyData.resize(0);
  }


  /// This contains the pointer to the switch state machine.
  EapSwitchStateMachine& switchStateMachine;
  
  /// This variable indicates whether the method is done.  It is the
  /// responsibility of each EAP method implementation to set this
  /// variable to an appropriate value when the method is done.
  bool isDone;

  /// This variable stores the key.  It is the
  /// responsibility of each EAP method implementation to initialize
  /// and set this variable.
  std::string keyData;

 private:
};

#endif // __EAP_FSM_HXX__

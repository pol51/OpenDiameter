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

// $Id: md5_test.cxx,v 1.8 2006/03/16 17:01:52 vfajardo Exp $ 
// A test program for EAP API.
// Written by Yoshihiro Ohba

#include <iostream>
#include <ace/OS.h>
#include <ace/Signal.h>
#include <ace/Event_Handler.h>
#include <boost/shared_ptr.hpp>
#include "eap.hxx"
#include "eap_peerfsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_identity.hxx"
#include "eap_policy.hxx"
#include "eap_method_registrar.hxx"
#include "eap_fsm.hxx"
#include "eap_log.hxx"
#include "eap_md5.hxx"

class MyPeerSwitchStateMachine;
class MyStandAloneAuthSwitchStateMachine;
class MyBackendAuthSwitchStateMachine;
class MyPassThroughAuthSwitchStateMachine;

class PeerData;
class StandAloneAuthApplication;
class BackendAuthApplication;
class PassThroughAuthApplication;

/// Task class used in this sample program.
class EapTask : public AAA_Task
{
 public:
  /// Constructor.
  EapTask() : AAA_Task(AAA_SCHED_FIFO, "EAP") {}

  /// Destructor.
  ~EapTask() {}
};

/// This class defines an EAP transport.  When an EAP message is sent
/// to a particular entity (e.g., an entity may be peer, standalone
/// authenticator, backend authenticator or passthrough authenticator
/// depending on the role of the sender, Transmit() method of the
/// Channel object of the receiving entity is called.  Transmit()
/// method can have sub-channels which is used for distinguishing different
/// types of messages.
class Channel 
{
 public:
  Channel() {}
  virtual ~Channel() {}
  virtual void Transmit(AAAMessageBlock *msg)=0;
  virtual void Transmit(AAAMessageBlock *msg, int subChannel)=0;
};

/*
  ---------------------------------------------------------
  Brief explanation on what is done in this sample program.
  ---------------------------------------------------------

  This program includes three test cases from which the program will
  prompt you to choose one.

  o Case 1 is for EAP conversation between a peer and an authenticator
  *without* a passthrough authenticator in between.

  o Case 2 is for EAP conversation between a peer and an authenticator
  *with* a passthrough authenticator in between.

  o Case 3 is similar to Case 2, except that the passthrough
  authenticator originates the Request/Identity on behalf of the
  backend authenticator.
  
  All cases completes with success after Idenity exchange and
  MD5-Challenge authentication method is performed.  In both cases,
  the peer session entity will prompt you to input a username.  Once
  the username is input, it is carried in an Response/Identity message
  and sent to the (backend) authenticator.  In Case 2 and Case 3,
  passthrough authenticator will forward the response to the backend
  authenticator.

  The authenticator or passthrough authenticator will retransmit the
  Request/Identity message until you input the username or the number
  of retransmission reaches its maximum value (the maximum value is
  set to 3 in this program).  For the latter case, the authenticator
  will stop its state machine and the peer will timeout and fail.

  Case 1:

  Peer                Authenticator
   |  Request/Identity      |
   |<-----------------------|
   |                        |
(input userid)              |
   |                        |
   |  Response/Identity     |
   |----------------------->|
   |  Request/MD5-Challenge |
   |<-----------------------|
   |  Response/MD5-Challenge|
   |----------------------->|
   |                        |
   |       Success          |
   |<-----------------------|


Case 2:

  Peer                PassThough                   Backend
                      Authenticator                Authenticator
   |                        |  Null message            |
   |                        |------------------------->|
   |                        |                          |
   |                        |  Request/Identity        |
   |  Request/Identity      |<-------------------------|
   |<-----------------------|                          |
   |                        |                          |
(input userid)              |                          |
   |                        |                          |
   |  Response/Identity     |                          |
   |----------------------->|  Response/Identity       |
   |                        |------------------------->|
   |                        |  Request/MD5-Challenge   |
   |  Request/MD5-Challenge |<-------------------------|
   |<-----------------------|                          |
   |  Response/MD5-Challenge|                          |
   |----------------------->|  Response/MD5-Challenge  |
   |                        |------------------------->|
   |                        |                          |
   |                        |          Success         |
   |       Success          |<-------------------------|
   |<-----------------------|

Case 3:

  Peer                PassThough                   Backend
                      Authenticator                Authenticator
   |  Request/Identity      |                          |
   |<-----------------------|                          |
   |                        |                          |
(input userid)              |                          |
   |                        |                          |
   |  Response/Identity     |                          |
   |----------------------->|  Response/Identity       |
   |                        |------------------------->|
   |                        |  Request/MD5-Challenge   |
   |  Request/MD5-Challenge |<-------------------------|
   |<-----------------------|                          |
   |  Response/MD5-Challenge|                          |
   |----------------------->|  Response/MD5-Challenge  |
   |                        |------------------------->|
   |                        |                          |
   |                        |          Success         |
   |       Success          |<-------------------------|
   |<-----------------------|

 */

// Class definition for authenticator identity method for my application.
class MyEapAuthIdentityStateMachine : public EapAuthIdentityStateMachine
{
  friend class EapMethodStateMachineCreator<MyEapAuthIdentityStateMachine>;
public:
  MyEapAuthIdentityStateMachine(EapSwitchStateMachine &s)
    : EapAuthIdentityStateMachine(s) {} 

  // Reimplemented from EapAuthIdentityStateMachine.
  ProcessIdentityResult ProcessIdentity(std::string& identity) 
  {
    std::cout << "Identity received : " << identity << std::endl;
    return EapAuthIdentityStateMachine::Success;
  }
private:
  ~MyEapAuthIdentityStateMachine() {} 
};

// Class definition for peer MD5-Challenge method for my application.
class MyEapPeerMD5ChallengeStateMachine 
  : public EapPeerMD5ChallengeStateMachine
{
  friend class EapMethodStateMachineCreator<MyEapPeerMD5ChallengeStateMachine>;
public:
  MyEapPeerMD5ChallengeStateMachine(EapSwitchStateMachine &s)
    : EapPeerMD5ChallengeStateMachine(s) {} 

  // Reimplemented from EapPeerMD5ChallengeStateMachine.
  void InputPassphrase() 
  {
    std::string &passphrase = Passphrase();
    passphrase.assign("abcd1234");
  }
private:
  ~MyEapPeerMD5ChallengeStateMachine() {} 
};

// Class definition for authenticator MD5-Challenge method for my application.
class MyEapAuthMD5ChallengeStateMachine 
  : public EapAuthMD5ChallengeStateMachine
{
  friend class EapMethodStateMachineCreator<MyEapPeerMD5ChallengeStateMachine>;
public:
  MyEapAuthMD5ChallengeStateMachine(EapSwitchStateMachine &s)
    : EapAuthMD5ChallengeStateMachine(s) {} 

  // Reimplemented from EapPeerMD5ChallengeStateMachine.
  void InputPassphrase() 
  {
    std::string &passphrase = Passphrase();
    passphrase.assign("abcd1234");
  }

private:
  ~MyEapAuthMD5ChallengeStateMachine() {} 
};

class MyPeerSwitchStateMachine: public EapPeerSwitchStateMachine
{
 public:

  MyPeerSwitchStateMachine(ACE_Reactor &r, EapJobHandle& h) 
    : EapPeerSwitchStateMachine(r, h)
  {}

  void Send(AAAMessageBlock *b);

  void Success();

  void Failure();

  void Notification(std::string &str);

  void Abort();

  std::string& InputIdentity();

 private:

  std::string identity;
};

class MyStandAloneAuthSwitchStateMachine 
  : public EapStandAloneAuthSwitchStateMachine
{

 public:

  MyStandAloneAuthSwitchStateMachine(ACE_Reactor &r, EapJobHandle& h) : 
    EapStandAloneAuthSwitchStateMachine(r, h)
  {}

  void Send(AAAMessageBlock *b);

  void Success(AAAMessageBlock *b);

  void Success();

  void Failure(AAAMessageBlock *b);

  void Failure();

  void Abort();

 private:
};

class MyBackendAuthSwitchStateMachine: public EapBackendAuthSwitchStateMachine
{
public:

  MyBackendAuthSwitchStateMachine(ACE_Reactor &r, EapJobHandle& h) : 
    EapBackendAuthSwitchStateMachine(r, h)
  {}

  void Send(AAAMessageBlock *b);

  void Success(AAAMessageBlock *b);

  void Success();

  void Failure(AAAMessageBlock *b);

  void Failure();

  void Abort();

 private:
};

class MyPassThroughAuthSwitchStateMachine
  : public EapPassThroughAuthSwitchStateMachine
{
 public:

  MyPassThroughAuthSwitchStateMachine(ACE_Reactor &r, EapJobHandle& h) 
    : EapPassThroughAuthSwitchStateMachine(r, h)
  {}

  void Send(AAAMessageBlock *b);

  void Success(AAAMessageBlock *b);

  void Success();

  void Failure(AAAMessageBlock *b);

  void Failure();

  void Abort();

  void ForwardResponse(AAAMessageBlock *b);

 private:
};

class PeerChannel : public Channel
{
 public:
  PeerChannel(MyPeerSwitchStateMachine &s) : eap(s) {}
  void Transmit(AAAMessageBlock *msg) { eap.Receive(msg); }
  void Transmit(AAAMessageBlock *msg, int) {}
  MyPeerSwitchStateMachine &eap;
};

class StandAloneAuthChannel : public Channel
{
 public:
  StandAloneAuthChannel(MyStandAloneAuthSwitchStateMachine &s) : eap(s) {}
  void Transmit(AAAMessageBlock *msg) { eap.Receive(msg); }
  void Transmit(AAAMessageBlock *msg, int) {}
  MyStandAloneAuthSwitchStateMachine &eap;
};
class BackendAuthChannel : public Channel
{
 public:
  BackendAuthChannel(MyBackendAuthSwitchStateMachine &s)
    : eap(s), firstMessage(true) {}
  void Transmit(AAAMessageBlock *msg=0) 
  { 
    if (firstMessage)
      {
	msg ? eap.Start(msg) : eap.Start();
	firstMessage = false;
      }
    else
      {
	eap.Receive(msg); 
      }
  }
  void Transmit(AAAMessageBlock *msg, int) {}
  MyBackendAuthSwitchStateMachine &eap;
  bool firstMessage;
};
class PassThroughAuthChannel : public Channel
{
 public:
  PassThroughAuthChannel(MyPassThroughAuthSwitchStateMachine& s) : eap(s) {}

  void Transmit(AAAMessageBlock *msg) { eap.Receive(msg); }
  void Transmit(AAAMessageBlock *msg, int subChannel) 
  { 
    switch(subChannel)
      {
      case 1:
	eap.AAA_Continue(msg); 
	break;
      case 2:
	eap.AAA_Success(msg); 
	break;
      case 3:
	eap.AAA_Failure(msg); 
	break;
      }
  }
  MyPassThroughAuthSwitchStateMachine &eap;
};

class PeerApplication : public AAA_JobData
{
 public:
  PeerApplication(EapTask &task, ACE_Semaphore &sem) : 
    handle(EapJobHandle(AAA_GroupedJob::Create(task.Job(), this, "peer"))),
    eap(boost::shared_ptr<MyPeerSwitchStateMachine>
	(new MyPeerSwitchStateMachine(*task.reactor(), handle))),
    semaphore(sem),
    rxChannel(PeerChannel(*eap)),
    txChannel(0),
    md5Method(EapContinuedPolicyElement(EapType(4)))
  {
    eap->Policy().InitialPolicyElement(&md5Method);
    semaphore.acquire();
  }
  ~PeerApplication() 
  {}
  void Start(Channel *c)
  { 
    txChannel = c;
    eap->Start(); 
  }

  PeerChannel* RxChannel() { return &rxChannel; }

  Channel& TxChannel() { return *txChannel; }

  MyPeerSwitchStateMachine& Eap() { return *eap; }

  ACE_Semaphore& Semaphore() { return semaphore; }

 private:
  EapJobHandle handle;
  boost::shared_ptr<MyPeerSwitchStateMachine> eap;
  ACE_Semaphore &semaphore;
  PeerChannel rxChannel;
  Channel  *txChannel;
  EapContinuedPolicyElement md5Method;
};

// My application session (not used in this test program).
class StandAloneAuthApplication : public AAA_JobData
{

 public:
  StandAloneAuthApplication(EapTask &task, ACE_Semaphore &sem) 
    : handle(EapJobHandle(AAA_GroupedJob::Create(task.Job(), this, "standalone"))),
      eap(boost::shared_ptr<MyStandAloneAuthSwitchStateMachine>
	  (new MyStandAloneAuthSwitchStateMachine(*task.reactor(), handle))),
      semaphore(sem),
      rxChannel(StandAloneAuthChannel(*eap)),
      txChannel(0),
      identityMethod(EapContinuedPolicyElement(EapType(1))),
      md5Method(EapContinuedPolicyElement(EapType(4))),
      notificationMethod(EapContinuedPolicyElement(EapType(2)))
  {
    // Policy settings for the authenticator
    identityMethod.AddContinuedPolicyElement
      (&md5Method, EapContinuedPolicyElement::PolicyOnSuccess);
    identityMethod.AddContinuedPolicyElement
      (&notificationMethod, EapContinuedPolicyElement::PolicyOnFailure);
    eap->Policy().InitialPolicyElement(&identityMethod);
    semaphore.acquire();
  }

  ~StandAloneAuthApplication() {}

  void Start(Channel *c)
  { 
    txChannel = c;
    eap->Start(); 
  }

  StandAloneAuthChannel* RxChannel() { return &rxChannel; }

  Channel& TxChannel() { return *txChannel; }

  //  MyEapStandAloneAuthSession& Eap() { return eap; }
  MyStandAloneAuthSwitchStateMachine& Eap() { return *eap; }

  ACE_Semaphore& Semaphore() { return semaphore; }

 private:
  EapJobHandle handle;
  boost::shared_ptr<MyStandAloneAuthSwitchStateMachine> eap;
  ACE_Semaphore &semaphore;
  StandAloneAuthChannel rxChannel;
  Channel *txChannel;
  EapContinuedPolicyElement identityMethod;
  EapContinuedPolicyElement md5Method;
  EapContinuedPolicyElement notificationMethod;
};

// My application session (not used in this test program).
class BackendAuthApplication : public AAA_JobData
{

 public:
  BackendAuthApplication(EapTask &task, ACE_Semaphore &sem, bool pickup=false)
    : handle(EapJobHandle
	     (AAA_GroupedJob::Create(task.Job(), this, "backend"))),
      eap(boost::shared_ptr<MyBackendAuthSwitchStateMachine>
	  (new MyBackendAuthSwitchStateMachine(*task.reactor(), handle))),
      semaphore(sem),
      rxChannel(BackendAuthChannel(*eap)),
      txChannel(0),
      identityMethod(EapContinuedPolicyElement(EapType(1))),
      md5Method(EapContinuedPolicyElement(EapType(4))),
      notificationMethod(EapContinuedPolicyElement(EapType(2)))
  {
    MyBackendAuthSwitchStateMachine test(*task.reactor(), handle);
    // Policy settings for the backend authenticator
    identityMethod.AddContinuedPolicyElement
      (&notificationMethod, EapContinuedPolicyElement::PolicyOnFailure);
    identityMethod.AddContinuedPolicyElement
      (&md5Method, EapContinuedPolicyElement::PolicyOnSuccess);
      
    eap->Policy().InitialPolicyElement(&identityMethod);

    if (pickup)
      {
	eap->NeedInitialRequestToSend(false);
      }
    semaphore.acquire();
  }
  ~BackendAuthApplication() {}
  void Start(Channel *c) 
  { 
    txChannel = c;
  }
  void Start(Channel *c, AAAMessageBlock *b) 
  { 
    txChannel = c;
  }

  Channel* RxChannel() { return &rxChannel; }

  Channel& TxChannel() { return *txChannel; }

  MyBackendAuthSwitchStateMachine& Eap() { return *eap; }
  
  ACE_Semaphore& Semaphore() { return semaphore; }

 private:
  EapJobHandle handle;
  boost::shared_ptr<MyBackendAuthSwitchStateMachine> eap;
  ACE_Semaphore &semaphore;
  BackendAuthChannel rxChannel;
  Channel *txChannel;
  EapContinuedPolicyElement identityMethod;
  EapContinuedPolicyElement md5Method;
  EapContinuedPolicyElement notificationMethod;
};

// My application session (not used in this test program).
class PassThroughAuthApplication : public AAA_JobData
{

 public:
  PassThroughAuthApplication(EapTask &task, ACE_Semaphore &sem,
			     bool pickup=false)
    : handle(EapJobHandle
	     (AAA_GroupedJob::Create(task.Job(), this, "passthrough"))),
      eap(boost::shared_ptr<MyPassThroughAuthSwitchStateMachine>
	  (new MyPassThroughAuthSwitchStateMachine(*task.reactor(), handle))),
      semaphore(sem),
      rxChannel(PassThroughAuthChannel(*eap)),
      peerTxChannel(0),
      backendTxChannel(0),
      identityMethod(EapContinuedPolicyElement(EapType(1)))
  {
    if (pickup)
      eap->Policy().InitialPolicyElement(&identityMethod);
    semaphore.acquire();
  }
  ~PassThroughAuthApplication() {}

  void Start(Channel *c1, Channel *c2) 
  { 
    peerTxChannel = c1;
    backendTxChannel = c2;
    eap->Start(); 
  }

  Channel* RxChannel() { return &rxChannel; }

  Channel& PeerTxChannel() { return *peerTxChannel; }

  Channel& BackendTxChannel() { return *backendTxChannel; }

  MyPassThroughAuthSwitchStateMachine& Eap() { return *eap; }

  ACE_Semaphore& Semaphore() { return semaphore; }

 private:
  EapJobHandle handle;
  boost::shared_ptr<MyPassThroughAuthSwitchStateMachine> eap;
  ACE_Semaphore &semaphore;
  PassThroughAuthChannel rxChannel;
  Channel *peerTxChannel;
  Channel *backendTxChannel;
  EapContinuedPolicyElement identityMethod;
};

// ----------------- Definition --------------
void MyPeerSwitchStateMachine::Send(AAAMessageBlock *b)
{
  std::cout << "EAP Response sent from peer" << std::endl;
  JobData(Type2Type<PeerApplication>()).TxChannel().Transmit(b);
}

void MyPeerSwitchStateMachine::Success()
  {
    std::cout << "Authentication success detected at peer" << std::endl;
    std::cout << "Welcome to the world, " 
	      << PeerIdentity() 
	      << " !!!" << std::endl;
    Stop();
    JobData(Type2Type<PeerApplication>()).Semaphore().release();
  }
void MyPeerSwitchStateMachine::Failure()
  {
    std::cout << "Authentication failure detected at peer" << std::endl;
    std::cout << "Sorry, " 
	      << PeerIdentity() 
	      << " try next time !!!" << std::endl;
    Stop();
    JobData(Type2Type<PeerApplication>()).Semaphore().release();
  }
void MyPeerSwitchStateMachine::Notification(std::string &str)
  {
    std::cout << "Following notification received" << std::endl;
    std::cout << str << std::endl;
  }
void MyPeerSwitchStateMachine::Abort()
  {
    std::cout << "Peer aborted for an error in state machine" << std::endl;
    JobData(Type2Type<PeerApplication>()).Semaphore().release();
  }
std::string& MyPeerSwitchStateMachine::InputIdentity() 
  {
    identity = std::string("ohba");
#if 0
    std::cout << "Input username (within 10sec.): " << std::endl;
    std::cin >> identity;
    std::cout << "username = " << identity << std::endl;
#endif
    return identity;
  }

void MyStandAloneAuthSwitchStateMachine::Send(AAAMessageBlock *b)
  {
    std::cout << "EAP Request sent from authenticator" << std::endl;
    JobData(Type2Type<StandAloneAuthApplication>()).TxChannel().Transmit(b);
  }
void MyStandAloneAuthSwitchStateMachine::Success(AAAMessageBlock *b)
  {
    std::cout << "EAP Success sent from authenticator" << std::endl;
    JobData(Type2Type<StandAloneAuthApplication>()).TxChannel().Transmit(b);
    Stop();
    JobData(Type2Type<StandAloneAuthApplication>()).Semaphore().release();
  }
void MyStandAloneAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
    Stop();
    JobData(Type2Type<StandAloneAuthApplication>()).Semaphore().release();
  }
void MyStandAloneAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
  {
    std::cout << "EAP Failure sent from authenticator" << std::endl;
    JobData(Type2Type<StandAloneAuthApplication>()).TxChannel().Transmit(b);
    Stop();
    JobData(Type2Type<StandAloneAuthApplication>()).Semaphore().release();
  }
void MyStandAloneAuthSwitchStateMachine::Failure()
  {
    std::cout << "Failure without an EAP Failure" << std::endl;
    Stop();
    JobData(Type2Type<StandAloneAuthApplication>()).Semaphore().release();
  }
void MyStandAloneAuthSwitchStateMachine::Abort()
  {
    std::cout << "Session aborted for an error in state machine" << std::endl;
    Stop();
    JobData(Type2Type<StandAloneAuthApplication>()).Semaphore().release();
  }

void MyBackendAuthSwitchStateMachine::Send(AAAMessageBlock *b)
  {
    std::cout << "EAP Request sent from authenticator" << std::endl;
    JobData(Type2Type<BackendAuthApplication>()).TxChannel().Transmit(b, 1);
  }
void MyBackendAuthSwitchStateMachine::Success(AAAMessageBlock *b)
  {
    std::cout << "EAP Success sent from authenticator" << std::endl;
    JobData(Type2Type<BackendAuthApplication>()).TxChannel().Transmit(b, 2);
    Stop();
    JobData(Type2Type<BackendAuthApplication>()).Semaphore().release();
  }
void MyBackendAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
    Stop();
    JobData(Type2Type<BackendAuthApplication>()).Semaphore().release();
  }
void MyBackendAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
  {
    std::cout << "EAP Failure sent from authenticator" << std::endl;
    JobData(Type2Type<BackendAuthApplication>()).TxChannel().Transmit(b, 3);
    Stop();
    JobData(Type2Type<BackendAuthApplication>()).Semaphore().release();
  }
void MyBackendAuthSwitchStateMachine::Failure()
  {
    std::cout << "Failure without an EAP Failure" << std::endl;
    Stop();
    JobData(Type2Type<BackendAuthApplication>()).Semaphore().release();
  }
void MyBackendAuthSwitchStateMachine::Abort()
  {
    std::cout << "Session aborted for an error in state machine" << std::endl;
    Stop();
    JobData(Type2Type<BackendAuthApplication>()).Semaphore().release();
  }

void MyPassThroughAuthSwitchStateMachine::Send(AAAMessageBlock *b)
  {
    std::cout << "EAP Request sent from passthrough authenticator" 
	      << std::endl;
    JobData(Type2Type<PassThroughAuthApplication>()).
      PeerTxChannel().Transmit(b);
  }
void MyPassThroughAuthSwitchStateMachine::Success(AAAMessageBlock *b)
  {
    std::cout << "EAP Success sent from passthrough authenticator" 
	      << std::endl;
    JobData(Type2Type<PassThroughAuthApplication>()).
      PeerTxChannel().Transmit(b);
    Stop();
    JobData(Type2Type<PassThroughAuthApplication>()).Semaphore().release();
  }
void MyPassThroughAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
    Stop();
    JobData(Type2Type<PassThroughAuthApplication>()).Semaphore().release();
  }
void MyPassThroughAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
  {
    std::cout << "EAP Failure sent from passthrough authenticator" 
	      << std::endl;
    JobData(Type2Type<PassThroughAuthApplication>()).
      PeerTxChannel().Transmit(b);
    Stop();
    JobData(Type2Type<PassThroughAuthApplication>()).Semaphore().release();
  }
void MyPassThroughAuthSwitchStateMachine::Failure()
  {
    std::cout << "Failure without an EAP Failure" << std::endl;
    Stop();
    JobData(Type2Type<PassThroughAuthApplication>()).Semaphore().release();
  }
void MyPassThroughAuthSwitchStateMachine::Abort()
  {
    std::cout << "Session aborted for an error in state machine" << std::endl;
    Stop();
    JobData(Type2Type<PassThroughAuthApplication>()).Semaphore().release();
  }
void MyPassThroughAuthSwitchStateMachine::ForwardResponse(AAAMessageBlock *b)
  {
    // if this is the first message from the peer, then create the
    // authenticator on the EAP server and start it.
    if (b)
	std::cout << "Passthrough authenticator is forwarding an EAP-Response "
		  << "to EAP server" << std::endl;
    else
	std::cout << "Passthrough authenticator is sending a null EAP message"
	          << "to EAP server to start EAP." << std::endl;

    JobData(Type2Type<PassThroughAuthApplication>()).
      BackendTxChannel().Transmit(b);
  }

int main(int argc, char **argv)
{
  // Initialize the log.
#ifdef WIN32
  EapLogMsg_S::instance()->open("EAP", ACE_Log_Msg::STDERR);
#else
  EapLogMsg_S::instance()->open("EAP", ACE_Log_Msg::SYSLOG);
#endif
  EapLogMsg_S::instance()->enable_debug_messages();

  // Register the mapping from EapType to the creator of the
  // user-defined method class for each user-defined method
  // implementation.

  EapMethodStateMachineCreator<MyEapAuthIdentityStateMachine> 
    myAuthIdentityCreator;

  EapMethodStateMachineCreator<MyEapPeerMD5ChallengeStateMachine> 
    myPeerMD5ChallengeCreator;

  EapMethodStateMachineCreator<MyEapAuthMD5ChallengeStateMachine> 
    myAuthMD5ChallengeCreator;

  EapMethodRegistrar methodRegistrar;

  methodRegistrar.registerMethod
    (std::string("Identity"), EapType(1), 
     Authenticator, myAuthIdentityCreator);

  methodRegistrar.registerMethod
    (std::string("MD5-Challenge"), EapType(4), 
     Peer, myPeerMD5ChallengeCreator);

  methodRegistrar.registerMethod
    (std::string("MD5-Challenge"), EapType(4), Authenticator, 
     myAuthMD5ChallengeCreator);

  int com;

 again:
  // Input command.
  std::cout << "1 - peer<->authenticator exchange" 
	    << std::endl;
  std::cout << "2 - peer<->passthrough authenticator<->backend authenticator"
	    << std::endl; 
  std::cout << "    exchange (backend originates Request/Identity)" 
	    << std::endl;
  std::cout << "3 - peer<->passthrough authenticator<->backend authenticator" 
	    << std::endl; 
  std::cout << "    exchange (passthrough originates Request/Identity)" 
	    << std::endl;
  std::cout << "Input command (1-3) [type Ctrl-C to exit anytime] " 
	    << std::endl;
  std::cin >> com;

  std::cout << "Input command: " << com << std::endl;
  
  if (com<1 || com>3)
    {
      std::cout << "Invalid command" << std::endl;
      goto again;
    }

  bool pickup = ((com==3) ? true : false);
  EapTask task;

  task.Start(2);

#if WIN32
  #define num 100
#else
  int num = 100;
#endif

  ACE_Semaphore semaphore(4*num);

  std::auto_ptr<PeerApplication> peerApp[num];
  std::auto_ptr<StandAloneAuthApplication> standAloneAuthApp[num];
  std::auto_ptr<BackendAuthApplication> backendAuthApp[num];
  std::auto_ptr<PassThroughAuthApplication> passThroughAuthApp[num];

  for (int i=0; i<num; i++)
    {
      peerApp[i] = 
	std::auto_ptr<PeerApplication>
	(new PeerApplication(task, semaphore));
      standAloneAuthApp[i] = 
	std::auto_ptr<StandAloneAuthApplication>
	(new StandAloneAuthApplication(task, semaphore));
      backendAuthApp[i] = 
	std::auto_ptr<BackendAuthApplication>
	(new BackendAuthApplication(task, semaphore, pickup));
      passThroughAuthApp[i] = 
	std::auto_ptr<PassThroughAuthApplication>
	(new PassThroughAuthApplication (task, semaphore, pickup));

      if (com==1)
	{
	  peerApp[i]->Start(standAloneAuthApp[i]->RxChannel());
	  standAloneAuthApp[i]->Start(peerApp[i]->RxChannel());
	}
      else
	{
	  peerApp[i]->Start(passThroughAuthApp[i]->RxChannel());
	  // Starting order of passthrough and backend authenticators are
	  // important.
	  if (com==2)
	    {
	      backendAuthApp[i]->Start(passThroughAuthApp[i]->RxChannel());
	      passThroughAuthApp[i]->Start
		(peerApp[i]->RxChannel(), backendAuthApp[i]->RxChannel());
	    }
	  else
	    {
	      passThroughAuthApp[i]->Start
		(peerApp[i]->RxChannel(), backendAuthApp[i]->RxChannel());
	      backendAuthApp[i]->Start(passThroughAuthApp[i]->RxChannel());
	    }
	}
    }
  // Block until the EAP conversation completes.
  if (com==1)
    for (int i=0; i<2*num; i++)
      semaphore.acquire();
  else
    for (int i=0; i<3*num; i++)
      semaphore.acquire();

  task.Stop();
  return 0;
}


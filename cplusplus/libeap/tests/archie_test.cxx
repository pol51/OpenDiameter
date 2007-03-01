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
// $Id: archie_test.cxx,v 1.9 2006/03/16 17:01:52 vfajardo Exp $ 
// A test program for EAP API.
// Written by Yoshihiro Ohba

#include "eap.hxx"
#include "eap_peerfsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_identity.hxx"
#include "eap_policy.hxx"
#include "eap_method_registrar.hxx"
#include "eap_fsm.hxx"
#include "eap_log.hxx"
#include "eap_md5.hxx"
#include "eap_archie.hxx"
#include "eap_archie_fsm.hxx"
#include <openssl/rand.h>
#include <iostream>

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
  EapTask() : AAA_Task(AAA_SCHED_WFQ, "EAP") 
  {}

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
  virtual void Transmit(AAAMessageBlock *msg, std::string& key)=0;
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

  All cases completes with success after EAP-Archie authentication
  method is performed.  In both cases, the peer session entity will
  prompt you to input a username.  Once the username is input, it is
  carried in an Response/Archie-Response message and sent to the
  (backend) authenticator.  In Case 2, passthrough authenticator will
  forward the response to the backend authenticator.

  The authenticator or passthrough authenticator will retransmit the
  Request/Archie-Request message until you input the username or the
  number of retransmission reaches its maximum value (the maximum
  value is set to 3 in this program).  For the latter case, the
  authenticator will stop its state machine and the peer will timeout
  and fail.

  Message passing between the entities are based on callbacks.  For
  example, when an authenticator sends an Request, Send() callback
  function is called from the authenticator state machine.  In this
  program, the contents of Send() is simply calling the Receive()
  function of the communicating peer entity.  For example, in Case 1,
  Send() of the authenticator calls Receive() of the peer.

  Case 1:

  Peer                Authenticator
   |                          |
   |  Request/Archie-Request  |
   |<-------------------------|
   |  Response/Archie-Response|
   |------------------------->|
   |  Request/Archie-Confirm  |
   |<-------------------------|
   |  Response/Archie-Finish  |
   |------------------------->|
   |                          |
   |       Success            |
   |<-------------------------|


Case 2:

  Peer                  PassThough                   Backend
                        Authenticator                Authenticator
   |                          |  Null message            |
   |                          |------------------------->|
   |                          |                          |
   |                          |  Request/Archie-Request  |
   |  Request/Archie-Request  |<-------------------------|
   |<-------------------------|                          |
   |                          |                          |
   |  Response/Archie-Response|                          |
   |------------------------->|  Response/Archie-Response|
   |                          |------------------------->|
   |                          |  Request/Archie-Confirm  |
   |  Request/Archie-Confirm  |<-------------------------|
   |<-------------------------|                          |
   |  Response/Archie-Finish  |                          |
   |------------------------->|  Response/Archie-Finish  |
   |                          |------------------------->|
   |                          |                          |
   |                          |          Success         |
   |       Success            |<-------------------------|
   |<-------------------------|

 */

static std::string sharedSecret;

// Class definition for peer archie state machine.
class MyEapPeerArchieStateMachine : public EapPeerArchieStateMachine
{
  friend class EapMethodStateMachineCreator<MyEapPeerArchieStateMachine>;
public:
  MyEapPeerArchieStateMachine(EapSwitchStateMachine &s)
    : EapPeerArchieStateMachine(s) {}

  /// This pure virtual function is a callback used when a shared-secret 
  /// needs to be obtained.
  std::string& InputSharedSecret()
  {
    return ::sharedSecret;
  }

  /// This pure virtual function is a callback used when an AuthID
  /// needs to be obtained.
  std::string& InputIdentity()
  {
    std::cout << "Received an Archie-Request from " 
	      << AuthID() << std::endl;
      
    static std::string identity;
    std::cout << "Input username (within 10sec.): " << std::endl;
    std::cin >> identity;
    std::cout << "username = " << identity << std::endl;
    return identity;
  }
private:
  ~MyEapPeerArchieStateMachine() {} 
};

// Class definition for authenticator identity method for my application.
class MyEapAuthArchieStateMachine : public EapAuthArchieStateMachine
{
  friend class EapMethodStateMachineCreator<MyEapAuthArchieStateMachine>;
public:
  MyEapAuthArchieStateMachine(EapSwitchStateMachine &s) :
    EapAuthArchieStateMachine(s) {}

  /// This pure virtual function is a callback used when a shared-secret 
  /// needs to be obtained.
  std::string& InputSharedSecret()
  {
    return ::sharedSecret;
  }

  /// This pure virtual function is a callback used when an AuthID
  /// needs to be obtained.
  std::string& InputIdentity()
  {
    static std::string serverID("myserver@opendiameter.org");
    return serverID;
  }

private:
  ~MyEapAuthArchieStateMachine() {} 
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

  MyStandAloneAuthSwitchStateMachine(ACE_Reactor &r, EapJobHandle& h)
    : EapStandAloneAuthSwitchStateMachine(r, h)
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

  MyBackendAuthSwitchStateMachine(ACE_Reactor &r, EapJobHandle& h)
    : EapBackendAuthSwitchStateMachine(r, h)
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
  void Transmit(AAAMessageBlock *msg, std::string&) {}
  MyPeerSwitchStateMachine &eap;
};

class StandAloneAuthChannel : public Channel
{
 public:
  StandAloneAuthChannel(MyStandAloneAuthSwitchStateMachine &s) : eap(s) {}
  void Transmit(AAAMessageBlock *msg) { eap.Receive(msg); }
  void Transmit(AAAMessageBlock *msg, int) {}
  void Transmit(AAAMessageBlock *msg, std::string&) {}
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
  void Transmit(AAAMessageBlock *msg, std::string& key) {}
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
  void Transmit(AAAMessageBlock *msg, std::string& key) 
  {
    eap.AAA_Success(msg, key); 
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
    method(EapContinuedPolicyElement(EapType(ARCHIE_METHOD_TYPE)))
  {
    eap->Policy().InitialPolicyElement(&method);
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
  EapContinuedPolicyElement method;
};

// My application session (not used in this test program).
class StandAloneAuthApplication : public AAA_JobData
{

 public:
  StandAloneAuthApplication(EapTask &task, ACE_Semaphore &sem) 
    : handle(EapJobHandle
	     (AAA_GroupedJob::Create(task.Job(), this, "standalone"))),
      eap(boost::shared_ptr<MyStandAloneAuthSwitchStateMachine>
	  (new MyStandAloneAuthSwitchStateMachine(*task.reactor(), handle))),
      semaphore(sem),
      rxChannel(StandAloneAuthChannel(*eap)),
      txChannel(0),
      method(EapContinuedPolicyElement(EapType(ARCHIE_METHOD_TYPE)))
  {
    // Policy settings for the authenticator
    eap->Policy().InitialPolicyElement(&method);

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

  MyStandAloneAuthSwitchStateMachine& Eap() { return *eap; }

  ACE_Semaphore& Semaphore() { return semaphore; }

 private:
  EapJobHandle handle;
  boost::shared_ptr<MyStandAloneAuthSwitchStateMachine> eap;
  ACE_Semaphore &semaphore;
  StandAloneAuthChannel rxChannel;
  Channel *txChannel;
  EapContinuedPolicyElement method;
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
      method(EapContinuedPolicyElement(EapType(ARCHIE_METHOD_TYPE)))
  {
    // Policy settings for the backend authenticator
    eap->Policy().InitialPolicyElement(&method);

    if (pickup)
      {
	eap->NeedInitialRequestToSend(false);
      }

    // Set shared secret.
    unsigned char tmp[64];
    RAND_bytes(tmp, sizeof tmp);
    sharedSecret = std::string((char*)tmp, sizeof tmp);

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
  EapContinuedPolicyElement method;
  std::string sharedSecret;
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
      method(EapContinuedPolicyElement(EapType(1)))
  {
    if (pickup)
      eap->Policy().InitialPolicyElement(&method);
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
  EapContinuedPolicyElement method;
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

    if (!KeyAvailable())
      {
	std::cout << "Error: key is not available" << std::endl;
	Abort();
	return;
      }

    for (int i=0; i<32; i++)
      {
	char c[100];
	const char* p = KeyData().data();
	sprintf(c, "%02X ", *(unsigned char*)(p+i));
	std::cout << c;
	if ((i+1) % 16 == 0) std::cout << std::endl;
	  
      }
    std::cout << std::endl;
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
    std::cout << "Input username (within 10sec.): " << std::endl;
    std::cin >> identity;
    std::cout << "username = " << identity << std::endl;
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
    if (!KeyAvailable())
      {
	std::cout << "Error: key is not available" << std::endl;
	Abort();
	return;
      }
    for (int i=0; i<32; i++)
      {
	char c[100];
	const char* p = KeyData().data();
	sprintf(c, "%02X ", *(unsigned char*)(p+i));
	std::cout << c;
	if ((i+1) % 16 == 0) std::cout << std::endl;
	  
      }
    std::cout << std::endl;
    Stop();
  }
void MyStandAloneAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
    Stop();
  }
void MyStandAloneAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
  {
    std::cout << "EAP Failure sent from authenticator" << std::endl;
    JobData(Type2Type<StandAloneAuthApplication>()).TxChannel().Transmit(b);
    Stop();
  }
void MyStandAloneAuthSwitchStateMachine::Failure()
  {
    std::cout << "Failure without an EAP Failure" << std::endl;
    Stop();
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
    if (!KeyAvailable())
      {
	std::cout << "Error: key is not available" << std::endl;
	Abort();
	return;
      }

    for (int i=0; i<32; i++)
      {
	char c[100];
	const char* p = KeyData().data();
	sprintf(c, "%02X ", *(unsigned char*)(p+i));
	std::cout << c;
	if ((i+1) % 16 == 0) std::cout << std::endl;
	  
      }
    std::cout << std::endl;
    std::cout << "EAP Success sent from authenticator" << std::endl;
    JobData(Type2Type<BackendAuthApplication>()).
      TxChannel().Transmit(b, KeyData());
    Stop();
  }
void MyBackendAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
    Stop();
  }
void MyBackendAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
  {
    std::cout << "EAP Failure sent from authenticator" << std::endl;
    JobData(Type2Type<BackendAuthApplication>()).TxChannel().Transmit(b, 3);
    Stop();
  }
void MyBackendAuthSwitchStateMachine::Failure()
  {
    std::cout << "Failure without an EAP Failure" << std::endl;
    Stop();
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
  }
void MyPassThroughAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
    Stop();
  }
void MyPassThroughAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
  {
    std::cout << "EAP Failure sent from passthrough authenticator" 
	      << std::endl;
    JobData(Type2Type<PassThroughAuthApplication>()).
      PeerTxChannel().Transmit(b);
    Stop();
  }
void MyPassThroughAuthSwitchStateMachine::Failure()
  {
    std::cout << "Failure without an EAP Failure" << std::endl;
    Stop();
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
#ifndef WIN32
  EapLogMsg_S::instance()->open("EAP", ACE_Log_Msg::SYSLOG);
#else
  EapLogMsg_S::instance()->open("EAP", ACE_Log_Msg::STDERR);
#endif
  EapLogMsg_S::instance()->enable_debug_messages();

  // Register the mapping from EapType to the creator of the
  // user-defined method class for each user-defined method
  // implementation.

  EapMethodStateMachineCreator<MyEapPeerArchieStateMachine> 
    myPeerArchieCreator;

  EapMethodStateMachineCreator<MyEapAuthArchieStateMachine> 
    myAuthArchieCreator;

  EapMethodRegistrar methodRegistrar;

  methodRegistrar.registerMethod
    (std::string("Archie"), EapType(ARCHIE_METHOD_TYPE), 
     Peer, myPeerArchieCreator);

  methodRegistrar.registerMethod
    (std::string("Archie"), EapType(ARCHIE_METHOD_TYPE), 
     Authenticator, myAuthArchieCreator);

  int com;

 again:
  // Input command.
  std::cout << "1 - peer<->authenticator exchange" 
	    << std::endl;
  std::cout << "2 - peer<->passthrough authenticator<->backend authenticator"
	    << std::endl; 
  std::cout << "    exchange (backend originates Request/Identity)" 
	    << std::endl;
  std::cout << "Input command (1-2) [type Ctrl-C to exit anytime] " 
	    << std::endl;
  std::cin >> com;

  std::cout << "Input command: " << com << std::endl;

  if (com!=1 && com!=2)
    {
      std::cout << "Invalid command" << std::endl;
      goto again;
    }
  
  EapTask task;

  try {
    // Task starts with two threads in the thread pool.
    task.Start(2);
  }
  catch (...) {
    std::cout << "Task failed to start" << std::endl;
    exit(1);
  }

  // Set shared secret.
  unsigned char tmp[64];
  RAND_bytes(tmp, sizeof tmp);
  sharedSecret = std::string((char*)tmp, sizeof tmp);

  ACE_Semaphore semaphore(4);

  PeerApplication peerApp(task, semaphore);
  StandAloneAuthApplication standAloneAuthApp(task, semaphore);
  BackendAuthApplication backendAuthApp(task, semaphore);
  PassThroughAuthApplication passThroughAuthApp(task, semaphore);

  if (com==1)
    {
      peerApp.Start(standAloneAuthApp.RxChannel());
      standAloneAuthApp.Start(peerApp.RxChannel());
    }
  else
    {
      peerApp.Start(passThroughAuthApp.RxChannel());
      passThroughAuthApp.Start
      	(peerApp.RxChannel(), backendAuthApp.RxChannel());
      backendAuthApp.Start(passThroughAuthApp.RxChannel());
    }
  // Block until the EAP conversation completes.
  semaphore.acquire();

  task.Stop();
  return 0;
}


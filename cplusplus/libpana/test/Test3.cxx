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

// $Id: Test3.cxx,v 1.38 2006/05/04 19:46:40 vfajardo Exp $ 


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
#include "pana_node.h"
#include "pana_client_fsm.h"
#include "pana_paa_fsm.h"
#include "pana_paa_factory.h"
#include "ace/Get_Opt.h"
#include <openssl/rand.h>
#include <iostream>
#include "user_db.h"
#include "pana_auth_script.h"

class MyPeerSwitchStateMachine;
class MyStandAloneAuthSwitchStateMachine;
class StandAloneAuthApplication;

static std::string gUserName;

/// Task class used in this sample program.
class EapTask : public AAA_Task
{
 public:
  /// Constructor.
  EapTask(std::string &cfgfile) :
      AAA_Task(AAA_SCHED_WFQ, "EAP"),
      node(*this, cfgfile) {
  }

  /// Destructor.
  ~EapTask() {}

  PANA_Node node;
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
  virtual void Transmit(AAAMessageBlock *msg, std::string &key)=0;
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
static std::string tempKeyStorage;

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
    if (USER_DB_LOOKUP(identity)) {
        std::cout << "Valid Identity received : " << identity << std::endl;
        return EapAuthIdentityStateMachine::Success;
    }
    std::cout << "Invalid Identity received : " << identity << std::endl;
    return EapAuthIdentityStateMachine::Failure;
  }
private:
  ~MyEapAuthIdentityStateMachine() {} 
};

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
      
    std::cout << "Received an Archie-Request from " 
	      << AuthID() << std::endl;
    std::cout << "username = " << AuthSwitchStateMachine().PeerIdentity() << std::endl;
    return AuthSwitchStateMachine().PeerIdentity();
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

class PeerChannel : public PANA_ClientEventInterface
{
 public:
  PeerChannel(PANA_Node &n,
              MyPeerSwitchStateMachine &s,
              ACE_Semaphore &sem) :
      eap(s), pana(n, *this), semaphore(sem) {
  }
  virtual ~PeerChannel() {
  }
  void Initialize() {
      pana.Start(); // initialize
  }
  void EapStart() {
      eap.Stop();
      eap.Start();
  }
  void EapRequest(AAAMessageBlock *request) {
      eap.Receive(request);
  }
  void EapAltReject() {
  }
  void Authorize(PANA_AuthorizationArgs &args) {
     PANA_AuthScriptCtl::Print(args);
     // send an address update request
#if defined(ACE_HAS_IPV6)
     ACE_INET_Addr newAddr("2001::1111:0");
#else
     ACE_INET_Addr newAddr("127.0.0.1:0");
#endif
     pana.Update(newAddr);
  }
  bool IsKeyAvailable(pana_octetstring_t &key) {
    if (eap.KeyAvailable()) {
       for (int i=0; i<32; i++)
         {
	   char c[100];
           const char* p = eap.KeyData().data();
           sprintf(c, "%02X ", *(unsigned char*)(p+i));
           std::cout << c;
           if ((i+1) % 16 == 0) std::cout << std::endl;
         }

       std::cout << "Assigning key" << std::endl;
       key.assign(eap.KeyData().data(), eap.KeyData().size());
       return true;
    }
    return false;
  }
  void Disconnect(ACE_UINT32 cause) {
      eap.Stop();
      semaphore.release();
  }
  void Error(ACE_UINT32 resultCode) { 
      eap.Stop();
  }
  void Stop() {
      pana.Stop();
  }
  MyPeerSwitchStateMachine &eap;
  PANA_PacSession pana;
  ACE_Semaphore &semaphore;
};

class PassThroughAuthChannel : public PANA_PaaEventInterface
{
 public:
    class LocalBackendTxChannel : public Channel {
      public:
        LocalBackendTxChannel(MyPassThroughAuthSwitchStateMachine &s) :
            eap(s) { }
        void Transmit(AAAMessageBlock *msg) { }
        void Transmit(AAAMessageBlock *msg, int subChannel) {
            switch(subChannel) {
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
        void Transmit(AAAMessageBlock *msg, std::string &key) {
	    eap.AAA_Success(msg, key);
        }
      private:
        MyPassThroughAuthSwitchStateMachine &eap;
    };

 public:
    PassThroughAuthChannel(PANA_PaaSessionChannel &ch,
                           MyPassThroughAuthSwitchStateMachine &s) :
       paaSession(ch, *this), eap(s),
       backendTxChannel(s) {
   }
   virtual ~PassThroughAuthChannel() {
   }
   virtual void EapStart() {
      eap.Stop(); 
      eap.Start();
   }
   virtual void EapResponse(AAAMessageBlock *response) {
      eap.Receive(response);
   }
   void EapAltReject() {
   }
   void Authorize(PANA_AuthorizationArgs &args) {
      PANA_AuthScriptCtl::Print(args);
   }
   bool IsKeyAvailable(pana_octetstring_t &key) {
    if (eap.KeyAvailable()) {
       for (int i=0; i<32; i++)
         {
	   char c[100];
           const char* p = eap.KeyData().data();
           sprintf(c, "%02X ", *(unsigned char*)(p+i));
           std::cout << c;
           if ((i+1) % 16 == 0) std::cout << std::endl;
         }

       std::cout << "Assigning key" << std::endl;
       key.assign(eap.KeyData().data(), eap.KeyData().size());
       return true;
     }
     return false;
   }
   bool IsUserAuthorized() {
      return true;
   }
   void Disconnect(ACE_UINT32 cause) {
      eap.Stop();
   }
   void Timeout(PANA_TID id) {
      eap.Stop();
   }
   void Error(ACE_UINT32 resultCode) { 
      eap.Stop();
   }
   EapPassThroughAuthSwitchStateMachine& Eap() { 
      return eap; 
   }
   PANA_PaaSession &Pana() {
      return paaSession;
   }
   Channel &BackendTxChannel() {
      return backendTxChannel;
   }
   PANA_PaaSession &PaaSession() {
      return paaSession;
   }
 protected:
  PANA_PaaSession paaSession;
  MyPassThroughAuthSwitchStateMachine &eap;
  LocalBackendTxChannel backendTxChannel;
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
  void Transmit(AAAMessageBlock *msg, std::string &key) {}
  MyBackendAuthSwitchStateMachine &eap;
  bool firstMessage;
};

class PeerApplication : public AAA_JobData
{
 public:
  PeerApplication(EapTask &task, ACE_Semaphore &sem) : 
    handle(EapJobHandle(AAA_GroupedJob::Create(task.Job(), this, "peer"))),
    eap(boost::shared_ptr<MyPeerSwitchStateMachine>
	(new MyPeerSwitchStateMachine(*task.reactor(), handle))),
    semaphore(sem),
    channel(task.node, *eap, sem),
    method(EapContinuedPolicyElement(EapType(ARCHIE_METHOD_TYPE)))
  {
    eap->Policy().InitialPolicyElement(&method);
  }
  ~PeerApplication() 
  {}

  PeerChannel &Channel() { return channel; }

  MyPeerSwitchStateMachine& Eap() { return *eap; }

  ACE_Semaphore& Semaphore() { return semaphore; }

 private:
  EapJobHandle handle;
  boost::shared_ptr<MyPeerSwitchStateMachine> eap;
  ACE_Semaphore &semaphore;
  PeerChannel channel;
  EapContinuedPolicyElement method;
};

// My application session (not used in this test program).
class PassThroughAuthApplication : public AAA_JobData
{

 public:
    PassThroughAuthApplication(PANA_PaaSessionChannel &ch,
                               bool pickup=false)
    : handle(EapJobHandle
	     (AAA_GroupedJob::Create
              (ch.Node().Task().Job(), this, "passthrough"))),
      eap(boost::shared_ptr<MyPassThroughAuthSwitchStateMachine>
	  (new MyPassThroughAuthSwitchStateMachine
           (*ch.Node().Task().reactor(), handle))),
      channel(ch, *eap),
      backendRxChannel(0),
      method(EapContinuedPolicyElement(EapType(1)))
  {
      if (pickup)
        eap->Policy().InitialPolicyElement(&method);
  }
  ~PassThroughAuthApplication() {}

  void Start(Channel *rxBackend)
  { 
    backendRxChannel = rxBackend;
  }

  Channel& BackendTxChannel() { return channel.BackendTxChannel(); }

  Channel& BackendRxChannel() { return *backendRxChannel; }
    
  MyPassThroughAuthSwitchStateMachine& Eap() { return *eap; }

  PANA_PaaSession &Pana() { return channel.Pana(); }  

 private:
  EapJobHandle handle;
  boost::shared_ptr<MyPassThroughAuthSwitchStateMachine> eap;
  PassThroughAuthChannel channel;
  Channel *backendRxChannel;
  EapContinuedPolicyElement method;
};

// My application session (not used in this test program).
class BackendAuthApplication : public AAA_JobData
{

 public:
  BackendAuthApplication(EapTask &task, bool pickup=false)
    : handle(EapJobHandle
	     (AAA_GroupedJob::Create(task.Job(), this, "backend"))),
      eap(boost::shared_ptr<MyBackendAuthSwitchStateMachine>
	  (new MyBackendAuthSwitchStateMachine(*task.reactor(), handle))),
      rxChannel(BackendAuthChannel(*eap)),
      txChannel(0),
      identityMethod(EapContinuedPolicyElement(EapType(1))),
      archieMethod(EapContinuedPolicyElement(EapType(ARCHIE_METHOD_TYPE)))
  {
    // Policy settings for the authenticator
    identityMethod.AddContinuedPolicyElement
      (&archieMethod, EapContinuedPolicyElement::PolicyOnSuccess);
    eap->Policy().InitialPolicyElement(&identityMethod);

    if (pickup)
      {
	eap->NeedInitialRequestToSend(false);
      }
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

 private:
  EapJobHandle handle;
  boost::shared_ptr<MyBackendAuthSwitchStateMachine> eap;
  BackendAuthChannel rxChannel;
  Channel *txChannel;
  EapContinuedPolicyElement identityMethod;
  EapContinuedPolicyElement archieMethod;
};

// ----------------- Definition --------------
void MyPeerSwitchStateMachine::Send(AAAMessageBlock *b)
{
  std::cout << "EAP Response sent from peer" << std::endl;
  JobData(Type2Type<PeerApplication>()).Channel().pana.EapSendResponse(b);
}

void MyPeerSwitchStateMachine::Success()
  {
    std::cout << "Authentication success detected at peer" << std::endl;
    std::cout << "Welcome to the world, " 
	      << PeerIdentity() 
	      << " !!!" << std::endl;
  JobData(Type2Type<PeerApplication>()).Channel().pana.EapSuccess();
  }
void MyPeerSwitchStateMachine::Failure()
  {
    std::cout << "Authentication failure detected at peer" << std::endl;
    std::cout << "Sorry, " 
	      << PeerIdentity() 
	      << " try next time !!!" << std::endl;
    Stop();
    JobData(Type2Type<PeerApplication>()).Channel().pana.EapFailure();
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
    JobData(Type2Type<PeerApplication>()).Channel().pana.EapFailure();
    JobData(Type2Type<PeerApplication>()).Semaphore().release();
  }
std::string& MyPeerSwitchStateMachine::InputIdentity() 
  {
    if (gUserName.length() > 0) {
        return gUserName;
    }
    std::cout << "Input username (within 10sec.): " << std::endl;
    std::cin >> identity;
    std::cout << "username = " << identity << std::endl;
    return identity;
  }
void MyPassThroughAuthSwitchStateMachine::Send(AAAMessageBlock *b)
  {
    std::cout << "EAP Request sent from passthrough authenticator" 
	      << std::endl;
    JobData(Type2Type<PassThroughAuthApplication>()).
        Pana().EapSendRequest(b);
  }
void MyPassThroughAuthSwitchStateMachine::Success(AAAMessageBlock *b)
  {
    std::cout << "EAP Success sent from passthrough authenticator" 
	      << std::endl;
    JobData(Type2Type<PassThroughAuthApplication>()).
        Pana().EapSuccess(b);
  }
void MyPassThroughAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
    JobData(Type2Type<PassThroughAuthApplication>()).
        Pana().EapSuccess();
  }
void MyPassThroughAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
  {
    std::cout << "EAP Failure sent from passthrough authenticator" 
	      << std::endl;
    JobData(Type2Type<PassThroughAuthApplication>()).
        Pana().EapFailure(b);
    Stop();
  }
void MyPassThroughAuthSwitchStateMachine::Failure()
  {
    std::cout << "Failure without an EAP Failure" << std::endl;
    JobData(Type2Type<PassThroughAuthApplication>()).
        Pana().EapFailure();
    Stop();
  }
void MyPassThroughAuthSwitchStateMachine::Abort()
  {
    std::cout << "Session aborted for an error in state machine" << std::endl;
    JobData(Type2Type<PassThroughAuthApplication>()).
        Pana().EapFailure();
    Stop();
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
      BackendRxChannel().Transmit(b);
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
  }
void MyBackendAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
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
  }

class PassThroughAuthAppFactory : public PANA_PaaSessionFactory
{
   public:
      PassThroughAuthAppFactory(EapTask &t) :
          PANA_PaaSessionFactory(t.node), task(t) { }
      PANA_PaaSession *Create() {
         BackendAuthApplication *b = new BackendAuthApplication(task); 
         PassThroughAuthApplication *p = new PassThroughAuthApplication
             (*this);
         p->Start(b->RxChannel());
         b->Start(&p->BackendTxChannel());
         return &(p->Pana());
      }
   protected:
      EapTask &task;
};

int main(int argc, char **argv)
{
  std::string cfgfile;
  std::string userdb;
  bool b_client = false;

  // Gather command line options
  ACE_Get_Opt opt(argc, argv, "cf:u:U:", 1);
    
  for (int c; (c = opt()) != (-1); ) {
      switch (c) {
          case 'f': cfgfile.assign(opt.optarg); break;
          case 'c': b_client = true; break;
          case 'u': userdb.assign(opt.optarg); break;    
          case 'U': gUserName.assign(opt.optarg); break;
      }
  }

  if ((opt.argc() < 1) || (cfgfile.length() == 0)) {
    std::cout << "Usage: pana_test3 [-c] -f [configuration file]" << std::endl;
    return (0);
  }
  else if (! b_client && (userdb.length() == 0)) {
    std::cout << "Usage: pana_test [-c] -f [configuration file] -u [user db]" << std::endl;
    return (0);
  }
  
  // Initialize the log.
  EapLogMsg_S::instance()->open("EAP", ACE_Log_Msg::STDERR);
  EapLogMsg_S::instance()->enable_debug_messages();

  // Register the mapping from EapType to the creator of the
  // user-defined method class for each user-defined method
  // implementation.

  EapMethodStateMachineCreator<MyEapAuthIdentityStateMachine> 
    myAuthIdentityCreator;

  EapMethodStateMachineCreator<MyEapPeerArchieStateMachine> 
    myPeerArchieCreator;

  EapMethodStateMachineCreator<MyEapAuthArchieStateMachine> 
    myAuthArchieCreator;

  EapMethodRegistrar methodRegistrar;

  methodRegistrar.registerMethod
    (std::string("Identity"), EapType(1), 
     Authenticator, myAuthIdentityCreator);

  methodRegistrar.registerMethod
    (std::string("Archie"), EapType(ARCHIE_METHOD_TYPE), 
     Peer, myPeerArchieCreator);

  methodRegistrar.registerMethod
    (std::string("Archie"), EapType(ARCHIE_METHOD_TYPE), 
     Authenticator, myAuthArchieCreator);

  EapTask task(cfgfile);
  
  try {
    // Task starts with two threads in the thread pool.
    task.Start(5);
  }
  catch (...) {
    std::cout << "Task failed to start" << std::endl;
    exit(1);
  }

  // Set shared secret.
  char secret[64] = { char(0x70),  char(0x3a),  char(0x97),  char(0x9c),
                      char(0xc5),  char(0xd7),  char(0xb0),  char(0x8d),
                      char(0x37),  char(0x5c),  char(0x10),  char(0xd7),
                      char(0x60),  char(0xfb),  char(0x7f),  char(0x51),
                      char(0x54),  char(0x50),  char(0xec),  char(0x65),
                      char(0x9e),  char(0x14),  char(0xdb),  char(0x35),
                      char(0x41),  char(0xb),  char(0xae),  char(0xd8),
                      char(0x50),  char(0x2b),  char(0x5c),  char(0x3b),
                      char(0x5e),  char(0x6c),  char(0xbf),  char(0x77),
                      char(0xe7),  char(0x1a),  char(0xe0),  char(0x5b),
                      char(0x5),  char(0xe0),  char(0xf7),  char(0xb0),
                      char(0xcb),  char(0x83),  char(0xff),  char(0x24),
                      char(0x6e),  char(0x87),  char(0x49),  char(0xcf),
                      char(0x7f),  char(0xc6),  char(0x1b),  char(0xc2),
                      char(0xcf),  char(0x64),  char(0x13),  char(0xc6),
                      char(0xd5),  char(0xb3),  char(0x5f),  char(0x9) };
  sharedSecret = std::string((char*)secret, sizeof secret);

  try {
     if (b_client) {
         ACE_Semaphore semaphore(0);
         PeerApplication peerApp(task, semaphore);
	 peerApp.Channel().Initialize();
         semaphore.acquire();
         task.Stop();
     }
     else {
         USER_DB_OPEN(userdb);
         PassThroughAuthAppFactory factory(task);
         while (true); // TBD: have a clean exit here
         task.Stop();
         USER_DB_CLOSE();
     }
  }
  catch (PANA_Exception &e) {
      std::cout << "PANA exception: " << e.description() << std::endl;
  }
  catch (...) {
      std::cout << "Unknown exception ... aborting" << std::endl;
  }

  return 0;
}

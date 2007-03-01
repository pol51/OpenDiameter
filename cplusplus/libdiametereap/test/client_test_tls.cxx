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
   client_test.cxx
   Client test program for Diameter EAP Application 
   Written by Yoshihiro Ohba
   Created December 5, 2003.
*/

/*
  ---------------------------------------------------------
  Brief explanation on what is done in this sample program.
  ---------------------------------------------------------

  This program includes the test for Diameter EAP application where
  EAP Identity method is performed either between a peer and a NAS,
  and then EAP MD5-Challenge authenticator method is performed between
  the peer and the backend EAP server via the NAS.

  The peer session entity will prompt you to input a username.  Once
  the username is input, it is carried in an Response/Identity message
  and sent to the Diameter server.

  The NAS will retransmit the Request/Identity message until you input
  the username or the number of retransmission reaches its maximum
  value (the maximum value is set to 3 in this program).  For the
  latter case, the authenticator will stop its state machine and the
  peer will timeout and fail.

  Peer                NAS                         EAP Server
                     (= EAP PassThough          (= Diameter Server)
                        Authenticator +
                        Diameter client)         
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

#include <iostream>
#include <ace/Log_Msg.h>
#include <ace/OS.h>
#include <ace/Atomic_Op_T.h>
#include "diameter_api.h"
#include "diameter_eap_client_session.hxx"
#include "eap.hxx"
#include "eap_peerfsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_identity.hxx"
#include "eap_policy.hxx"
#include "eap_method_registrar.hxx"
#include "eap_fsm.hxx"
#include "eap_log.hxx"
#include "eap_tls.hxx"
#include "eap_tls_fsm.hxx"

// Modified by Santiago Zapata Hernandez for UMU
// New
void print_cadena (ACE_Byte *cad, ACE_INT32 length)
{
    EAP_LOG (LM_DEBUG, "LENGTH cad %d\n", length);
    for (int i = 0; i < length; i++)
    {
	EAP_LOG (LM_DEBUG, "%02x ", (ACE_Byte) (cad[i]));
    }
    EAP_LOG (LM_DEBUG, "\n");
}
// End New

ACE_Atomic_Op<ACE_Thread_Mutex, int> TotalSuccess;

typedef AAA_JobHandle<AAA_GroupedJob> MyJobHandle;

class MyPeerSwitchStateMachine;
class MyPassThroughAuthSwitchStateMachine;

class PeerData;
class PassThroughAuthApplication;

/// Task class used in this sample program.
class EapTask : public AAA_Task
{
 public:
  /// Constructor.
  EapTask() : AAA_Task(AAA_SCHED_FIFO, "EAP") 
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
  virtual void Transmit(AAAMessageBlock *msg)=0;
  virtual void Transmit(AAAMessageBlock *msg, int subChannel)=0;
};


/******** Diameter EAP Client Session ********/

class MyDiameterEapClientSession : public DiameterEapClientSession
{
 public:
  MyDiameterEapClientSession(AAAApplicationCore& appCore, MyJobHandle h) 
    : DiameterEapClientSession(appCore, h)
  {}

  /// This virtual function is called when an EAP client session is
  /// aborted due to enqueue failure of a job or an event inside
  /// Diametger EAP client state machine.
  void Abort();

  /// This virtual function is called when an EAP-Response message is
  /// passed to the EAP passthrough authenticator.
  void SignalContinue(std::string &eapMsg);

  /// Reimplemented from the parent class.
  void SignalSuccess(std::string &eapMsg);

  /// Reimplemented from the parent class.
  void SignalFailure(std::string &eapMsg);

  /// Reimplemented from the parent class.
  void SignalReauthentication();

  /// Reimplemented from the parent class.
  void SignalDisconnect() {}

  /// Reimplemented from the parent class.
  void SetDestinationRealm
  (DiameterScholarAttribute<diameter_utf8string_t> &realm);

  /// Reimplemented from parent class.
  void SetUserName(DiameterScholarAttribute<diameter_utf8string_t> &username);
};

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

// Class definitio for peer EAP-TLS method for my application
class MyEapPeerTlsStateMachine
	: public EapPeerTlsStateMachine
{
	friend class EapMethodStateMachineCreator<MyEapPeerTlsStateMachine>;
public:
	MyEapPeerTlsStateMachine(EapSwitchStateMachine &s)
		: EapPeerTlsStateMachine(s) {}

  /// This pure virtual function is a callback used when an AuthID
  /// needs to be obtained.
  std::string& InputIdentity()
  {
    std::cout << "Received an Tls-Request "<< std::endl;
      
    static std::string identity;
    std::cout << "Input username (within 10sec.): " << std::endl;
    std::cin >> identity;
    std::cout << "username = " << identity << std::endl;
    return identity;
  }
  
protected:
  std::string& InputConfigFile()
  {
    std::cout << "Received an Tls-Request "<< std::endl;
      
//    static std::string configFile;
//    std::cout << "Input config filename (within 10sec.): " << std::endl;
//    std::cin >> configFile;
//    std::cout << "Config file name = " << configFile << std::endl;
static std::string configFile("./config/client.eap-tls.xml");
    return configFile;
  }

private:
	~MyEapPeerTlsStateMachine() {}
};

class MyPeerSwitchStateMachine: public EapPeerSwitchStateMachine
{
 public:

  MyPeerSwitchStateMachine(ACE_Reactor &r, MyJobHandle& h) 
    : EapPeerSwitchStateMachine(r, h)
  {
    AuthPeriod() = 60;
  }

  void Send(AAAMessageBlock *b);

  void Success();

  void Failure();

  void Notification(std::string &str);

  void Abort();

  std::string& InputIdentity();

 private:

  std::string identity;
};

class MyPassThroughAuthSwitchStateMachine
  : public EapPassThroughAuthSwitchStateMachine
{
 public:

  MyPassThroughAuthSwitchStateMachine(ACE_Reactor &r, MyJobHandle& h) 
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
    handle(MyJobHandle
	   (AAA_GroupedJob::Create(task.Job(), this, "peer"))),
    eap(boost::shared_ptr<MyPeerSwitchStateMachine>
	(new MyPeerSwitchStateMachine(*task.reactor(), handle))),
    semaphore(sem),
    rxChannel(PeerChannel(*eap)),
    txChannel(0),
      methodTls(EapContinuedPolicyElement(EapType(TLS_METHOD_TYPE)))
  {
    eap->Policy().InitialPolicyElement(&methodTls);
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
  MyJobHandle handle;
  boost::shared_ptr<MyPeerSwitchStateMachine> eap;
  ACE_Semaphore &semaphore;
  PeerChannel rxChannel;
  Channel  *txChannel;
	EapContinuedPolicyElement methodTls;
};

// My application session (not used in this test program).
class NAS_Application : public AAA_JobData
{

 public:
  NAS_Application(EapTask &task, AAAApplicationCore& appCore,
		  ACE_Semaphore &sem, bool pickup=false)
    : handle(MyJobHandle
	     (AAA_GroupedJob::Create(task.Job(), this, "NAS"))),
      diameter(boost::shared_ptr<MyDiameterEapClientSession>
	       (new MyDiameterEapClientSession(appCore, handle))),
      eap(boost::shared_ptr<MyPassThroughAuthSwitchStateMachine>
	  (new MyPassThroughAuthSwitchStateMachine
	   (*task.reactor(), handle))),
      semaphore(sem),
      rxChannel(PassThroughAuthChannel(*eap)),
      peerTxChannel(0),
      method(EapContinuedPolicyElement(EapType(1)))
  {
    if (pickup)
      eap->Policy().InitialPolicyElement(&method);

    semaphore.acquire();
  }
  ~NAS_Application() {}

  void Start(Channel *c)
  { 
    peerTxChannel = c;
    diameter->Start(); 
    eap->RetransmissionInterval() = 5;

    eap->Start(); 
  }

  Channel* RxChannel() { return &rxChannel; }

  Channel& PeerTxChannel() { return *peerTxChannel; }

  MyPassThroughAuthSwitchStateMachine& Eap() { return *eap; }

  MyDiameterEapClientSession &Diameter() { return *diameter; }

  ACE_Semaphore& Semaphore() { return semaphore; }

 private:
  MyJobHandle handle;
  boost::shared_ptr<MyDiameterEapClientSession> diameter;
  boost::shared_ptr<MyPassThroughAuthSwitchStateMachine> eap;
  ACE_Semaphore &semaphore;
  PassThroughAuthChannel rxChannel;
  Channel *peerTxChannel;
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
    TotalSuccess++;
    std::cout << "Authentication success detected at peer" << std::endl;
    std::cout << "Welcome to the world, " 
	      << PeerIdentity() 
	      << " !!! (" << TotalSuccess.value() << ")" << std::endl;
// Modified by Santiago Zapata Hernandez for UMU
// New
    EAPTLS_session_t_peer * session_peer = ((MyEapPeerTlsStateMachine &) MethodStateMachine ()).get_tls_session ();
    AAAMessageBlock * skey_peer = EAPTLSCrypto_callbacks::eaptls_gen_mppe_keys (session_peer->get_master_key (),
										session_peer->get_client_random (),
										session_peer->get_server_random ());
    EAP_LOG (LM_DEBUG, "Peer TLS session key\n");
    print_cadena ((ACE_Byte *) skey_peer->base (), skey_peer->length ());
// End New
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
    std::cout << "Input username (within 10sec.): " << std::endl;
    std::cin >> identity;
    std::cout << "username = " << identity << std::endl;
    ACE_OS::sleep(2);
    return identity;
  }

void MyPassThroughAuthSwitchStateMachine::Send(AAAMessageBlock *b)
  {
    std::cout << "EAP Request sent from passthrough authenticator" 
	      << std::endl;
    JobData(Type2Type<NAS_Application>()).PeerTxChannel().Transmit(b);
  }
void MyPassThroughAuthSwitchStateMachine::Success(AAAMessageBlock *b)
  {
    std::cout << "EAP Success sent from passthrough authenticator" 
	      << std::endl;
    JobData(Type2Type<NAS_Application>()).PeerTxChannel().Transmit(b);
    Stop();
    JobData(Type2Type<NAS_Application>()).Semaphore().release();
  }
void MyPassThroughAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
    Stop();
    JobData(Type2Type<NAS_Application>()).Semaphore().release();
  }
void MyPassThroughAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
  {
    std::cout << "EAP Failure sent from passthrough authenticator" 
	      << std::endl;
    JobData(Type2Type<NAS_Application>()).PeerTxChannel().Transmit(b);
    Stop();
    JobData(Type2Type<NAS_Application>()).Semaphore().release();
  }
void MyPassThroughAuthSwitchStateMachine::Failure()
  {
    std::cout << "Failure without an EAP Failure" << std::endl;
    Stop();
    JobData(Type2Type<NAS_Application>()).Semaphore().release();
  }
void MyPassThroughAuthSwitchStateMachine::Abort()
  {
    std::cout << "Session aborted for an error in state machine" << std::endl;
    Stop();
    JobData(Type2Type<NAS_Application>()).Diameter().Stop();
    JobData(Type2Type<NAS_Application>()).Semaphore().release();
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

    std::string eapMsg;

    if (b)
      {
	std::cout << "Length = " << (unsigned)b->size() << std::endl;
	eapMsg.assign(b->base(), b->size());
      }

    JobData(Type2Type<NAS_Application>()).Diameter().ForwardResponse(eapMsg);
  }

void 
MyDiameterEapClientSession::Abort()
  {
    std::cout << "Diameter EAP client session aborted." << std::endl;
    DiameterEapClientSession::Stop();
    JobData(Type2Type<NAS_Application>()).Eap().Abort();
  }

void 
MyDiameterEapClientSession::SignalContinue(std::string &eapMsg)
  {
    AAAMessageBlock *msg 
      = AAAMessageBlock::Acquire((char*)eapMsg.data(), (ACE_UINT32)eapMsg.length());
    JobData(Type2Type<NAS_Application>()).Eap().AAA_Continue(msg);
    msg->Release();
  }

void 
MyDiameterEapClientSession::SignalSuccess(std::string &eapMsg)
  {
    AAAMessageBlock *msg = 0;
    if (eapMsg.length() > 0)
      {
	msg = AAAMessageBlock::Acquire((char*)eapMsg.data(), (ACE_UINT32)eapMsg.length());
      }

    JobData(Type2Type<NAS_Application>()).Eap().AAA_Success(msg);
    
    if (msg)
      msg->Release();
  }

void 
MyDiameterEapClientSession::SignalFailure(std::string &eapMsg)
  {
    AAAMessageBlock *msg = 0;
    if (eapMsg.length() > 0)
      msg = AAAMessageBlock::Acquire((char*)eapMsg.data(), (ACE_UINT32)eapMsg.length());
    else
      AAA_LOG((LM_DEBUG, "SignalFailure without EAP-Failure.\n"));

    JobData(Type2Type<NAS_Application>()).Eap().AAA_Failure(msg);

    if (msg)
      msg->Release();
  }

void 
MyDiameterEapClientSession::SignalReauthentication()
  {
    JobData(Type2Type<NAS_Application>()).Eap().Restart();
  }

void
MyDiameterEapClientSession::SetDestinationRealm
(DiameterScholarAttribute<diameter_utf8string_t> &realm)
{
  std::string& userName 
    = JobData(Type2Type<NAS_Application>()).Eap().PeerIdentity();

  size_t pos;

  if ((pos = userName.find('@')) != std::string::npos)
    realm.Set(std::string(userName, ++pos, userName.length() - pos));
  else
    realm.Set(std::string("research2.org"));
}

void
MyDiameterEapClientSession::SetUserName
(DiameterScholarAttribute<diameter_utf8string_t> &username)
{
  std::string& userName 
    = JobData(Type2Type<NAS_Application>()).Eap().PeerIdentity();

  size_t pos;

  if ((pos = userName.find('@')) != std::string::npos)
// Modified by Santiago Zapata Hernandez for UMU
// Old
//    username.Set(std::string(userName.substr(pos)));
// New
    username.Set(std::string(userName, 0, pos));
// End New
  else
    username = userName;
}

class MyInitializer
{
 public:
  MyInitializer(EapTask &t, AAAApplicationCore &appCore) 
    : task(t), applicationCore(appCore)
  {
    Start();
  }

  ~MyInitializer() 
  {
    Stop();
  }

 private:

  void Start()
  {
    InitEapTask();
    InitApplicationCore();
  }

  void Stop()
  {
    task.Stop();
  }

  void InitApplicationCore()
  {
    AAA_LOG((LM_DEBUG, "[%N] Application starting\n"));
    if (applicationCore.Open("config/client.local.xml",
                             task) != AAA_ERR_SUCCESS)
      {
	ACE_ERROR((LM_ERROR, "[%N] Can't open configuraiton file."));
	exit(1);
      }
  }

  void InitEapTask()
  {
    AAA_LOG((LM_DEBUG, "[%N] EAP Task starting.\n"));
    methodRegistrar.registerMethod
      (std::string("Identity"), EapType(1), 
       Authenticator, myAuthIdentityCreator);

    methodRegistrar.registerMethod
      (std::string("TLS"), EapType(TLS_METHOD_TYPE), Peer, 
       myPeerTlsCreator);

    try {
      // Task starts with two threads in the thread pool.
      task.Start(10);
    }
    catch (...) {
      ACE_ERROR((LM_ERROR, "[%N]Task failed to start.\n"));
      exit(1);
    }
  }

  EapTask &task;
  AAAApplicationCore &applicationCore;
  EapMethodRegistrar methodRegistrar;

  EapMethodStateMachineCreator<MyEapPeerTlsStateMachine> 
  myPeerTlsCreator;

  EapMethodStateMachineCreator<MyEapAuthIdentityStateMachine> 
  myAuthIdentityCreator;
};

int
main(int argc, char *argv[])
{
  EapTask task;
  AAAApplicationCore applicationCore;
  MyInitializer initializer(task, applicationCore);

#if defined(WIN32)
  #define num 100
#else
  int num;
  std::cout << "Input number of sessions: ";
  std::cin >> num;
#endif
  ACE_Semaphore semaphore(2*num);

  TotalSuccess=0;

  std::auto_ptr<PeerApplication> peerApp[num];
  std::auto_ptr<NAS_Application> nasApp[num];
  for (int i=0; i<num; i++)
    {
      peerApp[i] 
	= std::auto_ptr<PeerApplication>(new PeerApplication(task, semaphore));
      nasApp[i] 
	= std::auto_ptr<NAS_Application>
	(new NAS_Application(task, applicationCore, semaphore, true));
      peerApp[i]->Start(nasApp[i]->RxChannel());
      nasApp[i]->Start(peerApp[i]->RxChannel());
    }

  // Block until the EAP conversation completes.
  for (int i=0; i<2*num; i++)
    semaphore.acquire();

  std::cout << "Total number of sessions : " << num << std::endl;
  std::cout << "Total number of success : " << TotalSuccess.value() << std::endl;

  for (int i=0; i<num; i++)
    {
      peerApp[i].reset();
      nasApp[i].reset();
    }
  return 0;
}


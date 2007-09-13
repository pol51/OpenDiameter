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

// $Id: Test1.cxx,v 1.35 2006/05/04 19:46:40 vfajardo Exp $
// A test program for EAP API.
// Written by Victor Fajardo

#include <iostream>
#include <ace/OS.h>
#include <ace/Signal.h>
#include <ace/Event_Handler.h>
#include <ace/Get_Opt.h>
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
#include "pana_client_fsm.h"
#include "pana_paa_factory.h"
#include "user_db.h"
#include "pana_auth_script.h"

// #define KEY_TESTING 1

class MyPeerSwitchStateMachine;
class MyStandAloneAuthSwitchStateMachine;
class MyBackendAuthSwitchStateMachine;
class MyPassThroughAuthSwitchStateMachine;

class PeerData;
class StandAloneAuthApplication;
class BackendAuthApplication;
class PassThroughAuthApplication;

static std::string gUserName;
static std::string gPasswd;
static std::string gPacAddrFromEp;
static PANA_PacSession *gPacReference = NULL;
static bool isRunning = false;

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
    std::cout << "Input password: " << std::endl;
    if (gPasswd.length() > 0) {
        Passphrase() = gPasswd;
    }
    else {
        //std::cin >> Passphrase();
        Passphrase() = "12345";
    }
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
    size_t pos;
    std::string userName;
    std::string &identity = AuthSwitchStateMachine().PeerIdentity();
    if ((pos = identity.find('@')) != std::string::npos) {
       userName.assign(identity.substr(0, pos));
    }
    else {
       userName = identity;
    }
    AAA_UserEntry *e = USER_DB_LOOKUP(userName);
    if (e) {
        passphrase.assign(e->m_Passphrase);
    }
  }

private:
  ~MyEapAuthMD5ChallengeStateMachine() {}
};

class MyPeerSwitchStateMachine: public EapPeerSwitchStateMachine
{
 public:

  MyPeerSwitchStateMachine(ACE_Reactor &r, EapJobHandle& h)
      : EapPeerSwitchStateMachine(r, h) {}

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
      EapStandAloneAuthSwitchStateMachine(r, h) {}

  void Send(AAAMessageBlock *b);

  void Success(AAAMessageBlock *b);

  void Success();

  void Failure(AAAMessageBlock *b);

  void Failure();

  void Abort();
};

class PeerApplication : public AAA_JobData,
                        public PANA_ClientEventInterface
{
 public:
  PeerApplication(PANA_Node &n) :
    pacSession(n, *this),
    handle(EapJobHandle(AAA_GroupedJob::Create(n.Task().Job(), this, "peer"))),
    eap(boost::shared_ptr<MyPeerSwitchStateMachine>
	(new MyPeerSwitchStateMachine(*n.Task().reactor(), handle))),
    md5Method(EapContinuedPolicyElement(EapType(4)))
  {
    eap->Policy().InitialPolicyElement(&md5Method);
  }
  virtual ~PeerApplication() { }

  MyPeerSwitchStateMachine& Eap() { return *eap; }

  void EapStart() {
     eap->Stop();
     eap->Start();
  }
  void EapRequest(AAAMessageBlock *request) {
     eap->Receive(request);
  }
  void EapAltReject() {
  }
  void Authorize(PANA_AuthorizationArgs &args) {
     PANA_AuthScriptCtl::Print(args);
  }
  bool IsKeyAvailable(pana_octetstring_t &key) {
#ifdef KEY_TESTING
    static int toggle = 0;
    static char *keys[] = { "0123456789012345678901234567890123456789012345678901234567890123",
                            "3210987654321098765432109876543210987654321098765432109876543210" };
    key.assign(keys[toggle]);
    toggle = (toggle) ? 0 : 1;
    return true;
#else
    return false;
#endif
  }
  void Disconnect(ACE_UINT32 cause) {
  }
  PANA_PacSession &pac() {
      return pacSession;
  }

 private:
  PANA_PacSession pacSession;
  EapJobHandle handle;
  boost::shared_ptr<MyPeerSwitchStateMachine> eap;
  EapContinuedPolicyElement md5Method;
};

// My application session (not used in this test program).
class StandAloneAuthApplication : public AAA_JobData,
                                  public PANA_PaaEventInterface
{

 public:
  StandAloneAuthApplication(PANA_PaaSessionChannel &ch)
    : paaSession(ch, *this),
      handle(EapJobHandle(AAA_GroupedJob::Create
                          (ch.Node().Task().Job(), this, "standalone"))),
      eap(boost::shared_ptr<MyStandAloneAuthSwitchStateMachine>
	  (new MyStandAloneAuthSwitchStateMachine
           (*ch.Node().Task().reactor(), handle))),
      identityMethod(EapContinuedPolicyElement(EapType(1))),
      md5Method(EapContinuedPolicyElement(EapType(4))),
      notificationMethod(EapContinuedPolicyElement(EapType(2)))
  {
    // start paa session
    paaSession.Start();
    // Policy settings for the authenticator
    identityMethod.AddContinuedPolicyElement
      (&md5Method, EapContinuedPolicyElement::PolicyOnSuccess);
    identityMethod.AddContinuedPolicyElement
      (&notificationMethod, EapContinuedPolicyElement::PolicyOnFailure);
    eap->Policy().InitialPolicyElement(&identityMethod);
  }
  virtual ~StandAloneAuthApplication() {
    paaSession.Stop();
  }
  void EapStart() {
     eap->Stop();
     eap->Start();
  }
  void EapResponse(AAAMessageBlock *response) {
     eap->Receive(response);
  }
  void EapAltReject() {
  }
  void Authorize(PANA_AuthorizationArgs &args) {
     PANA_AuthScriptCtl::Print(args);
     paaSession.Ping();
  }
  bool IsKeyAvailable(pana_octetstring_t &key) {
#ifdef KEY_TESTING
    static int toggle = 0;
    static char *keys[] = { "0123456789012345678901234567890123456789012345678901234567890123",
                            "3210987654321098765432109876543210987654321098765432109876543210" };
    key.assign(keys[toggle]);
    toggle = (toggle) ? 0 : 1;
    return true;
#else
    return false;
#endif
  }
  bool IsUserAuthorized() {
      return true;
  }
  void Disconnect(ACE_UINT32 cause) {
     eap->Stop();
  }
  MyStandAloneAuthSwitchStateMachine& Eap() {
     return *eap;
  }
  PANA_PaaSession &paa() { return paaSession; }

 private:
  PANA_PaaSession paaSession;
  EapJobHandle handle;
  boost::shared_ptr<MyStandAloneAuthSwitchStateMachine> eap;
  EapContinuedPolicyElement identityMethod;
  EapContinuedPolicyElement md5Method;
  EapContinuedPolicyElement notificationMethod;
};

class StandAloneSessionFactoryAdapter : public PANA_PaaSessionFactory
{
   public:
      StandAloneSessionFactoryAdapter(PANA_Node &n) :
          PANA_PaaSessionFactory(n), node(n) {
      }
      PANA_PaaSession *Create() {
         StandAloneAuthApplication *app = new StandAloneAuthApplication(*this);
         if (app) {
	    return &(app->paa());
	 }
         return (0);
      }
   protected:
      PANA_Node &node;
};

// ----------------- Definition --------------
void MyPeerSwitchStateMachine::Send(AAAMessageBlock *b)
{
  std::cout << "EAP Response sent from peer" << std::endl;
  //ACE_INET_Addr newAddr("192.168.1.1:0");
  JobData(Type2Type<PeerApplication>()).pac().EapSendResponse(b);
}

void MyPeerSwitchStateMachine::Success()
  {
    std::cout << "Authentication success detected at peer" << std::endl;
    std::cout << "Welcome to the world, "
	      << PeerIdentity()
	      << " !!!" << std::endl;

    // Let PANA bind on success to the EAP event
   JobData(Type2Type<PeerApplication>()).pac().EapSuccess();
  }
void MyPeerSwitchStateMachine::Failure()
  {
    std::cout << "Authentication failure detected at peer" << std::endl;
    std::cout << "Sorry, "
	      << PeerIdentity()
	      << " try next time !!!" << std::endl;
    JobData(Type2Type<PeerApplication>()).pac().EapFailure();
    JobData(Type2Type<PeerApplication>()).Eap().Stop();
    //JobData(Type2Type<PeerApplication>()).Semaphore().release();
  }
void MyPeerSwitchStateMachine::Notification(std::string &str)
  {
    std::cout << "Following notification received" << std::endl;
    std::cout << str << std::endl;
  }
void MyPeerSwitchStateMachine::Abort()
  {
    std::cout << "Peer aborted for an error in state machine" << std::endl;
    JobData(Type2Type<PeerApplication>()).Eap().Stop();
    JobData(Type2Type<PeerApplication>()).pac().EapFailure();
    //JobData(Type2Type<PeerApplication>()).Semaphore().release();
  }
std::string& MyPeerSwitchStateMachine::InputIdentity()
  {
    if (gUserName.length() > 0) {
        return gUserName;
    }
    identity = std::string("user2");
    std::cout << "Input username (within 10sec.): " << std::endl;
    //std::cin >> identity;
    identity = "user1";
    std::cout << "username = " << identity << std::endl;
    return identity;
  }

void MyStandAloneAuthSwitchStateMachine::Send(AAAMessageBlock *b)
  {
    std::cout << "EAP Request sent from authenticator" << std::endl;
    JobData(Type2Type<StandAloneAuthApplication>()).paa().EapSendRequest(b);
  }
void MyStandAloneAuthSwitchStateMachine::Success(AAAMessageBlock *b)
  {
    std::cout << "EAP Success sent from authenticator" << std::endl;
    JobData(Type2Type<StandAloneAuthApplication>()).paa().EapSuccess(b);
  }
void MyStandAloneAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
    JobData(Type2Type<StandAloneAuthApplication>()).Eap().Stop();
    JobData(Type2Type<StandAloneAuthApplication>()).paa().EapSuccess();
  }
void MyStandAloneAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
  {
    std::cout << "EAP Failure sent from authenticator" << std::endl;
    JobData(Type2Type<StandAloneAuthApplication>()).paa().EapFailure(b);
    JobData(Type2Type<StandAloneAuthApplication>()).Eap().Stop();
  }
void MyStandAloneAuthSwitchStateMachine::Failure()
  {
    std::cout << "Failure without an EAP Failure" << std::endl;
    JobData(Type2Type<StandAloneAuthApplication>()).paa().EapFailure();
    JobData(Type2Type<StandAloneAuthApplication>()).Eap().Stop();
    //JobData(Type2Type<StandAloneAuthApplication>()).Semaphore().release();
  }
void MyStandAloneAuthSwitchStateMachine::Abort()
  {
    std::cout << "Session aborted for an error in state machine" << std::endl;
    JobData(Type2Type<StandAloneAuthApplication>()).Eap().Stop();
    JobData(Type2Type<StandAloneAuthApplication>()).paa().EapFailure();
    //JobData(Type2Type<StandAloneAuthApplication>()).Semaphore().release();
  }

#if defined (ACE_HAS_SIG_C_FUNC)
extern "C" {
#endif
static void MySigHandler(int signo)
{
  if (gPacReference) {
      switch (signo) {
         case SIGUSR1: gPacReference->Ping(); break;
         case SIGHUP:  gPacReference->ReAuthenticate(); break;
         case SIGTERM: isRunning = false; break;
         default: break;
      }
  }
  else {
      std::cout << "Signal has been received but reference is not yet ready" << std::endl;
  }
}
#if defined (ACE_HAS_SIG_C_FUNC)
}
#endif

int main(int argc, char **argv)
{
  std::string cfgfile;
  std::string userdb;
  bool b_client = false;

  // Gather command line options
  ACE_Get_Opt opt(argc, argv, "cf:u:U:P:A:", 1);

  for (int c; (c = opt()) != (-1); ) {
      switch (c) {
          case 'f': cfgfile.assign(opt.optarg); break;
          case 'c': b_client = true; break;
          case 'u': userdb.assign(opt.optarg); break;
          case 'U': gUserName.assign(opt.optarg); break;
          case 'P': gPasswd.assign(opt.optarg); break;
          case 'A': gPacAddrFromEp.assign(opt.optarg); break;
      }
  }

  if ((opt.argc() < 1) ||
      (cfgfile.length() == 0)) {
    std::cout << "Usage: pana_test [-c] -f [configuration file]" << std::endl;
    return (0);
  }
  else if (! b_client && (userdb.length() == 0)) {
    std::cout << "Usage: pana_test [-c] -f [configuration file] -u [user db]" << std::endl;
    return (0);
  }

  // Initialize the log.
#ifdef WIN32
  EapLogMsg_S::instance()->open("EAP", ACE_Log_Msg::STDERR);
#else
  EapLogMsg_S::instance()->open("EAP", ACE_Log_Msg::STDERR);
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

  EapTask task;
  task.Start(5);

  ACE_Sig_Action sa(reinterpret_cast <ACE_SignalHandler> (MySigHandler));
  sa.register_action (SIGUSR1);
  sa.register_action (SIGHUP);
  sa.register_action (SIGTERM);

  try {
      if (b_client) {
          PANA_Node node(task, cfgfile);
          PeerApplication peer(node);
          peer.pac().Start();
          gPacReference = &peer.pac();

          // Test code only
          isRunning = true;
          do {
             ACE_Time_Value tm(1);
             ACE_OS::sleep(tm);
          }
          while (isRunning);
          gPacReference->Stop();
      }
      else {
          USER_DB_OPEN(userdb);
          PANA_Node node(task, cfgfile);
          StandAloneSessionFactoryAdapter factory(node);

          if (gPacAddrFromEp.length() > 0) {
	      ACE_INET_Addr pac(gPacAddrFromEp.data());
              factory.PacFound(pac);
          }

          do {
             ACE_Time_Value tm(1);
             ACE_OS::sleep(tm);
          } while (true);

          USER_DB_CLOSE();
      }
      task.Stop();
      std::cout << "Done" << std::endl;
  }
  catch (PANA_Exception &e) {
      std::cout << "PANA exception: " << e.description() << std::endl;
  }
  catch (...) {
      std::cout << "Unknown exception ... aborting" << std::endl;
  }

  return 0;
}


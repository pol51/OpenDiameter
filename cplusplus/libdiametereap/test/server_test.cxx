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
   server_test.cxx
   Server test program for Diameter EAP Application 
   Written by Yoshihiro Ohba
   Created December 23, 2003.
*/

/*
  ---------------------------------------------------------
  Brief explanation on what is done in this sample program.
  ---------------------------------------------------------

  This program includes the test for Diameter EAP application where
  EAP Identity method is performed either between a peer and a NAS
  (Case 1) or between the peer and a backend EAP server via the NAS
  (Case 2), and then EAP MD5-Challenge authenticator method is
  performed between the peer and the backend EAP server via the NAS.

  The peer session entity will prompt you to input a username.  Once
  the username is input, it is carried in an Response/Identity message
  and sent to the Diameter server.

  The NAS will retransmit the Request/Identity message until you input
  the username or the number of retransmission reaches its maximum
  value (the maximum value is set to 3 in this program).  For the
  latter case, the authenticator will stop its state machine and the
  peer will timeout and fail.

Case 1:

  Peer                NAS                         EAP Server
                     (= EAP PassThough          (= Diameter Server)
                        Authenticator +
                        Diameter client)         
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

Case 2:

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
#include "diameter_api.h"
#include "diameter_eap_server_session.hxx"
#include "eap.hxx"
#include "eap_peerfsm.hxx"
#include "eap_authfsm.hxx"
#include "eap_identity.hxx"
#include "eap_policy.hxx"
#include "eap_method_registrar.hxx"
#include "eap_fsm.hxx"
#include "eap_log.hxx"
#include "eap_md5.hxx"

typedef AAA_JobHandle<AAA_GroupedJob> MyJobHandle;

class MyBackendAuthSwitchStateMachine;

/******** Diameter EAP Client Session ********/

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

// Class definition for authenticator MD5-Challenge method for my application.
class MyEapAuthMD5ChallengeStateMachine 
  : public EapAuthMD5ChallengeStateMachine
{
  friend class 
  EapMethodStateMachineCreator<MyEapAuthMD5ChallengeStateMachine>;
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

class MyBackendAuthSwitchStateMachine 
  : public EapBackendAuthSwitchStateMachine
{
 public:

  MyBackendAuthSwitchStateMachine(ACE_Reactor &r, MyJobHandle& h) 
    : EapBackendAuthSwitchStateMachine(r, h)
  {}

  void Send(AAAMessageBlock *b);

  void Success(AAAMessageBlock *b);

  void Success();

  void Failure(AAAMessageBlock *b);

  void Failure();

  void Abort();

 private:
};

class MyDiameterEapServerSession : public DiameterEapServerSession
{
 public:
  MyDiameterEapServerSession(AAAApplicationCore& appCore, 
			     diameter_unsigned32_t appId=EapApplicationId) 
    : DiameterEapServerSession(appCore, appId),
      handle(EapJobHandle
	     (AAA_GroupedJob::Create(appCore.GetTask().Job(), 
				     this, "backend"))),
      eap(boost::shared_ptr<MyBackendAuthSwitchStateMachine>
	  (new MyBackendAuthSwitchStateMachine
	   (*appCore.GetTask().reactor(), handle))),
      identityMethod(EapContinuedPolicyElement(EapType(1))),
      md5Method(EapContinuedPolicyElement(EapType(4))),
      initial(true)
  {
    identityMethod.AddContinuedPolicyElement
      (&md5Method, EapContinuedPolicyElement::PolicyOnSuccess);
      
    eap->Policy().InitialPolicyElement(&identityMethod);

    eap->NeedInitialRequestToSend(false);

    this->Start();
  }

  void Start() throw(AAA_Error)
  { 
    DiameterEapServerSession::Start(); 
  }

  /// This virtual function is called when an EAP server session is
  /// aborted due to enqueue failure of a job or an event inside
  /// Diametger EAP server state machine.
  void Abort()
  {
    std::cout << "Diameter EAP server session aborted." << std::endl;
    DiameterEapServerSession::Stop();
    Eap().Stop();
  }

  MyBackendAuthSwitchStateMachine& Eap() { return *eap; }

  /// This virtual function is called when an EAP-Response message is
  /// passed to the EAP backend authenticator.
  void ForwardEapResponse(std::string &eapMsg);

  bool ValidateUserName(const diameter_utf8string_t &userName)
  {
    if (userName == "ohba")
      return true;
    else
      return false;
  }

 private:
  EapJobHandle handle;
  boost::shared_ptr<MyBackendAuthSwitchStateMachine> eap;
  EapContinuedPolicyElement identityMethod;
  EapContinuedPolicyElement md5Method;
  bool initial;
};

// ----------------- Definition --------------
void MyBackendAuthSwitchStateMachine::Send(AAAMessageBlock *b)
  {
    std::cout << "EAP Request sent from backend authenticator" 
	      << std::endl;
    std::string eapMsg(b->base(), b->length());
    JobData(Type2Type<MyDiameterEapServerSession>()).SignalContinue(eapMsg);
  }
void MyBackendAuthSwitchStateMachine::Success(AAAMessageBlock *b)
  {
    std::cout << "EAP Success sent from backend authenticator" 
	      << std::endl;
    std::string eapMsg(b->base(), b->length());
    JobData(Type2Type<MyDiameterEapServerSession>()).SignalSuccess(eapMsg);
    Stop();
  }
void MyBackendAuthSwitchStateMachine::Success()
  {
    std::cout << "Success without an EAP Success" << std::endl;
    std::string eapMsg("");
    JobData(Type2Type<MyDiameterEapServerSession>()).SignalSuccess(eapMsg);
    Stop();
  }
void MyBackendAuthSwitchStateMachine::Failure(AAAMessageBlock *b)
  {
    std::cout << "EAP Failure sent from backend authenticator" 
	      << std::endl;
    std::string eapMsg(b->base(), b->length());
    JobData(Type2Type<MyDiameterEapServerSession>()).SignalFailure(eapMsg);
    Stop();
  }
void MyBackendAuthSwitchStateMachine::Failure()
  {
    std::cout << "Failure without an EAP Failure" << std::endl;
    std::string eapMsg("");
    JobData(Type2Type<MyDiameterEapServerSession>()).SignalFailure(eapMsg);
    Stop();
  }
void MyBackendAuthSwitchStateMachine::Abort()
  {
    std::cout << "Session aborted for an error in state machine" << std::endl;
    Stop();
  }


void 
MyDiameterEapServerSession::ForwardEapResponse(std::string &eapMsg)
{
  std::cout << "EAP Response forwarded to backend authenticator" 
	    << std::endl;
  AAAMessageBlock *msg;
  if (eapMsg.length() > 0)
    {
      msg = AAAMessageBlock::Acquire((ACE_UINT32)eapMsg.length());
      msg->copy((char*)eapMsg.data(), eapMsg.length());
      if (initial)
	{
	  initial=false;
	  Eap().Start(msg);    // The initial EAP-Response message.
	}
      else
	{
	  Eap().Receive(msg);
	}
      msg->release();
    }
}

typedef AAAServerSessionClassFactory<MyDiameterEapServerSession> 
MyServerFactory;

class MyInitializer
{
 public:
  MyInitializer(AAA_Task &t, AAAApplicationCore &appCore) 
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
    InitTask();
    InitApplicationCore();
    InitEap();
    myAuthFactory = std::auto_ptr<MyServerFactory>
         (new MyServerFactory(applicationCore, EapApplicationId));
    applicationCore.RegisterServerSessionFactory(myAuthFactory.get());
  }

  void Stop() {}

  void InitTask()
  {
     try {
        task.Start(10);
     }
     catch (...) {
        ACE_ERROR((LM_ERROR, "(%P|%t) Server: Cannot start task\n"));
        exit(1);
     }
  }
  void InitApplicationCore()
  {
    AAA_LOG((LM_DEBUG, "[%N] Application starting\n"));
    if (applicationCore.Open("config/server.local.xml",
                             task) != AAA_ERR_SUCCESS)
      {
	ACE_ERROR((LM_ERROR, "[%N] Can't open configuraiton file."));
	exit(1);
      }
  }

  void InitEap()
  {
    AAA_LOG((LM_DEBUG, "[%N] EAP initialization.\n"));
    methodRegistrar.registerMethod
      (std::string("Identity"), EapType(1), 
       Authenticator, myAuthIdentityCreator);

    methodRegistrar.registerMethod
      (std::string("MD5-Challenge"), EapType(4), Authenticator, 
       myAuthMD5ChallengeCreator);
  }

  AAA_Task &task;
  AAAApplicationCore &applicationCore;
  EapMethodRegistrar methodRegistrar;

  EapMethodStateMachineCreator<MyEapAuthMD5ChallengeStateMachine> 
  myAuthMD5ChallengeCreator;

  EapMethodStateMachineCreator<MyEapAuthIdentityStateMachine> 
  myAuthIdentityCreator;

  std::auto_ptr<MyServerFactory> myAuthFactory;
};

int
main(int argc, char *argv[])
{
  AAA_Task task;
  AAAApplicationCore applicationCore;

  MyInitializer initializer(task, applicationCore);

  while (1) 
      ACE_OS::sleep(1);
  return 0;
}


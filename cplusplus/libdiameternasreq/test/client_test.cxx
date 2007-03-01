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
   Client test program for Diameter NASREQ Application 
   Written by Yoshihiro Ohba
   Created May 4, 2004.
*/

/*
  ---------------------------------------------------------
  Brief explanation on what is done in this sample program.
  ---------------------------------------------------------

  This program includes a test for Diameter NASREQ application where
  PAP or CHAP is used for authentication between a client host and a
  NASREQ server via a NAS.  Half of the clients are authenticated with
  PAP and the other half are authenticated with CHAP.

  The client session entity will prompt you to input a username.  Once
  the username is input, it is passed to the NAS.  The NAS generates a
  NASREQ AA-Request and sents to the NASREQ server.

  When PAP is used, it will be the following sequence:

  Peer                NAS                         EAP Server
                     (= NASREQ client)          (= NASREQ Server)
   |    null request        |                          |
   |<-----------------------|                          |
   |                        |                          |
(input username)            |                          |
   |                        |                          |
   |  Username/Password     |                          |
   |----------------------->|  AA_Request/User-Name    |
   |                        |    User-Password         |
   |                        |------------------------->|
   |                        |  AA-Answer/Result-Code=  |
   |                        |    DIAMETER_SUCCESS      |
   |  Success indication    |<-------------------------|
   |<-----------------------|                          |
   |                        |                          |

  When CHAP is used, it will be the following sequence:

  Peer                NAS                         EAP Server
                     (= NASREQ client)          (= NASREQ Server)
   |  Challenge request     |                          |
   |<-----------------------|                          |
   |                        |                          |
(input username)            |                          |
   |                        |                          |
   |  Challenge response/   |                          |
   |    Username            |                          |
   |----------------------->|  AA_Request/User-Name    |
   |                        |    CHAP-Auth,            |
   |                        |    CHAP-Challenge        |
   |                        |------------------------->|
   |                        |  AA-Answer/Result-Code=  |
   |                        |    DIAMETER_SUCCESS      |
   |  Success indication    |<-------------------------|
   |<-----------------------|                          |
   |                        |                          |

 */

#include <iostream>
#include <ace/Log_Msg.h>
#include <ace/OS.h>
#include <ace/Atomic_Op_T.h>
#include "diameter_api.h"
#include "diameter_nasreq_client_session.hxx"

ACE_Atomic_Op<ACE_Thread_Mutex, int> TotalSuccess;

typedef AAA_JobHandle<AAA_GroupedJob> MyJobHandle;

class ClientData;
class NAS_Application;

/// Task class used in this sample program.
class NasreqTask : public AAA_Task
{
 public:
  /// Constructor.
  NasreqTask() : AAA_Task(AAA_SCHED_FIFO, "NASREQ") 
  {}

  /// Destructor.
  ~NasreqTask() {}
};

/// This class defines a transport.  When a message is sent
/// to a particular entity (e.g., an entity may be a client, a NAS or
/// a NASREQ server depending on the role of the sender, Transmit()
/// method of the Channel object of the receiving entity is called.
/// Transmit() method can have sub-channels which is used for
/// distinguishing different types of messages.
class Channel 
{
 public:
  Channel() {}
  virtual ~Channel() {}
  virtual void Transmit(DiameterNasreqAuthenticationInfo &authInfo)=0;
};


/******** Diameter NASREQ Client Session ********/

class MyDiameterNasreqClientSession : public DiameterNasreqClientSession
{
 public:
  MyDiameterNasreqClientSession(AAAApplicationCore& appCore, MyJobHandle h) 
    : DiameterNasreqClientSession(appCore, h)
  {}

  /// This virtual function is called when a NASREQ client session is
  /// aborted due to enqueue failure of a job or an event inside
  /// Diametger NASREQ client state machine.
  void Abort();

  /// This virtual function is called when a NASREQ Authentication-Info
  /// is passed to the NAS.
  void SignalContinue(DiameterNasreqAuthenticationInfo&);

  /// Reimplemented from the parent class.
  void SignalSuccess();

  /// Reimplemented from the parent class.
  void SignalFailure();

  /// Reimplemented from the parent class.
  void SignalReauthentication();

  /// Reimplemented from the parent class.
  void SignalDisconnect() {}

  /// Reimplemented from the parent class.
  void SetDestinationRealm
  (DiameterScholarAttribute<diameter_utf8string_t> &realm);

  /// Reimplemented from parent class.
  void SetAuthInfo(DiameterNasreqAuthenticationInfo &authInfo);
};

/// Class for end-host client.
class ClientSession : AAA_Job
{
 public:

  ClientSession(MyJobHandle &handle) : handle(handle) {}

  int Serve() { return 0; }
  
  int Schedule(AAA_Job*, size_t) { return 0; }

  void Receive(DiameterNasreqAuthenticationInfo &authInfo)
  {

    if (authInfo.AuthenticationType() == 
	NASREQ_AUTHENTICATION_TYPE_PAP)
      // PAP
      {
	PAP_Info& papInfo = (PAP_Info&)authInfo;
	papInfo.UserName() = username;
	papInfo.UserPassword() = password;
	Send(papInfo);
      }
    else if (authInfo.AuthenticationType() == 
	     NASREQ_AUTHENTICATION_TYPE_CHAP)
      // CHAP
      {
	CHAP_Info& chapInfo = (CHAP_Info&)authInfo;
	
	chapInfo.UserName() = username;
	// Initialize the result.
	std::string md5Result(MD5_DIGEST_LENGTH, '\0');

	// Do MD5.
	std::string rawResponse(chapInfo.ChapAuth().ChapIdent());
	rawResponse.append(password);
	rawResponse.append((std::string&)chapInfo.ChapChallenge());
	MD5((const unsigned char*)rawResponse.data(), 
	    (unsigned)rawResponse.size(), (unsigned char*)md5Result.data());
	chapInfo.ChapAuth().ChapResponse = md5Result;
	Send(chapInfo);
      }
    else
      {
	AAA_LOG((LM_ERROR, "invalid authenticaiton type.\n"));
	Send(authInfo);
      }
  }

  void Send(DiameterNasreqAuthenticationInfo &authInfo);

  void Success();

  void Failure();

  void SetCredentials(std::string uname, std::string pword)
  {
    username = uname;
    password = pword;
  }

 private:

  std::string username;
  std::string password;
  MyJobHandle& handle;
};

class ClientChannel : public Channel
{
 public:
  ClientChannel(ClientSession &s) : session(s) {}
  void Transmit(DiameterNasreqAuthenticationInfo &authInfo) 
  { session.Receive(authInfo); }
  ClientSession &session;
};

class NAS_Channel : public Channel
{
 public:
  NAS_Channel(MyDiameterNasreqClientSession& s) : session(s) {}

  void Transmit(DiameterNasreqAuthenticationInfo &authInfo) 
  { session.ForwardAuthenticationInfo(authInfo); }

  MyDiameterNasreqClientSession &session;
};

class ClientApplication : public AAA_JobData
{
 public:
  ClientApplication(NasreqTask &task, ACE_Semaphore &sem) : 
    handle(MyJobHandle
	   (AAA_GroupedJob::Create(task.Job(), this, "client"))),
    session(boost::shared_ptr<ClientSession>(new ClientSession(handle))),
    rxChannel(ClientChannel(*session)),
    txChannel(0)
  {
    session->SetCredentials("opendiameter@research2.org", "abcdef12345678");
  }
  ~ClientApplication() 
  {}
  void Start(Channel *c)
  { 
    txChannel = c;
  }

  ClientChannel* RxChannel() { return &rxChannel; }

  Channel& TxChannel() { return *txChannel; }

 private:
  MyJobHandle handle;
  boost::shared_ptr<ClientSession> session;
  ClientChannel rxChannel;
  Channel  *txChannel;
};

// My application session (not used in this test program).
class NAS_Application : public AAA_JobData
{

 public:
  NAS_Application(NasreqTask &task, AAAApplicationCore& appCore,
		  ACE_Semaphore &sem)
    : handle(MyJobHandle
	     (AAA_GroupedJob::Create(appCore.GetTask().Job(), this, "NAS"))),
      diameter(boost::shared_ptr<MyDiameterNasreqClientSession>
	       (new MyDiameterNasreqClientSession(appCore, handle))),
      semaphore(sem),
      rxChannel(NAS_Channel(*diameter)),
      txChannel(0)
  {
    semaphore.acquire();
  }
  ~NAS_Application() {}

  /// if algorithm is 0 use PAP, otherwise use CHAP.
  void Start(Channel *c, int algorithm)
  { 
    txChannel = c;
    diameter->Start(); 
    if (algorithm == 0)
      {
	std::cout << "Start PAP." << std::endl;
	PAP_Info papInfo;
	c->Transmit(papInfo);
      }
    else
      {
	std::cout << "Start CHAP." << std::endl;
	CHAP_Info chapInfo;
	chapInfo.ChapAuth().ChapAlgorithm = CHAP_ALGORITHM_MD5;
	chapInfo.ChapAuth().ChapIdent = std::string(1, '1'-'0');
	chapInfo.ChapChallenge() = std::string("dkjf;akjkljrk;le");
	c->Transmit(chapInfo);
      }
  }

  Channel* RxChannel() { return &rxChannel; }

  Channel& TxChannel() { return *txChannel; }

  MyDiameterNasreqClientSession &Diameter() { return *diameter; }

  ACE_Semaphore& Semaphore() { return semaphore; }

 private:
  MyJobHandle handle;
  boost::shared_ptr<MyDiameterNasreqClientSession> diameter;
  ACE_Semaphore &semaphore;
  NAS_Channel rxChannel;
  Channel *txChannel;
  std::string username;
};

// ----------------- Definition --------------
void 
MyDiameterNasreqClientSession::Abort()
  {
    std::cout << "Diameter NASREQ client session aborted." << std::endl;
    DiameterNasreqClientSession::Stop();
    JobData(Type2Type<NAS_Application>()).Semaphore().release();
  }

void 
MyDiameterNasreqClientSession::SignalContinue
(DiameterNasreqAuthenticationInfo& authInfo)
{
  JobData(Type2Type<NAS_Application>()).TxChannel().Transmit(authInfo);
}

void 
MyDiameterNasreqClientSession::SignalSuccess()
  {
    AAA_LOG((LM_DEBUG, "Client authentication successful.\n"));
    TotalSuccess++;
    Stop();
    JobData(Type2Type<NAS_Application>()).Semaphore().release();
  }

void 
MyDiameterNasreqClientSession::SignalFailure()
  {
    AAA_LOG((LM_DEBUG, "Client authentication failed.\n"));
    Abort();
  }

void 
MyDiameterNasreqClientSession::SignalReauthentication()
  {
    AAA_LOG((LM_DEBUG, "Client Re-authentication triggerred (to be implemented).\n"));
    Abort();
  }

void
MyDiameterNasreqClientSession::SetDestinationRealm
(DiameterScholarAttribute<diameter_utf8string_t> &realm)
{
  std::string& userName = AuthenticationInfo().UserName();

  size_t pos = userName.find('@');

  if (pos != std::string::npos) {
    pos ++;
    realm.Set(std::string(userName, pos, userName.length() - pos));
  }
  else {
    realm.Set(std::string("research.org"));
  }
}

void
ClientSession::Send(DiameterNasreqAuthenticationInfo &authInfo)
{
  handle.Job().Data(Type2Type<ClientApplication>())->TxChannel()
    .Transmit(authInfo);
}

class MyInitializer
{
 public:
  MyInitializer(NasreqTask &t, AAAApplicationCore &appCore) 
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
  }

  void Stop()
  {
    task.Stop();
  }

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
    if (applicationCore.Open("config/client.local.xml",
                             task) != AAA_ERR_SUCCESS)
      {
	ACE_ERROR((LM_ERROR, "[%N] Can't open configuraiton file."));
	exit(1);
      }
  }

  NasreqTask &task;

  AAAApplicationCore &applicationCore;
};

int
main(int argc, char *argv[])
{
  NasreqTask task;
  AAAApplicationCore applicationCore;
  MyInitializer initializer(task, applicationCore);

#if defined(WIN32)
  #define num 100
  char c;
  std::cout << "Input any characer and return after peer connection has established: ";
  std::cin >> c;
#else
  int num;
  std::cout << "Input number of sessions and return after peer connection has established: ";
  std::cin >> num;
#endif

  ACE_Semaphore semaphore(num);

  TotalSuccess=0;

  std::auto_ptr<ClientApplication> clientApp[num];
  std::auto_ptr<NAS_Application> nasApp[num];
  for (int i=0; i<num; i++)
    {
      clientApp[i] 
	= std::auto_ptr<ClientApplication>
	(new ClientApplication(task, semaphore));
      nasApp[i] 
	= std::auto_ptr<NAS_Application>
	(new NAS_Application(task, applicationCore, semaphore));
      clientApp[i]->Start(nasApp[i]->RxChannel());
      nasApp[i]->Start(clientApp[i]->RxChannel(), i % 2);
    }

  // Block until the NAS conversation completes.
  for (int i=0; i<num; i++)
    semaphore.acquire();

  task.Stop();
  std::cout << "Total number of sessions : " << num << std::endl;
  std::cout << "Total number of success : " << TotalSuccess.value() << std::endl;
  return 0;
}


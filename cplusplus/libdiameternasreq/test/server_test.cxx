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

/* $Id: */
/* 
   server_test.cxx
   Server test program for Diameter NASREQ Application 
   Written by Yoshihiro Ohba
   Created June 2, 2004.
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
#include "diameter_api.h"
#include "diameter_nasreq_parser.hxx"
#include "diameter_nasreq_server_session.hxx"
#include "diameter_nasreq_authinfo.hxx"

typedef AAA_JobHandle<AAA_GroupedJob> MyJobHandle;

/******** Diameter NASREQ Server Session ********/

class BackendSession : AAA_Job
{
 public:

  BackendSession(MyJobHandle &handle) : handle(handle) {}

  void Send(DiameterNasreqAuthenticationInfo &authInfo);

  void Receive(DiameterNasreqAuthenticationInfo &authInfo)
  {
    bool authSuccess;
    AAA_LOG(LM_DEBUG, "Username = %s.\n", authInfo.UserName().c_str());
    diameter_utf8string_t password("abcdef12345678");
    if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_PAP)
      authSuccess = ((PAP_Info&)authInfo).Validate(password);
    else if (authInfo.AuthenticationType() == NASREQ_AUTHENTICATION_TYPE_CHAP)
      authSuccess = ((CHAP_Info&)authInfo).Validate(password);
    else
      authSuccess = false;
    if (authSuccess)
      Success();
    else
      Failure();
  }

  int Serve() { return 0; }
  
  int Schedule(AAA_Job*, size_t) { return 0; }

  void Success();

  void Failure();

 private:
  std::string username;
  std::string password;
  MyJobHandle& handle;
};

class MyDiameterNasreqServerSession : public DiameterNasreqServerSession
{
 public:
  MyDiameterNasreqServerSession(AAAApplicationCore& appCore,
				diameter_unsigned32_t appId=NasreqApplicationId) 
    : DiameterNasreqServerSession(appCore, appId),
      handle(MyJobHandle
	     (AAA_GroupedJob::Create(appCore.GetTask().Job(), 
				     this, "backend"))),
      session(boost::shared_ptr<BackendSession>(new BackendSession(handle)))
  {
    this->Start();
  }

  void Start() throw(AAA_Error)
  { 
    DiameterNasreqServerSession::Start(); 
  }

  /// This virtual function is called when a NASREQ server session is
  /// aborted due to enqueue failure of a job or an event inside
  /// Diameter NASREQ server state machine.
  void Abort()
  {
    std::cout << "Diameter NASREQ server session aborted." << std::endl;
    DiameterNasreqServerSession::Stop();
  }

  BackendSession& Session() { return *session; }

  /// This virtual function is called when an authentication information is
  /// passed to the backend.
  void ForwardAuthenticationInfo(DiameterNasreqAuthenticationInfo &authInfo);

 private:
  MyJobHandle handle;
  boost::shared_ptr<BackendSession> session;
};

// ----------------- Definition --------------
void BackendSession::Send(DiameterNasreqAuthenticationInfo &authInfo)
  {
    std::cout << "A request sent from backend" << std::endl;
    handle.Job().Data(Type2Type<MyDiameterNasreqServerSession>())->
      SignalContinue(authInfo);
  }
void BackendSession::Success()
  {
    std::cout << "Success sent from backend" << std::endl;
    handle.Job().Data(Type2Type<MyDiameterNasreqServerSession>())->
      SignalSuccess();
  }
void BackendSession::Failure()
  {
    std::cout << "Failure" << std::endl;
    handle.Job().Data(Type2Type<MyDiameterNasreqServerSession>())->
      SignalFailure();
  }

void 
MyDiameterNasreqServerSession::ForwardAuthenticationInfo
(DiameterNasreqAuthenticationInfo &authInfo)
{
  std::cout << "Auth-Info forwarded to backend" << std::endl;
  Session().Receive(authInfo);
}

typedef AAAServerSessionClassFactory<MyDiameterNasreqServerSession> 
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
    myAuthFactory = std::auto_ptr<MyServerFactory>
         (new MyServerFactory(applicationCore, NasreqApplicationId));
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
    AAA_LOG(LM_DEBUG, "[%N] Application starting\n");
    if (applicationCore.Open("config/server.local.xml",
                             task) != AAA_ERR_SUCCESS)
      {
	AAA_LOG(LM_ERROR, "[%N] Can't open configuraiton file.");
	exit(1);
      }
  }

  AAA_Task &task;
  AAAApplicationCore &applicationCore;
  std::auto_ptr<MyServerFactory> myAuthFactory;
};

int
main(int argc, char *argv[])
{
  AAA_Task myTask;
  AAAApplicationCore applicationCore;
  MyInitializer initializer(myTask, applicationCore);

  while (1) 
      ACE_OS::sleep(1);
  return 0;
}


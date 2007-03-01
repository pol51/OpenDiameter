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

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <ace/Get_Opt.h>
#include "eap_peerfsm.hxx"
#include "eap_identity.hxx"
#include "eap_md5.hxx"
#include "eap_archie_fsm.hxx"
#include "pana_client_fsm.h"
#include "pacd_config.h"
#include "pana_auth_script.h"

#define USAGE "Usage: pacd -f [configuration file]"

static std::string g_SharedSecret;

typedef AAA_JobHandle<AAA_GroupedJob> AppJobHandle;

class PeerData;
class AppPeerSwitchStateMachine;

class EapTask : public AAA_Task
{
   public:
      EapTask(std::string &cfgfile) :
            AAA_Task(AAA_SCHED_WFQ, "EAP"),
            m_Node(*this, cfgfile) {
      }
      virtual ~EapTask() {}

      PANA_Node m_Node;
};

// Definition for peer MD5-Challenge method.
class AppEapPeerMD5ChallengeStateMachine 
      : public EapPeerMD5ChallengeStateMachine
{
       friend class EapMethodStateMachineCreator
                    <AppEapPeerMD5ChallengeStateMachine>;
   public:
       AppEapPeerMD5ChallengeStateMachine(EapSwitchStateMachine &s)
           : EapPeerMD5ChallengeStateMachine(s) {} 

       void InputPassphrase() {
          std::cout << "Setting password" << std::endl;
          Passphrase() = PACD_CONFIG().m_Password;
       }

   private:
       ~AppEapPeerMD5ChallengeStateMachine() {} 
};

// Definition for peer archie state machine.
class AppEapPeerArchieStateMachine 
      : public EapPeerArchieStateMachine
{
       friend class EapMethodStateMachineCreator
                     <AppEapPeerArchieStateMachine>;
   public:
       AppEapPeerArchieStateMachine(EapSwitchStateMachine &s)
           : EapPeerArchieStateMachine(s) {}

       std::string& InputSharedSecret() {
          // Invoked on shared secret callback
          return g_SharedSecret;
       }

       std::string& InputIdentity() {
          // Invoked on Auth ID callback
          std::cout << "Received an Archie-Request from " 
	            << AuthID() << std::endl;
          return SwitchStateMachine().PeerIdentity();
       }

   private:
       ~AppEapPeerArchieStateMachine() {} 
};

// EAP peer state machine
class AppPeerSwitchStateMachine
      : public EapPeerSwitchStateMachine
{
   public:
       AppPeerSwitchStateMachine(ACE_Reactor &r,
                                 AppJobHandle& h,
                                 int type) 
	 : EapPeerSwitchStateMachine(r, h) { }

       void Send(AAAMessageBlock *b);

       void Success();

       void Failure();

       void Notification(std::string &str);

       void Abort();

       std::string& InputIdentity();
};

// EAP channel definition, bounded to PANA
class PeerChannel : public PANA_ClientEventInterface,
                    public ACE_Event_Handler
{
   public:
       PeerChannel(PANA_Node &n,
                   AppPeerSwitchStateMachine &s,
                   ACE_Semaphore &sem) :
          m_Eap(s), 
          m_PaC(n, *this), 
          m_Semaphore(sem),
          m_TimerHandle(0),
          m_Task(n.Task()),
          m_AuthScriptCtl(PACD_CONFIG().m_AuthScript) {
          m_PaC.Start();
       }
       virtual ~PeerChannel() {
       }
       virtual bool ResumeSession() {
          return false;
       }
       void SendEapResponse(AAAMessageBlock *msg) {
          m_PaC.EapSendResponse(msg);
       }
       void EapStart() {
          m_Eap.Stop();
          m_Eap.Start();
       }
       void EapRequest(AAAMessageBlock *request) {
          m_Eap.Receive(request);
       }
       void Success() {
          m_PaC.EapSuccess();
       }
       void Failure() {
          m_PaC.EapFailure();
       }
       void EapAltReject() {
       }
       void Authorize(PANA_AuthorizationArgs &args) {
          // Seed the auth-script
          m_AuthScriptCtl.Seed(args);
          m_AuthScriptCtl.Add();

          // Start auth period
          ACE_Time_Value delay(PACD_CONFIG().m_AuthPeriod, 0);
          m_TimerHandle = m_Task.ScheduleTimer(this, 0, delay);
       }
       virtual bool IsKeyAvailable(pana_octetstring_t &key) {
          if (m_Eap.KeyAvailable()) {
              std::cout << "Assigning key" << std::endl;
              key.assign(m_Eap.KeyData().data(), m_Eap.KeyData().size());
              return true;
          }
          return false;
       }
       void Disconnect(ACE_UINT32 cause) {
          std::cout << "PANA Disconnection" << std::endl;
          m_AuthScriptCtl.Remove();
          m_Eap.Stop();
          m_PaC.Abort();
          m_Task.CancelTimer(m_TimerHandle, 0);
          m_Semaphore.release();
       }
       void Error(ACE_UINT32 resultCode) { 
          std::cout << "PANA Error: " << resultCode << std::endl;
       }
       void Stop() {
          m_PaC.Stop();
       }
       void Abort() {
          m_PaC.Abort();
       }
   private:
       int handle_timeout(const ACE_Time_Value &tv, 
                          const void *arg) {
          std::cout << "Authorization period expired !!!"
                    << std::endl;
          Stop();
          return (0);
       }

       AppPeerSwitchStateMachine &m_Eap;
       PANA_PacSession m_PaC;
       ACE_Semaphore &m_Semaphore;
       ACE_UINT32 m_TimerHandle;
       AAA_Task &m_Task;
       PANA_AuthScriptCtl m_AuthScriptCtl;
};

// Peer Application
class PeerApplication : public AAA_JobData
{
   public:
       PeerApplication(EapTask &task, 
                       ACE_Semaphore &sem, 
                       int type) : 
          m_Handle(AppJobHandle
	         (AAA_GroupedJob::Create
                       (task.Job(), this, "peer"))),
          m_Eap(boost::shared_ptr<AppPeerSwitchStateMachine>
	         (new AppPeerSwitchStateMachine
                       (*task.reactor(), m_Handle, type))),
          m_Semaphore(sem),
          m_Channel(task.m_Node, *m_Eap, sem),
          m_Md5Method(EapContinuedPolicyElement(EapType(4))),
          m_ArchieMethod(EapContinuedPolicyElement
                       (EapType(ARCHIE_METHOD_TYPE))) {
          if (type == ARCHIE_METHOD_TYPE) {
              m_Eap->Policy().InitialPolicyElement(&m_ArchieMethod);
          }
          else {
              m_Eap->Policy().InitialPolicyElement(&m_Md5Method);
          }
       }
       PeerChannel& Channel() { 
          return m_Channel; 
       }
       AppPeerSwitchStateMachine& Eap() { 
          return *m_Eap; 
       }
       ACE_Semaphore& Semaphore() { 
          return m_Semaphore; 
       }

   private:
       AppJobHandle m_Handle;
       boost::shared_ptr<AppPeerSwitchStateMachine> m_Eap;
       ACE_Semaphore &m_Semaphore;
       PeerChannel m_Channel;
       EapContinuedPolicyElement m_Md5Method;
       EapContinuedPolicyElement m_ArchieMethod;
};

void AppPeerSwitchStateMachine::Send(AAAMessageBlock *b) 
{
   JobData(Type2Type<PeerApplication>()).Channel().
           SendEapResponse(b);
}

void AppPeerSwitchStateMachine::Success() 
{
   std::cout << "Authentication success at peer" 
             << std::endl;
   std::cout << "Welcome to the world, " 
             << PeerIdentity() 
	     << " !!!" << std::endl;
   JobData(Type2Type<PeerApplication>()).Channel().Success();
}

void AppPeerSwitchStateMachine::Failure() 
{
   std::cout << "Authentication failure detected at peer" 
             << std::endl;
   std::cout << "Sorry, " << PeerIdentity() 
             << " try next time !!!" << std::endl;
   JobData(Type2Type<PeerApplication>()).Channel().Failure();
   Stop();
   JobData(Type2Type<PeerApplication>()).Semaphore().release();
}

void AppPeerSwitchStateMachine::Notification(std::string &str) 
{
   std::cout << "Following notification received" 
             << std::endl;
   std::cout << str << std::endl;
}

void AppPeerSwitchStateMachine::Abort() 
{
   std::cout << "Peer aborted for an error in state machine" 
             << std::endl;
   JobData(Type2Type<PeerApplication>()).Channel().Abort();
   JobData(Type2Type<PeerApplication>()).Semaphore().release();
}

std::string& AppPeerSwitchStateMachine::InputIdentity() 
{
   std::cout << "Setting username: " 
             << PACD_CONFIG().m_Username
             << std::endl;
   return PACD_CONFIG().m_Username;
}

class PeerInitializer
{
   public:
       PeerInitializer(EapTask &t) : 
          m_Task(t) {
          Start();
       }
       ~PeerInitializer() {
          Stop();
       }

   private:
       void Start() {
          m_MethodRegistrar.registerMethod
             (std::string("MD5-Challenge"), EapType(4),
              Peer, m_AppPeerMD5ChallengeCreator);

          m_MethodRegistrar.registerMethod
             (std::string("Archie"), EapType(ARCHIE_METHOD_TYPE), 
              Peer, m_AppPeerArchieCreator);

          try {
             m_Task.Start(PACD_CONFIG().m_ThreadCount);
          }
          catch (...) {
	     std::cout << "Task failed to start !!\n" 
                       << std::endl;
             exit(1);
          }
       }
       void Stop() {
          m_Task.Stop();
       }

       EapTask &m_Task;
       EapMethodRegistrar m_MethodRegistrar;
       EapMethodStateMachineCreator
            <AppEapPeerMD5ChallengeStateMachine> 
             m_AppPeerMD5ChallengeCreator;    
       EapMethodStateMachineCreator
            <AppEapPeerArchieStateMachine> 
             m_AppPeerArchieCreator;
};

int main(int argc, char *argv[])
{
  std::string panaCfgfile;
  
  // verify command line options
  ACE_Get_Opt opt(argc, argv, "f:", 1);    
  for (int c; (c = opt()) != (-1); ) {
      switch (c) {
          case 'f': panaCfgfile.assign(opt.optarg); break;
      }
  }
  if ((opt.argc() < 1) || (panaCfgfile.length() == 0)) {
      std::cout << USAGE << std::endl;
      return (0);
  }

  // Load configuration file
  PACD_CONFIG_OPEN(panaCfgfile);

  // Get shared secret.
  fstream secret(PACD_CONFIG().m_Secret.data(), 
                 ios::in | ios::binary);
  if (secret) {
     unsigned char buffer[64];
     if (! secret.eof()) {
        memset(buffer, 0, sizeof(buffer));
        secret.read((char*)buffer, sizeof(buffer));
        g_SharedSecret.assign((char*)buffer, sizeof(buffer));
     }
     secret.close();
  }
  else {
     std::cout << "Cannot open file: " << PACD_CONFIG().m_Secret
               << std::endl;
     return (-1);
  }
  
  EapTask task(PACD_CONFIG().m_PaCCfgFile);
  PeerInitializer init(task);
  ACE_Semaphore semaphore(0);
  PeerApplication peer(task,
                       semaphore,
                       PACD_CONFIG().m_UseArchie ? 
                       ARCHIE_METHOD_TYPE : 4);
  semaphore.acquire();
  task.Stop();
  return 0;
}


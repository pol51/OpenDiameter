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

#pragma unmanaged

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <ace/OS.h>
#include <ace/Get_Opt.h>
#include <ace/Log_Msg.h>
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
#include "PanaClient.h"

typedef AAA_JobHandle<AAA_GroupedJob> AppJobHandle;

class AppPeerSwitchStateMachine;

// Class definition for peer MD5-Challenge method for my application.
class AppEapPeerMD5ChallengeStateMachine : public EapPeerMD5ChallengeStateMachine
{
       friend class EapMethodStateMachineCreator<AppEapPeerMD5ChallengeStateMachine>;
   public:
       AppEapPeerMD5ChallengeStateMachine(EapSwitchStateMachine &s)
            : EapPeerMD5ChallengeStateMachine(s) {} 

       // Reimplemented from EapPeerMD5ChallengeStateMachine.
       void InputPassphrase() {
           std::string &passphrase = Passphrase();
           passphrase = PANA_CLIENT->Arg().m_Password;
       }
   private:
       ~AppEapPeerMD5ChallengeStateMachine() {} 
};

// Class definition for peer archie state machine.
class AppEapPeerArchieStateMachine : public EapPeerArchieStateMachine
{
    friend class EapMethodStateMachineCreator<AppEapPeerArchieStateMachine>;
public:
    AppEapPeerArchieStateMachine(EapSwitchStateMachine &s)
        : EapPeerArchieStateMachine(s) {}

    /// This pure virtual function is a callback used when a shared-secret 
    /// needs to be obtained.
    std::string& InputSharedSecret() {
        return PANA_CLIENT->Arg().m_SharedSecret;
    }

    /// This pure virtual function is a callback used when an AuthID
    /// needs to be obtained.
    std::string& InputIdentity() {
        std::string log("Received an Archie-Request from ");
        log += AuthID();
        return SwitchStateMachine().PeerIdentity();
    }
private:
    ~AppEapPeerArchieStateMachine() {} 
};

class AppPeerSwitchStateMachine: public EapPeerSwitchStateMachine
{
   public:
        AppPeerSwitchStateMachine(ACE_Reactor &r,
			AppJobHandle& h) : EapPeerSwitchStateMachine(r, h) { }

        void Send(AAAMessageBlock *b);

        void Success();

        void Failure();

        void Notification(std::string &str);

        void Abort();

        std::string& InputIdentity();
};

class PeerChannel : public PANA_ClientEventInterface
{
   public:
        PeerChannel(PANA_Node &n,
                    AppPeerSwitchStateMachine &s) :
            m_Eap(s), 
            m_Pana(n, *this), 
            m_AuthScriptCtl(PANA_CLIENT->Arg().m_AuthScript) {
            m_Pana.EnableDhcpBootstrap() = true;
            m_Pana.Start(); // discovery
        }
        virtual ~PeerChannel() {
        }
        void EapStart(bool &nap) {
            m_Eap.Stop();
            m_Eap.Start();
        }
        void EapResponse(AAAMessageBlock *msg) {
            m_Pana.EapSendResponse(msg);
        }
        void ChooseISP(const PANA_CfgProviderList &list,
                       PANA_CfgProviderInfo *&choice) {
            choice = PANA_CLIENT->Event().IspSelection(list);
        }
        void EapRequest(AAAMessageBlock *request,
                        bool nap) {
            m_Eap.Receive(request);
        }
        void EapSuccess() {
            m_Pana.EapSuccess();
        }
        void EapFailure() {
            m_Pana.EapFailure();
        }
	void EapAltReject(void) {
	}
	void Notification(diameter_octetstring_t &) {
	}
        void Notification(diameter_octetstring_t &msg, 
                          PANA_DeviceId &pacId) {
        }
#if defined(PANA_MPA_SUPPORT)
        void PacIpAddress(PANA_DeviceId &ip,
                          PANA_DeviceId &oldip,
                          PANA_DeviceId &remoteip) {
           char display[64];
           ACE_INET_Addr addr;
           PANA_DeviceIdConverter::FormatToAddr(ip, addr);
        }
#endif
        void Authorize(PANA_AuthorizationArgs &args) {
            // Seed the auth-script
            m_AuthScriptCtl.Seed(args);
            m_AuthScriptCtl.Add();
            PANA_CLIENT->Event().Success(m_Pana.CurrentInterface());
        }
        void EapReAuthenticate() {
            m_Pana.EapReAuthenticate();
        }
        bool IsKeyAvailable(diameter_octetstring_t &key) {
            if (m_Eap.KeyAvailable()) {
                key.assign(m_Eap.KeyData().data(), 
                           m_Eap.KeyData().size());
                return true;
            }
            return false;
        }
        bool ResumeSession() {
            return false;
        }
        void UpdateAddress(ACE_INET_Addr &newAddr,
                           std::string &message) {
            m_Pana.UpdatePostPanaAddress(newAddr,
                                         message);
        }
        void SendPing() {
            m_Pana.Ping();
        }
        void SendNotification(std::string &message) {
            m_Pana.SendNotification(message);
        }
        void Disconnect(ACE_UINT32 cause) {
            m_AuthScriptCtl.Remove();
            m_Eap.Stop();
            if (cause != PANA_TERMCAUSE_LOGOUT) {
                PANA_CLIENT->Event().Disconnect();
            }
        }
        void Error(ACE_UINT32 resultCode) { 
        }
        void Stop() {
            m_Pana.Stop();
        }
   private:
        AppPeerSwitchStateMachine &m_Eap;
        PANA_PacSession m_Pana;
        PANA_AuthScriptCtl m_AuthScriptCtl;
};

class PeerApplication : public AAA_JobData
{
   public:
        PeerApplication(AAA_Task &task, PANA_Node &node, int type) : 
            m_Handle(AppJobHandle
	            (AAA_GroupedJob::Create(task.Job(), this, "peer"))),
            m_Eap(boost::shared_ptr<AppPeerSwitchStateMachine>
	            (new AppPeerSwitchStateMachine(*task.reactor(), m_Handle))),
			m_Channel(boost::shared_ptr<PeerChannel>(new PeerChannel(node, *m_Eap))),
            m_Md5Method(EapContinuedPolicyElement(EapType(4))),
            m_ArchieMethod(EapContinuedPolicyElement(EapType(ARCHIE_METHOD_TYPE)))
        {
            switch (type) {
            case ARCHIE_METHOD_TYPE:
                m_Eap->Policy().InitialPolicyElement(&m_ArchieMethod);
                break;
            case 4: // MD5
                m_Eap->Policy().InitialPolicyElement(&m_Md5Method);
            }
            m_Eap->AuthPeriod() = PANA_CLIENT->Arg().m_Timeout;
        }
        ~PeerApplication() {
			m_Eap.reset();
			m_Channel.reset();
		}
        PeerChannel& Channel() { 
            return *m_Channel; 
        }
        AppPeerSwitchStateMachine& Eap() { 
            return *m_Eap; 
        }
    private:
        AppJobHandle m_Handle;
        boost::shared_ptr<AppPeerSwitchStateMachine> m_Eap;
		boost::shared_ptr<PeerChannel> m_Channel;
        EapContinuedPolicyElement m_Md5Method;
        EapContinuedPolicyElement m_ArchieMethod;
};

// ----------------- Definition --------------
void AppPeerSwitchStateMachine::Send(AAAMessageBlock *b)
{
    JobData(Type2Type<PeerApplication>()).Channel().EapResponse(b);
}

void AppPeerSwitchStateMachine::Success()
{
    JobData(Type2Type<PeerApplication>()).Channel().EapSuccess();
}

void AppPeerSwitchStateMachine::Failure()
{
    Stop();
    JobData(Type2Type<PeerApplication>()).Channel().EapFailure();
    PANA_CLIENT->Event().Failure();
}
void AppPeerSwitchStateMachine::Notification(std::string &str)
{
}
void AppPeerSwitchStateMachine::Abort()
{
    Stop();
    PANA_CLIENT->Event().Failure();
}
std::string& AppPeerSwitchStateMachine::InputIdentity() 
{
    return PANA_CLIENT->Arg().m_Username;
}

class PeerInitializer
{
    public:
        PeerInitializer() : 
          m_Task(AAA_SCHED_WFQ, "PEER"),
          m_PanaNode(m_Task, PANA_CLIENT->Arg().m_PanaCfgFile) {
            Start();
        }
        virtual ~PeerInitializer() {
            Stop();
        }
        PANA_Node &PanaNode() {
            return m_PanaNode;
        }
        AAA_Task &Task() {
            return m_Task;
        }
    protected:
        void Start() {
            m_MethodRegistrar.registerMethod
               (std::string("MD5-Challenge"), EapType(4),
                Peer, m_PeerMD5ChallengeCreator);

            m_MethodRegistrar.registerMethod
               (std::string("Archie"), EapType(ARCHIE_METHOD_TYPE), 
                Peer, m_PeerArchieCreator);

            try {
               m_Task.Start(PANA_CLIENT->Arg().m_ThreadCount);
            }
            catch (...) {
            }
        }
        void Stop() {
            m_Task.Stop();
        }
    private:
        AAA_Task m_Task;
        PANA_Node m_PanaNode;
        EapMethodRegistrar m_MethodRegistrar;
        EapMethodStateMachineCreator<AppEapPeerMD5ChallengeStateMachine> 
            m_PeerMD5ChallengeCreator;            
        EapMethodStateMachineCreator<AppEapPeerArchieStateMachine> 
            m_PeerArchieCreator;
};

class PeerHandle : public PANAClientHandle
{
    public:
        PeerHandle() {
            m_Init = boost::shared_ptr<PeerInitializer>
                (new PeerInitializer);
            m_App = boost::shared_ptr<PeerApplication>
                (new PeerApplication(m_Init->Task(), 
                                     m_Init->PanaNode(),
                                     PANA_CLIENT->Arg().m_Md5Only ? 4 :
                                     ARCHIE_METHOD_TYPE));
        }
        virtual ~PeerHandle() {
            m_App.reset();
            m_Init.reset();
        }
        boost::shared_ptr<PeerInitializer> m_Init;
        boost::shared_ptr<PeerApplication> m_App;
};

void PANAClient::UpdateAddress(ACE_INET_Addr &addr,
                               std::string &message)
{
    PeerHandle *peer = static_cast<PeerHandle*>(m_Handle.get());
	if (peer) {
		peer->m_App->Channel().UpdateAddress(addr, message);
    }
}

void PANAClient::SendPing()
{
    PeerHandle *peer = static_cast<PeerHandle*>(m_Handle.get());
	if (peer) {
		peer->m_App->Channel().SendPing();
    }
}

void PANAClient::SendNotification(std::string &message)
{
    PeerHandle *peer = static_cast<PeerHandle*>(m_Handle.get());
	if (peer) {
		peer->m_App->Channel().SendNotification(message);
    }
}

void PANAClient::Start()
{
    Stop();
    if (! m_Handle.get()) {
        m_LogStream.open("log.txt");
        ACE_Log_Msg::instance()->msg_ostream(&m_LogStream);
        ACE_Log_Msg::instance()->open("PANA", ACE_Log_Msg::OSTREAM);
        ACE_Log_Msg::instance()->enable_debug_messages();
        m_Handle = boost::shared_ptr<PeerHandle>(new PeerHandle);
    }
}

void PANAClient::Stop()
{
    PeerHandle *peer = static_cast<PeerHandle*>(m_Handle.get());
	if (peer) {
		peer->m_App->Channel().Stop();
        ACE_Time_Value tout(15, 0);
        ACE_OS::sleep(tout);
        m_Handle.reset();
        m_LogStream.close();
	}
}

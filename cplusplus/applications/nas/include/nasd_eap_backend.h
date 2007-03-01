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

#ifndef __NASD_EAP_BACKEND_H__
#define __NASD_EAP_BACKEND_H__

#include <fstream>
#include "nasd_call_framework.h"
#include "eap.hxx"
#include "eap_authfsm.hxx"
#include "eap_archie.hxx"
#include "eap_archie_fsm.hxx"
#include "eap_md5.hxx"
#include "eap_method_registrar.hxx"

class NASD_EapBackendAuthArchieStateMachine : 
      public EapAuthArchieStateMachine
{
     friend class EapMethodStateMachineCreator
                  <NASD_EapBackendAuthArchieStateMachine>;

     typedef enum {
        DEFAULT_AUTH_PERIOD = 3600
     };

   public:
     NASD_EapBackendAuthArchieStateMachine(EapSwitchStateMachine &s)
        : EapAuthArchieStateMachine(s) {
        
        std::string name("local_eap_auth");
        m_CfgData = (NASD_AaaLocalEapAuthData*)
             NASD_AAAPROTO_TBL().Lookup(name);
        if (m_CfgData == NULL) {
            NASD_LOG(LM_ERROR, 
              "(%P|%t) Diameter-EAP configuration entry not found\n");
        }
     }
     std::string& InputSharedSecret() {

	static std::string sharedSecret = "";
        if (sharedSecret.length() > 0) {
	    return sharedSecret;
	}
        
        if (m_CfgData) {
            char sBuf[64];
            ifstream secretFile(m_CfgData->Protocol().
                                SharedSecretFile().data(), 
                                ios::binary | ios::in);
            if (secretFile.is_open()) {
	        secretFile.read(sBuf, sizeof(sBuf));
                secretFile.close();
                sharedSecret.assign(sBuf, sizeof(sBuf));
	    
            } else {
                NASD_LOG(LM_ERROR, 
                   "(%P|%t) Failed to open shared-secret file [%s]\n",
                         m_CfgData->Protocol().SharedSecretFile().data());
            }
	}
        return sharedSecret;
     }
     std::string& InputIdentity() {

	static std::string serverId = "";
        if (serverId.length() > 0) {
	    return serverId;
	}
        
        if (m_CfgData) {
	    serverId = m_CfgData->Protocol().Identity();
	}              
        return serverId;
     }
     
   private:
     virtual ~NASD_EapBackendAuthArchieStateMachine() {
     } 

     NASD_AaaLocalEapAuthData *m_CfgData;
};

class NASD_EapBackendAuthMd5StateMachine : 
      public EapAuthMD5ChallengeStateMachine
{
     friend class EapMethodStateMachineCreator
                  <NASD_EapBackendAuthMd5StateMachine>;

     typedef enum {
        DEFAULT_AUTH_PERIOD = 3600
     };

   public:
     NASD_EapBackendAuthMd5StateMachine(EapSwitchStateMachine &s)
        : EapAuthMD5ChallengeStateMachine(s) {
        
        std::string name("local_eap_auth");
        m_CfgData = (NASD_AaaLocalEapAuthData*)
             NASD_AAAPROTO_TBL().Lookup(name);
        if (m_CfgData == NULL) {
            NASD_LOG(LM_ERROR, 
              "(%P|%t) Diameter-EAP configuration entry not found\n");
        }
     }
     std::string& InputSharedSecret() {

	static std::string sharedSecret = "";
        if (sharedSecret.length() > 0) {
	    return sharedSecret;
	}
        
        if (m_CfgData) {
            char sBuf[64];
            ifstream secretFile(m_CfgData->Protocol().
                                SharedSecretFile().data(), 
                                ios::binary | ios::in);
            if (secretFile.is_open()) {
	        secretFile.read(sBuf, sizeof(sBuf));
                secretFile.close();
                sharedSecret.assign(sBuf, sizeof(sBuf));
	    
            } else {
                NASD_LOG(LM_ERROR, 
                   "(%P|%t) Failed to open shared-secret file [%s]\n",
                         m_CfgData->Protocol().SharedSecretFile().data());
            }
	}
        return sharedSecret;
     }
     void InputPassphrase()  {
        std::string &passphrase = Passphrase();
        passphrase.assign("12345");
     }
     std::string& InputIdentity() {

	static std::string serverId = "";
        if (serverId.length() > 0) {
	    return serverId;
	}
        
        if (m_CfgData) {
	    serverId = m_CfgData->Protocol().Identity();
	}              
        return serverId;
     }
     
   private:
     virtual ~NASD_EapBackendAuthMd5StateMachine() {
     } 

     NASD_AaaLocalEapAuthData *m_CfgData;
};

class NASD_EapBackendAuthSwitchStateMachine : 
   public EapBackendAuthSwitchStateMachine
{
   public:

      NASD_EapBackendAuthSwitchStateMachine
             (ACE_Reactor &r, 
              EapJobHandle& h,
              NASD_CallNode &n) :
          EapBackendAuthSwitchStateMachine(r, h),
          m_Node(n),
          m_ArchiePolicy
              (EapContinuedPolicyElement(EapType(ARCHIE_METHOD_TYPE))),
          m_Md5Policy(EapContinuedPolicyElement(EapType(4))) {
          Policy().InitialPolicyElement(&m_Md5Policy);
          m_Md5Policy.AddContinuedPolicyElement
              (&m_ArchiePolicy, EapContinuedPolicyElement::PolicyOnFailure);
               
      }
      void Send(AAAMessageBlock *b) {
          m_Node.SendEgress(*b);
      }
      void ForwardResponse(AAAMessageBlock *b) {
          m_Node.SendIngress(*b);
      }
      void Success(AAAMessageBlock *b) {
          m_Node.PrevNode()->Success(b);
      }
      void Success() {
          m_Node.PrevNode()->Success(0);
      }
      void Failure(AAAMessageBlock *b) {
          m_Node.PrevNode()->Failure(b);
      }
      void Failure() {
          m_Node.PrevNode()->Failure(0);
      }
      void Abort() {
          m_Node.PrevNode()->Failure(0);
      }
      
   private:
      /// node reference
      NASD_CallNode &m_Node;
      
      /// policy elements
      EapContinuedPolicyElement m_ArchiePolicy;
      EapContinuedPolicyElement m_Md5Policy;
};

class NASD_EapBackend : 
   public NASD_CallNode
{
   public:
      NASD_EapBackend(AAA_Task &t) : 
         m_Task(t),
         m_JobHandle(AAA_GroupedJob::Create
            (t.Job(), this, "eap-backend")),
         m_IsFirstMsg(true) {
      }
      virtual ~NASD_EapBackend() {
	 Stop();
         m_JobHandle->Stop();
         m_JobHandle.reset();
      }
      int Start() {
         if (m_Eap.get()) {
             return (-1);
         }
         
         m_MethodRegistrar.registerMethod
             (std::string("Archie"), EapType(ARCHIE_METHOD_TYPE),
               Authenticator, m_NasEapAuthArchieCreator);

         m_MethodRegistrar.registerMethod
             (std::string("MD5-Challenge"), EapType(4),
               Authenticator, m_NasEapAuthMd5Creator);

         m_Eap = boost::shared_ptr<NASD_EapBackendAuthSwitchStateMachine>
                  (new NASD_EapBackendAuthSwitchStateMachine
                      (*m_Task.reactor(), m_JobHandle, *this));

         m_Eap->NeedInitialRequestToSend(false);
         return (0);
      }
      bool IsRunning() {
         return (m_Eap.get()) ? true : false;
      }
      void Stop() {
         if (m_Eap.get()) {         
             m_Eap->Stop();
             m_Eap.reset();
         }
      }
   protected:
      bool CurrentKey(std::string &key) {
         if (m_Eap->KeyAvailable()) {
             NASD_LOG(LM_INFO, "(%P|%t) Assigning key\n");
             key.assign(m_Eap->KeyData().data(), 
                        m_Eap->KeyData().size());
             return true;
         }
         return false;
      }
      bool Identity(std::string &ident) {
         return false; // don't support
      }
      int ReceiveIngress(AAAMessageBlock &msg) {
	 if (m_IsFirstMsg) {
	     m_Eap->Start(&msg);
             m_IsFirstMsg = false;
	 }
         else {
	     m_Eap->Receive(&msg);
	 }
         return (0);
      }
      int ReceiveEgress(AAAMessageBlock &msg) {
         return (-1);
      }
      void Success(AAAMessageBlock *msg = 0) {
         if (msg) {
             SendEgress(*msg);
         }
      }
      void Failure(AAAMessageBlock *msg = 0) {
         if (msg) {
             SendEgress(*msg);
         }
      }
      void Timeout() {
         Stop();
      }
      void Error() {
         Stop();
      }
   
   private:
      /// framework
      AAA_Task &m_Task;
      AAA_JobHandle<AAA_GroupedJob> m_JobHandle;
      
      /// eap instance
      boost::shared_ptr<NASD_EapBackendAuthSwitchStateMachine> m_Eap;
      
      /// method creators
      EapMethodRegistrar m_MethodRegistrar;
      EapMethodStateMachineCreator<NASD_EapBackendAuthArchieStateMachine> 
          m_NasEapAuthArchieCreator;
      EapMethodStateMachineCreator<NASD_EapBackendAuthMd5StateMachine> 
          m_NasEapAuthMd5Creator;

      /// first message flag
      bool m_IsFirstMsg;
};

class NASD_EapBackendInitializer : 
    public NASD_CnInitCallback
{
    public:
        virtual bool Initialize(AAA_Task &t) {
           NASD_LOG(LM_INFO, 
                  "(%P|%t) Initializing Local EAP auth\n");
           return (true);
	}
        virtual NASD_CallElement *Create(AAA_Task &t) {
           return (new NASD_EapBackend(t));
	}
};

#endif // __NASD_EAP_BACKEND_H__




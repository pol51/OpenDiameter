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


#ifndef __NASD_EAP_PASSTHROUGH_H__
#define __NASD_EAP_PASSTHROUGH_H__

#include "nasd_call_framework.h"
#include "eap_identity.hxx"
#include "eap_method_registrar.hxx"

class NASD_EapPassThroughAuthIdentityStateMachine : 
      public EapAuthIdentityStateMachine
{
     friend class EapMethodStateMachineCreator
                  <NASD_EapPassThroughAuthIdentityStateMachine>;
   public:
     NASD_EapPassThroughAuthIdentityStateMachine(EapSwitchStateMachine &s)
        : EapAuthIdentityStateMachine(s) {
     } 
     ProcessIdentityResult ProcessIdentity(std::string& identity);
     
   private:
     virtual ~NASD_EapPassThroughAuthIdentityStateMachine() {
     } 
};

class NASD_EapPassThroughAuthSwitchStateMachine :
      public EapPassThroughAuthSwitchStateMachine
{
   public:
      NASD_EapPassThroughAuthSwitchStateMachine
             (ACE_Reactor &r, 
              EapJobHandle& h,
              NASD_CallRouting &rte) :
          EapPassThroughAuthSwitchStateMachine(r, h),
          m_Node(rte),
          m_IdentityPolicy(EapContinuedPolicyElement(EapType(1))) {
          Policy().InitialPolicyElement(&m_IdentityPolicy);
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
      NASD_CallRouting &CallRouting() {
          return dynamic_cast<NASD_CallRouting&>(m_Node);
      }
      
   private:
      /// node reference
      NASD_CallNode &m_Node;
      
      /// policy elements
      EapContinuedPolicyElement m_IdentityPolicy;
};

class NASD_EapPassThrough : 
   public NASD_CallRouting
{
   public:
      NASD_EapPassThrough(AAA_Task &t) : 
         m_Task(t),
         m_JobHandle(AAA_GroupedJob::Create
             (t.Job(), this, "eap-passthrough")) {
      }
      virtual ~NASD_EapPassThrough() {
	 Stop();
         m_JobHandle->Stop();
         m_JobHandle.reset();
      }
      int Start() {
         if (m_Eap.get()) {
             return (-1);
         }
         
         m_MethodRegistrar.registerMethod
             (std::string("Identity"), EapType(1),
               Authenticator, m_NasEapAuthIdentityCreator);

         m_Eap = boost::shared_ptr<NASD_EapPassThroughAuthSwitchStateMachine>
                  (new NASD_EapPassThroughAuthSwitchStateMachine
                      (*m_Task.reactor(), m_JobHandle, *this));

         m_Eap->NeedInitialRequestToSend(false);
         m_Eap->Start();
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
	 if (m_Identity.length() > 0) {
	     ident = m_Identity;
             return true;
	 }
         return false;
      }
      int ReceiveIngress(AAAMessageBlock &msg) {
         m_Eap->Receive(&msg);
         return (0);
      }
      int ReceiveEgress(AAAMessageBlock &msg) {
         m_Eap->AAA_Continue(&msg);
         return (0); // standalone does not have next
      }
      void Success(AAAMessageBlock *msg = 0) {
         if (msg) {
	     std::string key;
	     if (NextNode()->CurrentKey(key)) {
                 m_Eap->AAA_Success(msg, key);
	     }
             else {
                 m_Eap->AAA_Success(msg);
	     }
         }
      }
      void Failure(AAAMessageBlock *msg = 0) {
         if (msg) {
             m_Eap->AAA_Failure(msg);
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
      boost::shared_ptr<NASD_EapPassThroughAuthSwitchStateMachine> m_Eap;
      
      /// method creators
      EapMethodRegistrar m_MethodRegistrar;
      EapMethodStateMachineCreator<NASD_EapPassThroughAuthIdentityStateMachine> 
          m_NasEapAuthIdentityCreator;

      /// user information
      std::string m_Identity;

      friend class NASD_EapPassThroughAuthIdentityStateMachine;
};

inline EapAuthIdentityStateMachine::ProcessIdentityResult 
NASD_EapPassThroughAuthIdentityStateMachine::ProcessIdentity
    (std::string& identity) 
{
    NASD_EapPassThroughAuthSwitchStateMachine &fsm = 
       (NASD_EapPassThroughAuthSwitchStateMachine&)
            SwitchStateMachine();
 
    NASD_EapPassThrough &pt = (NASD_EapPassThrough&)fsm.CallRouting();
    pt.m_Identity = identity;

    pt.IncommingCall(identity);
    return EapAuthIdentityStateMachine::Success;
}

#endif // __NASD_EAP_PASSTHROUGH_H__


